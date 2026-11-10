from pathlib import Path
import shutil
import sys

SCRIPT_DIR = Path(__file__).resolve().parent

LINUX_BUILD_EXE = SCRIPT_DIR / "build/linux-rel/OpenPlayer"
LINUX_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"

WINDOWS_X64_BUILD_EXE = SCRIPT_DIR / "build/windows-rel-x64/OpenPlayer.exe"
WINDOWS_X64_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64"

WINDOWS_X86_BUILD_EXE = SCRIPT_DIR / "build/windows-rel-x86/OpenPlayer.exe"
WINDOWS_X86_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x86"

lxExportZip = Path(f"{LINUX_EXPORT_DIR}.zip")
wdX64ExportZip = Path(f"{WINDOWS_X64_EXPORT_DIR}.zip")
wdX86ExportZip = Path(f"{WINDOWS_X86_EXPORT_DIR}.zip")

# LINUX
if LINUX_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_EXPORT_DIR)
    print("Removed old Linux export")
if lxExportZip.exists():
    lxExportZip.unlink()
    print("Removed old Linux export -> zip")

# WINDOWS X64 & X86
if WINDOWS_X64_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_X64_EXPORT_DIR)
    print("Removed old Windows export")
if wdX64ExportZip.exists():
    wdX64ExportZip.unlink()
    print("Removed old Windows export -> zip")
if WINDOWS_X64_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_X64_EXPORT_DIR)
    print("Removed old Windows export")
if wdX64ExportZip.exists():
    wdX64ExportZip.unlink()
    print("Removed old Windows export -> zip")

LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if LINUX_BUILD_EXE.exists():
    destination = LINUX_EXPORT_DIR / "OpenPlayer"
    shutil.copy(str(LINUX_BUILD_EXE), str(destination))
    print(f"Moved Linux binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux binary not found at: {LINUX_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()


WINDOWS_X64_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
WINDOWS_X86_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if WINDOWS_X64_BUILD_EXE.exists():
    destination = WINDOWS_X64_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_X64_BUILD_EXE), str(destination))
    print(f"Moved Windows x64 binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows x64 binary not found at: {WINDOWS_X64_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()
if WINDOWS_X86_BUILD_EXE.exists():
    destination = WINDOWS_X86_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_X86_BUILD_EXE), str(destination))
    print(f"Moved Windows x86 binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows x86 binary not found at: {WINDOWS_X86_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

MUSIC_FOLDER_DIR = SCRIPT_DIR / "assets/music"

LINUX_MUSIC_DEST = LINUX_EXPORT_DIR / "music"
WINDOWS_X64_MUSIC_DEST = WINDOWS_X64_EXPORT_DIR / "music"
WINDOWS_X86_MUSIC_DEST = WINDOWS_X86_EXPORT_DIR / "music"

shutil.copytree(str(MUSIC_FOLDER_DIR), str(LINUX_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_X64_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_X86_MUSIC_DEST))

shutil.make_archive(
    base_name=str("EXPORT/OpenPlayer_Linux"),
    format="zip",
    root_dir=str(LINUX_EXPORT_DIR)
)
print("Compressed OpenPlayer_Linux.zip")

shutil.make_archive(
    base_name=str("EXPORT/OpenPlayer_Win_x64"),
    format="zip",
    root_dir=str(WINDOWS_X64_EXPORT_DIR)
)
print("Compressed OpenPlayer_Win_x64.zip")
shutil.make_archive(
    base_name=str("EXPORT/OpenPlayer_Win_x86"),
    format="zip",
    root_dir=str(WINDOWS_X86_EXPORT_DIR)
)
print("Compressed OpenPlayer_Win_x86.zip")