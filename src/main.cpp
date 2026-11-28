// STD stuff
#include <thread>
#include <vector>
#include <exception>
#include <cstdio>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int* attribList);

bool CheckOpenGLSupport(int majorVersion, int minorVersion) {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"DummyGLWindowClass";
    if (!RegisterClassExW(&wc)) return false;

    HWND hwnd = CreateWindowExW(
        0, wc.lpszClassName, L"Dummy", WS_OVERLAPPEDWINDOW,
        0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr
    );
    if (!hwnd) {
      UnregisterClassW(wc.lpszClassName, wc.hInstance);
      return false;
    }

    HDC hdc = GetDC(hwnd);
    if (!hdc) {
      DestroyWindow(hwnd);
      UnregisterClassW(wc.lpszClassName, wc.hInstance);
      return false;
    }

    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (!pf || !SetPixelFormat(hdc, pf, &pfd)) {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return false;
    }

    HGLRC dummyContext = wglCreateContext(hdc);
    if (!dummyContext || !wglMakeCurrent(hdc, dummyContext)) {
        if (dummyContext) wglDeleteContext(dummyContext);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return false;
    }

    auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");

    bool isSupported = false;

    if (wglCreateContextAttribsARB != nullptr && reinterpret_cast<std::intptr_t>(wglCreateContextAttribsARB) != 1 && reinterpret_cast<std::intptr_t>(wglCreateContextAttribsARB) != 2 && reinterpret_cast<std::intptr_t>(wglCreateContextAttribsARB) != 3 && reinterpret_cast<std::intptr_t>(wglCreateContextAttribsARB) != -1) {
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, majorVersion,
            WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        HGLRC modernContext = wglCreateContextAttribsARB(hdc, nullptr, attribs);
        if (modernContext) {
            int result = wglMakeCurrent(hdc, modernContext);
            if (result ) {
              std::cout << "Supported! Version: " << glGetString(GL_VERSION) << "\n";
              std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";

              auto glCrShaderAddress = wglGetProcAddress("glCreateShader");
              if (glCrShaderAddress != nullptr && reinterpret_cast<std::intptr_t>(glCrShaderAddress) != 1 && reinterpret_cast<std::intptr_t>(glCrShaderAddress) != 2 && reinterpret_cast<std::intptr_t>(glCrShaderAddress) != 3 && reinterpret_cast<std::intptr_t>(glCrShaderAddress) != -1) isSupported = true;
            }

            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(modernContext);
        }
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(dummyContext);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return isSupported;
}

#endif

// Project Includes

#include "EVENTS.h"
#include "eventManager.hpp"
#include "soundManager.hpp"
#include "uiManager.hpp"
#include "jsonManager.hpp"
#include "discover.hpp"

std::atomic<bool> programEnd(false);

void eventUpdate(SoundManager *soundManager, UiManager *uiManager, Discover::DiscoverManager *discoverManager, EventManager *eventManager) {

  while (!programEnd.load()) {
    const Event updateEvent = eventManager->getEvent();

    if (!programEnd.load()) {
      switch (updateEvent.eventTarget) {
        case SoundTarget: soundManager->pollEvents(updateEvent); break;
        case UiTarget: uiManager->pollEvents(updateEvent, eventManager, soundManager); break;
        case WebTarget: discoverManager->addTask(updateEvent);
      }
    }
  }
  std::cout << "Event thread end\n";
}

void soundUpdate(SoundManager *soundManager) {

  soundManager->Init(); // initialize sound engine on sound thread

  while (!programEnd.load()) {
    soundManager->soundThreadUpdate(&programEnd);
  }
  std::cout << "Sound thread end\n";
}

void discoverUpdate(Discover::DiscoverManager *discoverManager) {
  while (!programEnd.load()) {
    discoverManager->discoverThreadUpdate(&programEnd);
  }
  std::cout << "Web thread end\n";
}

int main(int argc, char* argv[]) {
    // setup rendering stuff
#ifdef _WIN32
    const std::filesystem::path softwareFlag = SoundData::getDataDir(DataType::Config) / "softwareRendering.flag";
    const std::filesystem::path hardwareFlag = SoundData::getDataDir(DataType::Config) / "hardwareRendering.flag";

    if (std::filesystem::exists(softwareFlag)) {
      _putenv_s("SLINT_BACKEND", "winit-software");
    }
    else if (std::filesystem::exists(hardwareFlag)) {
      _putenv_s("SLINT_BACKEND", "winit-femtovg");
    }
    else { // if no flag the commence the check
      if (CheckOpenGLSupport(3, 3)) {
        _putenv_s("SLINT_BACKEND", "winit-femtovg");
        std::ofstream(hardwareFlag).close();
      }
      else {
        _putenv_s("SLINT_BACKEND", "winit-software");
        std::ofstream(softwareFlag).close();
      }
    }
#else
    setenv("SLINT_BACKEND", "winit-femtovg", 1);
#endif

  // VIPS SETUP
  if (VIPS_INIT(argv[0])) {
   vips_error_exit(nullptr);
  }
  vips_cache_set_max(0);
  vips_cache_set_max_mem(0);
  vips_concurrency_set(1);

  ConfigManager configManager;
  const std::filesystem::path configPath = SoundData::getDataDir(DataType::Config) / "config/config.json";
  configManager.setConfigPath(configPath);

  // STARTUP

  CacheManager cacheManager;
  const std::filesystem::path cachePath = SoundData::getDataDir(DataType::Cache) / "cache/cache.json";
  cacheManager.setCachePath(cachePath);
  cacheManager.loadCache();

  SoundData::setCacheManager(&cacheManager);
  SoundData::setConfigManager(&configManager);

  EventManager eventManager;

  SoundManager soundManager{&eventManager, &cacheManager};

  std::unordered_set<std::string> allActiveAssets;

  std::error_code ec;
  for (const std::filesystem::path& entry :  soundManager.getPlaylists()) {
    const std::string playlistName = entry.filename().string();
    for (const auto& file : std::filesystem::directory_iterator(entry, ec)) {
      if (file.is_regular_file(ec)) {
        allActiveAssets.insert(playlistName + "/" + file.path().filename().string());
      }
    }
  }
  if (!allActiveAssets.empty()) {
    cacheManager.pruneAll(allActiveAssets);
    cacheManager.saveCache();
  }

  // STARTUP END

  Discover::DiscoverManager discoverManager;

  discoverManager.setManagers(&configManager, &eventManager);

  UiManager uiManager(&soundManager, &configManager, &discoverManager); // load config in uiManager
  uiManager.setupEvents(&eventManager, &soundManager);

  std::thread soundThread(soundUpdate, &soundManager);
  std::thread eventThread(eventUpdate, &soundManager, &uiManager, &discoverManager, &eventManager);
  std::thread webThread(discoverUpdate, &discoverManager);

  uiManager.Run();

  programEnd.store(true);
  eventManager.fireEvent({ .eventData = -1, .eventTarget = -1, .eventId = -1 });

  soundManager.END();
  eventManager.END();
  discoverManager.END();

  if (eventThread.joinable()) {
    eventThread.join();
  }
  if (soundThread.joinable()) {
    soundThread.join();
  }
  if (webThread.joinable()) {
    webThread.join();
  }

  soundManager.unloadSound(); // make sure no sounds are loaded

  vips_shutdown();

  std::cout << "Program end\n";

  return 0;
}