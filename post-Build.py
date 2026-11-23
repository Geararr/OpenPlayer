from pathlib import Path
import shutil
import sys
import subprocess

SCRIPT_DIR = Path(__file__).resolve().parent

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

LINUX_DEBUG_DIR = SCRIPT_DIR / "build/linux-debug"
LINUX_DEBUG_BUILD_EXE = LINUX_DEBUG_DIR / "OpenPlayer"
LINUX_DEBUG_EXPORT_DIR = SCRIPT_DIR / "EXPORT/OpenPlayer_Linux_DEBUG"

for export_dir in [LINUX_EXPORT_DIR, LINUX_DEBUG_EXPORT_DIR]:
    if export_dir.exists():
        shutil.rmtree(export_dir)
    zip_path = Path(f"{export_dir}.zip")
    if zip_path.exists():
        zip_path.unlink()

LINUX_EXPORT_DIR.mkdir(parents=True, exist_ok=True)
LINUX_DEBUG_EXPORT_DIR.mkdir(parents=True, exist_ok=True)

if LINUX_BUILD_EXE.exists():
    shutil.copy(str(LINUX_BUILD_EXE), str(LINUX_EXPORT_DIR / "OpenPlayer"))
    print(f"Copied Linux Release binary.")
else:
    print(f"Warning -> Linux Release binary not found at: {LINUX_BUILD_EXE}")

if LINUX_DEBUG_BUILD_EXE.exists():
    shutil.copy(str(LINUX_DEBUG_BUILD_EXE), str(LINUX_DEBUG_EXPORT_DIR / "OpenPlayer"))
    print(f"Copied Linux Debug binary.")
else:
    print(f"Warning -> Linux DEBUG binary not found at: {LINUX_DEBUG_BUILD_EXE}")

for export_dir in [LINUX_EXPORT_DIR, LINUX_DEBUG_EXPORT_DIR]:
    if any(export_dir.iterdir()):
        shutil.make_archive(base_name=str(export_dir), format="zip", root_dir=str(export_dir))
        print(f"Compressed {export_dir.name}.zip")

print("Linux local exports generated successfully!")