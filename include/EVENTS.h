#pragma once

// Event types and systems

enum EventTarget {
    SoundTarget,
    UiTarget,
};
enum EventId {
    PlaySound,
    PauseSound,
    RaiseVolume,
    LowerVolume,
    SelectSound,
    NextSound,
    PreviousSound,
    ReloadPlaylist,
};