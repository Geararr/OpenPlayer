from pathlib import Path
import shutil
import sys

SCRIPT_DIR = Path(__file__).resolve().parent

LINUX_BUILD_EXE = SCRIPT_DIR / "build/linux-rel/OpenPlayer"
LINUX_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"
LINUX_DEBUG_BUILD_EXE = SCRIPT_DIR / "build/linux-debug/OpenPlayer"
LINUX_DEBUG_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux_DEBUG"

WINDOWS_X64_BUILD_EXE = SCRIPT_DIR / "build/windows-rel-x64/OpenPlayer.exe"
WINDOWS_X64_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64"
WINDOWS_X64_DLLS = [
    "/usr/x86_64-w64-mingw32/bin/libstdc++-6.dll",
    SCRIPT_DIR / "build/windows-rel-x64/_deps/slint-build/slint_cpp.dll",
    "/usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll",
    "/usr/x86_64-w64-mingw32/bin/libtag.dll",
    "/usr/x86_64-w64-mingw32/bin/libssp-0.dll",
    "/usr/x86_64-w64-mingw32/bin/zlib1.dll",
    "/usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll",
]
WINDOWS_DEBUG_X64_BUILD_EXE = SCRIPT_DIR / "build/windows-debug-x64/OpenPlayer.exe"
WINDOWS_DEBUG_X64_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64_DEBUG"
WINDOWS_DEBUG_X64_DLLS = [
    "/usr/x86_64-w64-mingw32/bin/libstdc++-6.dll",
    SCRIPT_DIR / "build/windows-rel-x64/_deps/slint-build/slint_cpp.dll",
    "/usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll",
    "/usr/x86_64-w64-mingw32/bin/libtag.dll",
    "/usr/x86_64-w64-mingw32/bin/libssp-0.dll",
    "/usr/x86_64-w64-mingw32/bin/zlib1.dll",
    "/usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll",
]

WINDOWS_X86_BUILD_EXE = SCRIPT_DIR / "build/windows-rel-x86/OpenPlayer.exe"
WINDOWS_X86_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x86"
WINDOWS_X86_DLLS = [
    "/usr/i686-w64-mingw32/bin/libstdc++-6.dll",
    SCRIPT_DIR / "build/windows-rel-x86/_deps/slint-build/slint_cpp.dll",
    "/usr/i686-w64-mingw32/bin/libwinpthread-1.dll",
    "/usr/i686-w64-mingw32/bin/libtag.dll",
    "/usr/i686-w64-mingw32/bin/libssp-0.dll",
    "/usr/i686-w64-mingw32/bin/zlib1.dll",
    "/usr/i686-w64-mingw32/bin/libgcc_s_dw2-1.dll",
]

lxExportZip = Path(f"{LINUX_EXPORT_DIR}.zip")
lxDEBUGExportZip = Path(f"{LINUX_DEBUG_EXPORT_DIR}.zip")
wdX64ExportZip = Path(f"{WINDOWS_X64_EXPORT_DIR}.zip")
wdX64DEBUGExportZip = Path(f"{WINDOWS_DEBUG_X64_EXPORT_DIR}.zip")
wdX86ExportZip = Path(f"{WINDOWS_X86_EXPORT_DIR}.zip")

# LINUX
if LINUX_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_EXPORT_DIR)
    print("Removed old Linux export")
if lxExportZip.exists():
    lxExportZip.unlink()
    print("Removed old Linux export -> zip")
if LINUX_DEBUG_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_DEBUG_EXPORT_DIR)
    print("Removed of Linux DEBUG export")
if lxDEBUGExportZip.exists():
    lxDEBUGExportZip.unlink()
    print("Removed ol Linux DEBUG export -> zip")

# WINDOWS X64 & X86
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
if LINUX_DEBUG_BUILD_EXE.exists():
    destination = LINUX_DEBUG_EXPORT_DIR / "OpenPlayer"
    shutil.copy(str(LINUX_DEBUG_BUILD_EXE), str(destination))
    print(f"Moved Linux DEBUG binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux DEBUG binary not found at: {LINUX_DEBUG_BUILD_EXE.relative_to(SCRIPT_DIR)}")
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

# copy dlls into windows release folders
for dll in WINDOWS_X64_DLLS:
    print(f"Copying {str(dll)} -> Windows x64")
    shutil.copy(str(dll), str(WINDOWS_X64_EXPORT_DIR))
for dll in WINDOWS_DEBUG_X64_DLLS:
    print(f"Copying {str(dll)} -> Windows x64 DEBUG")
    shutil.copy(str(dll), str(WINDOWS_DEBUG_X64_EXPORT_DIR))
for dll in WINDOWS_X86_DLLS:
    print(f"Copying {str(dll)} -> Windows x86")
    shutil.copy(str(dll), str(WINDOWS_X86_EXPORT_DIR))

MUSIC_FOLDER_DIR = SCRIPT_DIR / "assets/music"

LINUX_MUSIC_DEST = LINUX_EXPORT_DIR / "music"
LINUX_DEBUG_MUSIC_DEST = LINUX_DEBUG_EXPORT_DIR / "music"
WINDOWS_X64_MUSIC_DEST = WINDOWS_X64_EXPORT_DIR / "music"
WINDOWS_DEBUG_X64_MUSIC_DEST = WINDOWS_DEBUG_X64_EXPORT_DIR / "music"
WINDOWS_X86_MUSIC_DEST = WINDOWS_X86_EXPORT_DIR / "music"

shutil.copytree(str(MUSIC_FOLDER_DIR), str(LINUX_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(LINUX_DEBUG_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_X64_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_DEBUG_X64_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_X86_MUSIC_DEST))

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