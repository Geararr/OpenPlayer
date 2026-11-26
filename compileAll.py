import subprocess

cmd = "xmake f -p linux -a x86_64 -m release -y && \
xmake -r -v && \
xmake f -p mingw -a x86_64 -m release -y && \
xmake -r -v && \
xmake f -p mingw -a i386 -m release -y && \
xmake -r -v"

with subprocess.Popen(
    cmd,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
    bufsize=1
) as process:
    for line in process.stdout:
        print(line, end="", flush=True)
