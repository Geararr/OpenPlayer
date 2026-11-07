#pragma once

#include <string>

inline constexpr std::string_view UI_RECIEVED_EVENT = "UI -> RECIEVED EVENT\n";
inline constexpr std::string_view UI_SENT_EVENT = "UI -> SENT EVENT\n";
inline constexpr std::string_view UI_EXECUTED_EVENT = "UI -> EXECUTED EVENT\n";

inline constexpr std::string_view SOUND_RECIEVED_EVENT = "SOUND-MANAGER -> RECIEVED EVENT\n";
inline constexpr std::string_view SOUND_SENT_EVENT = "SOUND-MANAGER -> SENT EVENT\n";
inline constexpr std::string_view SOUND_EXECUTED_EVENT = "SOUND-MANAGER -> EXECUTED EVENT\n";