from pathlib import Path
import shutil
import sys

SCRIPT_DIR = Path(__file__).resolve().parent
EXPORT_DIR = SCRIPT_DIR / "EXPORT"

if EXPORT_DIR.exists():
    shutil.rmtree(EXPORT_DIR)
EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if sys.platform.startswith("linux"):
    print("Packaging for Linux...")
    LINUX_REL_DIR = SCRIPT_DIR / "build"
    LINUX_BUILD_EXE = LINUX_REL_DIR / "OpenPlayer"
    LINUX_EXPORT_DIR = EXPORT_DIR / "OpenPlayer_Linux"
    LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

    if LINUX_BUILD_EXE.exists():
        shutil.copy(str(LINUX_BUILD_EXE), str(LINUX_EXPORT_DIR / "OpenPlayer"))

        slint_so = LINUX_REL_DIR / "_deps/slint-build/libslint_cpp.so"
        if slint_so.exists():
            shutil.copy(str(slint_so), str(LINUX_EXPORT_DIR))
        print(f"Copied Linux binary to {LINUX_EXPORT_DIR}")
    else:
        print(f"Error -> Linux binary not found at: {LINUX_BUILD_EXE}")
        sys.exit(1)

    shutil.make_archive(base_name=str(LINUX_EXPORT_DIR), format="zip", root_dir=str(LINUX_EXPORT_DIR))
    print("Created OpenPlayer_Linux.zip")

elif sys.platform == "win32":
    print("Packaging for Windows...")

    WIN_BUILD_DIR = SCRIPT_DIR / "build" / "Release"
    if not WIN_BUILD_DIR.exists():
        WIN_BUILD_DIR = SCRIPT_DIR / "build"

    WIN_BUILD_EXE = WIN_BUILD_DIR / "OpenPlayer.exe"
    WIN_EXPORT_DIR = EXPORT_DIR / "OpenPlayer_Windows"
    WIN_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

    if WIN_BUILD_EXE.exists():
        shutil.copy(str(WIN_BUILD_EXE), str(WIN_EXPORT_DIR / "OpenPlayer.exe"))

        for dll in WIN_BUILD_DIR.glob("*.dll"):
            shutil.copy(str(dll), str(WIN_EXPORT_DIR))

        print(f"Copied Windows binary and DLLs to {WIN_EXPORT_DIR}")
    else:
        print(f"Error -> Windows binary not found at: {WIN_BUILD_EXE}")
        sys.exit(1)

    shutil.make_archive(base_name=str(WIN_EXPORT_DIR), format="zip", root_dir=str(WIN_EXPORT_DIR))
    print("Created OpenPlayer_Windows.zip")

else:
    print(f"Unsupported platform: {sys.platform}")
    sys.exit(1)

print("Packaging complete!")