set_project("OpenPlayer")
set_version("6.0.1")
set_languages("cxx20")

add_rules("mode.debug", "mode.release")

add_requires("nlohmann_json", "taglib")

package("slint_source")
    set_sourcedir("slint_wrapper")

    on_install("windows", "linux", "mingw", function (package)
        import("package.tools.cmake").install(package, {})
    end)

    on_load(function (package)
        package:add("includedirs", "include", "include/slint")
        package:add("linkdirs", "lib", "build", "_deps/slint-build")
        package:add("links", "slint_cpp")
    end)
package_end()

add_requires("slint_source v1.18.0")

if is_plat("linux") then
    add_requires("libvips")
    if not is_arch("i386", "x86") then
        add_requires("tbb")
    end
end

local function getParent(path)
    path = path:gsub("([/\\]+)$", "")

    return path:match("^(.*)[/\\]")
end

target("OpenPlayer")
    set_kind("binary")

    add_files(
        "src/main.cpp",
        "src/uiManager.cpp",
        "src/soundManager.cpp",
        "src/utils.cpp",
        "src/eventManager.cpp",
        "src/jsonManager.cpp",
        "src/miniaudio_impl.cpp"
    )

    add_includedirs("include", "build/generated")

    on_load(function (target)
        local target_pkg = target:pkg("slint_source")
        local target_installdir = target_pkg:installdir()

        target:add("sysincludedirs", path.join(target_installdir, "include/slint"))

        if is_plat("linux") then
            target:add("syslinks", "fontconfig")
        end
    end)

    add_packages("nlohmann_json", "taglib", "slint_source")

    if is_plat("linux") then
        add_packages("libvips")
        add_syslinks("vips-cpp")
        if not is_arch("i386", "x86") then
            add_packages("tbb")
        end
    else
        add_defines("NO_TBB")
    end

    before_build(function (target)
        print("Compiling Slint UI files...")

        local slint_pkg = target:pkg("slint_source")
        local slint_pkg_installdir = slint_pkg:installdir()

        os.mkdir("build/generated")
        os.exec(path.join(slint_pkg_installdir, "bin", "slint-compiler") .. " include/ui/app_window.slint --format cpp --embed-resources embed-files -o build/generated/app_window.h")
    end)

    if is_mode("release") then
        set_optimize("smallest")
        set_strip("all")
        set_policy("build.optimization.lto", true)
    end

    if is_plat("windows", "mingw") then
        add_files("assets/win_icon.rc")
        add_syslinks("winmm")
        add_ldflags("-mwindows", "-Wl,--subsystem,windows", "-static-libgcc", "-static-libstdc++", {force = true})
    end

    after_build(function (target)
        print("Packaging build...")
        local plat = target:plat()
        local arch = target:arch()
        local mode = get_config("mode") or "release"
        local outdir = path.join("EXPORT/packaged", string.format("OpenPlayer-%s-%s-%s", plat, arch, mode))
        local appdir = "EXPORT/appdir"

        os.rm(outdir)
        os.rm(outdir .. ".zip")

        os.mkdir(outdir)
        os.cp(target:targetfile(), outdir)

        if plat == "windows" or plat == "mingw" then
            print("Copying Windows DLLs...")
            local pkg = target:pkg("slint_source")
            if pkg then
                os.trycp(path.join(pkg:installdir(), "bin", "*.dll"), outdir)
                os.trycp(path.join(pkg:installdir(), "lib", "*.dll"), outdir)
            end

            local zipname = outdir .. ".zip"
            print("Zipping Windows release...")
            os.exec(string.format("cd %s && zip -r ../../../%s .", outdir, zipname))
        elseif plat == "linux" then
            print("Creating Linux AppImage...")

            os.rm(appdir)

            os.execv(path.absolute("linuxdeploy-x86_64.AppImage"), {
             "--appdir", appdir,
             "--executable", path.absolute(path.join(outdir, "OpenPlayer")),
              "--desktop-file", "assets/OpenPlayer.desktop",
              "--icon-file", "assets/icon.svg",
              "--output", "appimage"
              }, {
                envs = {
                    OUTPUT = path.absolute(path.join(getParent(outdir), "OpenPlayer.AppImage"))
                }
              })
        end

        os.rm(outdir)

        print("Export complete in " .. outdir)
    end)