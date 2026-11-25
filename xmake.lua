set_project("OpenPlayer")
set_version("6.0.1")
set_languages("cxx20")

add_rules("mode.debug", "mode.release")

add_requires("nlohmann_json", "taglib", "libvips")
add_requireconfs("libvips.glib", {version = "2.78.1", override = true})

package("slint_source")
    set_sourcedir("slint_wrapper")

    on_install("windows", "linux", "mingw", function (package)
        local args = {
            "-DRust_CARGO_TARGET=i686-pc-windows-gnu",
            "-DRust_CARGO_TARGET=x86_64-pc-windows-gnu"
        }
        local acArgs = {}
        if (is_plat("windows", "mingw") and package:is_arch("i386")) then table.insert(acArgs, args[1]) end
        if (is_plat("windows", "mingw") and package:is_arch("x86_64")) then table.insert(acArgs, args[2]) end
        import("package.tools.cmake").install(package, acArgs)
    end)

    on_load(function (package)
        package:add("includedirs", "include", "include/slint")
        package:add("linkdirs", "lib", "build", "_deps/slint-build")
        package:add("links", "slint_cpp")
    end)
package_end()

if is_plat("windows", "mingw") then
    add_requires("slint_source v1.18.0", {configs = {shared = true}})
elseif is_plat("linux") then
    add_requires("slint_source v1.18.0", {configs = {shared = false}})
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

    add_packages("nlohmann_json", "taglib", "slint_source", "libvips")

    add_syslinks("vips-cpp")

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
        if is_plat("windows", "mingw") then
            set_policy("build.optimization.lto", false)
        else
	        set_policy("build.optimization.lto", true)
	    end
    end

    if is_plat("windows", "mingw") then
        add_files("assets/win_icon.rc")
        add_syslinks("winmm", "imm32", "opengl32", "synchronization", "ntdll")
        add_ldflags("-mwindows", "-Wl,--subsystem,windows", "-static-libgcc", "-static-libstdc++", {force = true})
    end

    after_build(function (target)
        print("Packaging build...")
        local plat = target:plat()
        local arch = target:arch()
        local mode = get_config("mode") or "release"
        local platName = plat
        if plat == "mingw" or plat == "windows" then platName = "win" end
        local outdir = path.join("EXPORT/packaged", string.format("OpenPlayer-%s-%s-%s", platName, arch, mode))
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
            local og_dir = os.curdir()
            os.cd(outdir)
            os.exec(string.format("zip -r ../../../%s .", zipname))
            os.cd(og_dir)
        elseif plat == "linux" then
            print("Creating Linux AppImage...")

            os.rm(appdir)

            os.execv(path.absolute("linuxdeploy-x86_64.AppImage"), {
             "--appdir", appdir,
             "--executable", path.absolute(path.join(outdir, "OpenPlayer")),
              "--desktop-file", "assets/OpenPlayer.desktop",
              "--icon-file", "assets/openplayer_icon.svg",
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
