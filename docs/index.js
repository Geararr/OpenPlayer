var linux_button = document.getElementById("linux-download")
var win_x64_button = document.getElementById("windows-x64-download")
var win_x86_button = document.getElementById("windows-x86-download")

var download_links = [
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer.AppImage",
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer-win-x86_64-release.zip",
    "https://github.com/Geararr/OpenPlayer/releases/latest/download/OpenPlayer-win-i386-release.zip"
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

var demo_video_button = document.getElementById("demo-video")
var demo_vid_clicked = false;

demo_video_button.addEventListener("click", () => {
    if (!demo_vid_clicked) {
        demo_video_button.innerHTML = `
        <iframe 
            width="771" 
            height="434" src="https://www.youtube.com/embed/wr992kEoP2I" 
            title="OpenPlayer v5.0.0 Demo" 
            frameborder="0" 
            allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" 
            referrerpolicy="strict-origin-when-cross-origin" 
            allowfullscreen>
        </iframe>
        `
        demo_vid_clicked = true;
    }
});