import subprocess
import sys

def runCmd(index):
    with subprocess.Popen(
            cmds[index],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
            shell=True
    ) as process:
        for line in process.stdout:
            print(line, end="", flush=True)

cmds = [
    "xmake f -p linux -a x86_64 -m debug -y && xmake -v",
    "xmake f -p linux -a x86_64 -m release -y && xmake -r -v",
    "xmake f -p mingw -a x86_64 -m release -y && xmake -r -v",
    "xmake f -p mingw -a i386 -m release -y && xmake -r -v"
]

platform = sys.argv[1]

if platform == "linux":
    mode = sys.argv[2]
else:
    arch = sys.argv[2]

if platform == "linux":
    if mode == "debug":
        runCmd(0)
    else:
        runCmd(1)
if platform == "windows" and arch == "x64":
    runCmd(2)
elif platform == "windows" and arch == "x86":
    runcmd(3)

if platform == "all":
    for i in range(4):
        runCmd(i)