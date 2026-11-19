#pragma once

// Event types and systems

enum EventTarget {
    SoundTarget,
    UiTarget,
};
enum EventId {
    PlaySound,
    PauseSound,
    SetVolume,
    SelectSound,
    NextSound,
    PreviousSound,
    ReloadPlaylist,
    StartProgressTimer,
    StopProgressTimer,
    Seek
};