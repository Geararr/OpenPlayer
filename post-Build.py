from pathlib import Path
import shutil
import sys

SCRIPT_DIR = Path(__file__).resolve().parent

LINUX_BUILD_EXE = SCRIPT_DIR / "build/linux-rel/OpenPlayer"
LINUX_SLINT_SO = SCRIPT_DIR / "build/linux-rel/_deps/slint-build/libslint_cpp.so"
LINUX_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"

LINUX_DEBUG_BUILD_EXE = SCRIPT_DIR / "build/linux-debug/OpenPlayer"
LINUX_DEBUG_SLINT_SO = SCRIPT_DIR / "build/linux-debug/_deps/slint-build/libslint_cpp.so"
LINUX_DEBUG_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux_DEBUG"

import subprocess

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

        out = subprocess.check_output([objdump, "-p", str(binary)], text=True, stderr=subprocess.DEVNULL)
        for line in out.splitlines():
            line = line.strip()
            if line.startswith("DLL Name:"):
                dll_name = line.split(":", 1)[1].strip().lower()
                if dll_name in available_dlls and dll_name not in resolved:
                    dll_path = available_dlls[dll_name]
                    resolved[dll_name] = dll_path
                    queue.append(dll_path)

    return list(resolved.values())

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

lxExportZip = Path(f"{LINUX_EXPORT_DIR}.zip")
lxDEBUGExportZip = Path(f"{LINUX_DEBUG_EXPORT_DIR}.zip")
wdX64ExportZip = Path(f"{WINDOWS_X64_EXPORT_DIR}.zip")
wdX64DEBUGExportZip = Path(f"{WINDOWS_DEBUG_X64_EXPORT_DIR}.zip")
wdX86ExportZip = Path(f"{WINDOWS_X86_EXPORT_DIR}.zip")

if LINUX_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_EXPORT_DIR)
    print("Removed old Linux export")
if lxExportZip.exists():
    lxExportZip.unlink()
    print("Removed old Linux export -> zip")
if LINUX_DEBUG_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_DEBUG_EXPORT_DIR)
    print("Removed old Linux DEBUG export")
if lxDEBUGExportZip.exists():
    lxDEBUGExportZip.unlink()
    print("Removed old Linux DEBUG export -> zip")

if WINDOWS_X64_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_X64_EXPORT_DIR)
    print("Removed old Windows X64 export")
if wdX64ExportZip.exists():
    wdX64ExportZip.unlink()
    print("Removed old Windows X64 export -> zip")
if WINDOWS_DEBUG_X64_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_DEBUG_X64_EXPORT_DIR)
    print("Removed old Windows DEBUG X64 export")
if wdX64DEBUGExportZip.exists():
    wdX64DEBUGExportZip.unlink()
    print("Removed old Windows DEBUG X64 export -> zip")

if WINDOWS_X86_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_X86_EXPORT_DIR)
    print("Removed old Windows X86 export")
if wdX86ExportZip.exists():
    wdX86ExportZip.unlink()
    print("Removed old Windows X86 export -> zip")

LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
LINUX_DEBUG_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if LINUX_BUILD_EXE.exists():
    destination = LINUX_EXPORT_DIR / "OpenPlayer"
    shutil.copy(str(LINUX_BUILD_EXE), str(destination))
    print(f"Moved Linux binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux binary not found at: {LINUX_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

if LINUX_SLINT_SO.exists():
    destination = LINUX_EXPORT_DIR / "libslint_cpp.so"
    shutil.copy(str(LINUX_SLINT_SO), str(destination))
    print(f"Moved Linux shared library to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux shared library not found at: {LINUX_SLINT_SO.relative_to(SCRIPT_DIR)}")
    sys.exit()

if LINUX_DEBUG_BUILD_EXE.exists():
    destination = LINUX_DEBUG_EXPORT_DIR / "OpenPlayer"
    shutil.copy(str(LINUX_DEBUG_BUILD_EXE), str(destination))
    print(f"Moved Linux DEBUG binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux DEBUG binary not found at: {LINUX_DEBUG_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

if LINUX_DEBUG_SLINT_SO.exists():
    destination = LINUX_DEBUG_EXPORT_DIR / "libslint_cpp.so"
    shutil.copy(str(LINUX_DEBUG_SLINT_SO), str(destination))
    print(f"Moved Linux DEBUG shared library to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux DEBUG shared library not found at: {LINUX_DEBUG_SLINT_SO.relative_to(SCRIPT_DIR)}")
    sys.exit()

WINDOWS_X64_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_DEBUG_X64_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_X86_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if WINDOWS_X64_BUILD_EXE.exists():
    destination = WINDOWS_X64_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_X64_BUILD_EXE), str(destination))
    print(f"Moved Windows x64 binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows x64 binary not found at: {WINDOWS_X64_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()
if WINDOWS_DEBUG_X64_BUILD_EXE.exists():
    destination = WINDOWS_DEBUG_X64_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_DEBUG_X64_BUILD_EXE), str(destination))
    print(f"Moved Windows x64 DEBUG binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows x64 DEBUG binary not found at: {WINDOWS_DEBUG_X64_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

if WINDOWS_X86_BUILD_EXE.exists():
    destination = WINDOWS_X86_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_X86_BUILD_EXE), str(destination))
    print(f"Moved Windows x86 binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows x86 binary not found at: {WINDOWS_X86_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

for dll in WINDOWS_X64_DLLS:
    print(f"Copying {str(dll)} -> Windows x64")
    shutil.copy(str(dll), str(WINDOWS_X64_EXPORT_DIR))
for dll in WINDOWS_DEBUG_X64_DLLS:
    print(f"Copying {str(dll)} -> Windows x64 DEBUG")
    shutil.copy(str(dll), str(WINDOWS_DEBUG_X64_EXPORT_DIR))
for dll in WINDOWS_X86_DLLS:
    print(f"Copying {str(dll)} -> Windows x86")
    shutil.copy(str(dll), str(WINDOWS_X86_EXPORT_DIR))

shutil.make_archive(
    base_name=str(SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"),
    format="zip",
    root_dir=str(LINUX_EXPORT_DIR)
)
print("Compressed OpenPlayer_Linux.zip")
shutil.make_archive(
    base_name=str(SCRIPT_DIR / "EXPORT/OpenPlayer_Linux_DEBUG"),
    format="zip",
    root_dir=str(LINUX_DEBUG_EXPORT_DIR)
)
print("Compressed OpenPlayer_Linux_DEBUG.zip")

shutil.make_archive(
    base_name=str(SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64"),
    format="zip",
    root_dir=str(WINDOWS_X64_EXPORT_DIR)
)
shutil.make_archive(
    base_name=str(SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64_DEBUG"),
    format="zip",
    root_dir=str(WINDOWS_DEBUG_X64_EXPORT_DIR)
)

print("Compressed OpenPlayer_Win_x64.zip")
shutil.make_archive(
    base_name=str(SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x86"),
    format="zip",
    root_dir=str(WINDOWS_X86_EXPORT_DIR)
)
print("Compressed OpenPlayer_Win_x86.zip")