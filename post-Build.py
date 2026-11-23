from pathlib import Path
import shutil
import sys
import subprocess

SCRIPT_DIR = Path(__file__).resolve().parent

def get_required_dlls(entry_binaries: list[Path], search_dirs: list[Path], objdump: str) -> list[Path]:
    available_dlls: dict[str, Path] = {}
    for folder in reversed(search_dirs):
        if folder.exists():
            for dll in folder.glob("*.dll"):
                available_dlls[dll.name.lower()] = dll

    resolved: dict[str, Path] = {}
    queue = list(entry_binaries)
    visited = set()

    while queue:
        binary = queue.pop(0)
        if binary in visited or not binary.exists():
            continue
        visited.add(binary)

        try:
            out = subprocess.check_output([objdump, "-p", str(binary)], text=True, stderr=subprocess.DEVNULL)
            for line in out.splitlines():
                line = line.strip()
                if line.startswith("DLL Name:"):
                    dll_name = line.split(":", 1)[1].strip().lower()
                    if dll_name in available_dlls and dll_name not in resolved:
                        dll_path = available_dlls[dll_name]
                        resolved[dll_name] = dll_path
                        queue.append(dll_path)
        except Exception:
            pass

    return list(resolved.values())

def get_required_sos(entry_binaries: list[Path], search_dirs: list[Path], objdump: str) -> list[Path]:
    available_sos: dict[str, Path] = {}
    for folder in reversed(search_dirs):
        if folder.exists():
            for so in folder.glob("*.so*"):
                available_sos[so.name.lower()] = so
                base_name = so.name.split('.so')[0] + '.so'
                available_sos[base_name.lower()] = so

    resolved: dict[str, Path] = {}
    queue = list(entry_binaries)
    visited = set()

    system_libs = {
        "libc.so.6", "libm.so.6", "libpthread.so.0", "libdl.so.2",
        "librt.so.1", "libgcc_s.so.1", "libstdc++.so.6", "ld-linux-x86-64.so.2"
    }

    while queue:
        binary = queue.pop(0)
        if binary in visited or not binary.exists():
            continue
        visited.add(binary)

        try:
            out = subprocess.check_output([objdump, "-p", str(binary)], text=True, stderr=subprocess.DEVNULL)
            for line in out.splitlines():
                line = line.strip()
                if "NEEDED" in line:
                    parts = line.split()
                    if len(parts) >= 2:
                        so_name = parts[-1].lower()
                        if so_name in system_libs:
                            continue
                        if so_name in available_sos and so_name not in resolved:
                            so_path = available_sos[so_name]
                            resolved[so_name] = so_path
                            queue.append(so_path)
        except Exception:
            pass

    return list(resolved.values())

LINUX_REL_DIR = SCRIPT_DIR / "build/linux-rel"
LINUX_BUILD_EXE = LINUX_REL_DIR / "OpenPlayer"
LINUX_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"
LINUX_SLINT_SO = LINUX_REL_DIR / "_deps/slint-build/libslint_cpp.so"
LINUX_DLLS = [
    LINUX_SLINT_SO,
    *get_required_sos(
        entry_binaries=[LINUX_BUILD_EXE, LINUX_SLINT_SO],
        search_dirs=[
            LINUX_REL_DIR / "_deps/slint-build",
            LINUX_REL_DIR,
            Path("/usr/lib"),
            Path("/usr/lib/x86_64-linux-gnu"),
            ],
        objdump="objdump",
    ),
]

LINUX_DEBUG_DIR = SCRIPT_DIR / "build/linux-debug"
LINUX_DEBUG_BUILD_EXE = LINUX_DEBUG_DIR / "OpenPlayer"
LINUX_DEBUG_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux_DEBUG"
LINUX_DEBUG_SLINT_SO = LINUX_DEBUG_DIR / "_deps/slint-build/libslint_cpp.so"
LINUX_DEBUG_DLLS = [
    LINUX_DEBUG_SLINT_SO,
    *get_required_sos(
        entry_binaries=[LINUX_DEBUG_BUILD_EXE, LINUX_DEBUG_SLINT_SO],
        search_dirs=[
            LINUX_DEBUG_DIR / "_deps/slint-build",
            LINUX_DEBUG_DIR,
            Path("/usr/lib"),
            Path("/usr/lib/x86_64-linux-gnu"),
            ],
        objdump="objdump",
    ),
]

WINDOWS_X64_BUILD_DIR = SCRIPT_DIR / "build/windows-rel-x64"
WINDOWS_X64_BUILD_EXE = WINDOWS_X64_BUILD_DIR / "OpenPlayer.exe"
WINDOWS_X64_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64"
WINDOWS_X64_SLINT_DLL = WINDOWS_X64_BUILD_DIR / "_deps/slint-build/slint_cpp.dll"
WINDOWS_X64_DLLS = [
    WINDOWS_X64_SLINT_DLL,
    *get_required_dlls(
        entry_binaries=[WINDOWS_X64_BUILD_EXE, WINDOWS_X64_SLINT_DLL],
        search_dirs=[
            WINDOWS_X64_BUILD_DIR / "vcpkg_installed/x64-mingw-dynamic/bin",
            Path("/usr/x86_64-w64-mingw32/bin"),
            ],
        objdump="x86_64-w64-mingw32-objdump",
    ),
]

WINDOWS_DEBUG_X64_BUILD_DIR = SCRIPT_DIR / "build/windows-debug-x64"
WINDOWS_DEBUG_X64_BUILD_EXE = WINDOWS_DEBUG_X64_BUILD_DIR / "OpenPlayer.exe"
WINDOWS_DEBUG_X64_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64_DEBUG"
WINDOWS_DEBUG_X64_SLINT_DLL = WINDOWS_DEBUG_X64_BUILD_DIR / "_deps/slint-build/slint_cpp.dll"
WINDOWS_DEBUG_X64_DLLS = [
    WINDOWS_DEBUG_X64_SLINT_DLL,
    *get_required_dlls(
        entry_binaries=[WINDOWS_DEBUG_X64_BUILD_EXE, WINDOWS_DEBUG_X64_SLINT_DLL],
        search_dirs=[
            WINDOWS_DEBUG_X64_BUILD_DIR / "vcpkg_installed/x64-mingw-dynamic/debug/bin",
            WINDOWS_DEBUG_X64_BUILD_DIR / "vcpkg_installed/x64-mingw-dynamic/bin",
            Path("/usr/x86_64-w64-mingw32/bin"),
            ],
        objdump="x86_64-w64-mingw32-objdump",
    ),
]

WINDOWS_X86_BUILD_DIR = SCRIPT_DIR / "build/windows-rel-x86"
WINDOWS_X86_BUILD_EXE = WINDOWS_X86_BUILD_DIR / "OpenPlayer.exe"
WINDOWS_X86_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x86"
WINDOWS_X86_SLINT_DLL = WINDOWS_X86_BUILD_DIR / "_deps/slint-build/slint_cpp.dll"
WINDOWS_X86_DLLS = [
    WINDOWS_X86_SLINT_DLL,
    *get_required_dlls(
        entry_binaries=[WINDOWS_X86_BUILD_EXE, WINDOWS_X86_SLINT_DLL],
        search_dirs=[
            WINDOWS_X86_BUILD_DIR / "vcpkg_installed/x86-mingw-dynamic/bin",
            Path("/usr/i686-w64-mingw32/bin"),
            ],
        objdump="i686-w64-mingw32-objdump",
    ),
]

for export_dir in [LINUX_EXPORT_DIR, LINUX_DEBUG_EXPORT_DIR, WINDOWS_X64_EXPORT_DIR, WINDOWS_DEBUG_X64_EXPORT_DIR, WINDOWS_X86_EXPORT_DIR]:
    if export_dir.exists():
        shutil.rmtree(export_dir)
    zip_path = Path(f"{export_dir}.zip")
    if zip_path.exists():
        zip_path.unlink()

LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
LINUX_DEBUG_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_X64_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_DEBUG_X64_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_X86_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if LINUX_BUILD_EXE.exists():
    shutil.copy(str(LINUX_BUILD_EXE), str(LINUX_EXPORT_DIR / "OpenPlayer"))

    for so in LINUX_DLLS:
        if so.exists():
            print(f"Copying {so.name} -> Linux release")
            shutil.copy(str(so), str(LINUX_EXPORT_DIR))
else:
    print(f"Error -> Linux binary not found at: {LINUX_BUILD_EXE}")
    sys.exit(1)

if LINUX_DEBUG_BUILD_EXE.exists():
    shutil.copy(str(LINUX_DEBUG_BUILD_EXE), str(LINUX_DEBUG_EXPORT_DIR / "OpenPlayer"))

    for so in LINUX_DEBUG_DLLS:
        if so.exists():
            print(f"Copying {so.name} -> Linux DEBUG")
            shutil.copy(str(so), str(LINUX_DEBUG_EXPORT_DIR))

else:
    print(f"Warning -> Linux DEBUG binary not found at: {LINUX_DEBUG_BUILD_EXE}")

if WINDOWS_X64_BUILD_EXE.exists():
    shutil.copy(str(WINDOWS_X64_BUILD_EXE), str(WINDOWS_X64_EXPORT_DIR / "OpenPlayer.exe"))
else:
    print(f"Error -> Windows x64 binary not found at: {WINDOWS_X64_BUILD_EXE}")
    sys.exit(1)

if WINDOWS_DEBUG_X64_BUILD_EXE.exists():
    shutil.copy(str(WINDOWS_DEBUG_X64_BUILD_EXE), str(WINDOWS_DEBUG_X64_EXPORT_DIR / "OpenPlayer.exe"))
else:
    print(f"Warning -> Windows x64 DEBUG binary not found")

if WINDOWS_X86_BUILD_EXE.exists():
    shutil.copy(str(WINDOWS_X86_BUILD_EXE), str(WINDOWS_X86_EXPORT_DIR / "OpenPlayer.exe"))
else:
    print(f"Warning -> Windows x86 binary not found")

for dll in WINDOWS_X64_DLLS:
    shutil.copy(str(dll), str(WINDOWS_X64_EXPORT_DIR))
for dll in WINDOWS_DEBUG_X64_DLLS:
    shutil.copy(str(dll), str(WINDOWS_DEBUG_X64_EXPORT_DIR))
for dll in WINDOWS_X86_DLLS:
    shutil.copy(str(dll), str(WINDOWS_X86_EXPORT_DIR))

for export_dir in [LINUX_EXPORT_DIR, LINUX_DEBUG_EXPORT_DIR, WINDOWS_X64_EXPORT_DIR, WINDOWS_DEBUG_X64_EXPORT_DIR, WINDOWS_X86_EXPORT_DIR]:
    if any(export_dir.iterdir()):
        shutil.make_archive(base_name=str(export_dir), format="zip", root_dir=str(export_dir))
        print(f"Compressed {export_dir.name}.zip")

print("All exports generated successfully!")