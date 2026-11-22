var linux_button = document.getElementById("linux-download")
var win_x64_button = document.getElementById("windows-x64-download")
var win_x86_button = document.getElementById("windows-x86-download")

var download_links = [
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer_Linux.zip",
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer_Win_x64.zip",
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer_Win_x86.zip"
]
// 0 = linux, 1 = win_x64, 2 = win_x86

linux_button.addEventListener("click", () => {
    window.open(download_links[0], "_blank", "noopener,noreferrer");
});
win_x64_button.addEventListener("click", () => {
    window.open(download_links[1], "_blank", "noopener,noreferrer");
});
win_x86_button.addEventListener("click", () => {
    window.open(download_links[2], "_blank", "noopener,noreferrer");
});

var main_project_button = document.getElementById("main-project")

main_project_button.addEventListener("click", () => {
    window.open("https://github.com/Geararr/OpenPlayer", "_blank", "noopener,noreferrer");
});