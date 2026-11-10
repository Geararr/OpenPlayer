from pathlib import Path
import shutil
import sys

SCRIPT_DIR = Path(__file__).resolve().parent

LINUX_BUILD_EXE = SCRIPT_DIR / "build/linux-rel/OpenPlayer"
LINUX_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux"

WINDOWS_BUILD_EXE = SCRIPT_DIR / "build/windows-rel/OpenPlayer.exe"
WINDOWS_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Win_x64"

lxExportZip = Path(f"{LINUX_EXPORT_DIR}.zip")
wdExportZip = Path(f"{WINDOWS_EXPORT_DIR}.zip")

if LINUX_EXPORT_DIR.exists():
    shutil.rmtree(LINUX_EXPORT_DIR)
    print("Removed old Linux export")
if lxExportZip.exists():
    lxExportZip.unlink()
    print("Removed old Linux export -> zip")

if WINDOWS_EXPORT_DIR.exists():
    shutil.rmtree(WINDOWS_EXPORT_DIR)
    print("Removed old Windows export")
if wdExportZip.exists():
    wdExportZip.unlink()
    print("Removed old Windows export -> zip")

LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if LINUX_BUILD_EXE.exists():
    destination = LINUX_EXPORT_DIR / "OpenPlayer"
    shutil.copy(str(LINUX_BUILD_EXE), str(destination))
    print(f"Moved Linux binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Linux binary not found at: {LINUX_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()


WINDOWS_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if WINDOWS_BUILD_EXE.exists():
    destination = WINDOWS_EXPORT_DIR / "OpenPlayer.exe"
    shutil.copy(str(WINDOWS_BUILD_EXE), str(destination))
    print(f"Moved Windows binary to -> {destination.relative_to(SCRIPT_DIR)}")
else:
    print(f"Error -> Windows binary not found at: {WINDOWS_BUILD_EXE.relative_to(SCRIPT_DIR)}")
    sys.exit()

MUSIC_FOLDER_DIR = SCRIPT_DIR / "assets/music"

LINUX_MUSIC_DEST = LINUX_EXPORT_DIR / "music"
WINDOWS_MUSIC_DEST = WINDOWS_EXPORT_DIR / "music"

shutil.copytree(str(MUSIC_FOLDER_DIR), str(LINUX_MUSIC_DEST))
shutil.copytree(str(MUSIC_FOLDER_DIR), str(WINDOWS_MUSIC_DEST))

shutil.make_archive(
    base_name=str("EXPORT/OpenPlayer_Linux"),
    format="zip",
    root_dir=str(LINUX_EXPORT_DIR)
)
print("Compressed OpenPlayer_Linux.zip")

shutil.make_archive(
    base_name=str("EXPORT/OpenPlayer_Win_x64"),
    format="zip",
    root_dir=str(WINDOWS_EXPORT_DIR)
)
print("Compressed OpenPlayer_Win_x64.zip")