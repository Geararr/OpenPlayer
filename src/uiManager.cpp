#include "uiManager.hpp"
#include "IconsProto.h"

#include <slint/slint.h>
#include <iostream>

#include <thread>
#include <atomic>
#include <chrono>

UiManager::UiManager(SoundManager* soundManager) : ui(OpenPlayerUI::create()) {
    ui->set_current_sound("No song selected...");
    ui->set_current_image(slint::Image{});
    ui->set_toggle_button(ICON_PLAY);

    ui->set_is_playing(false);

    const auto playlist_model = std::make_shared<slint::VectorModel<slint::SharedString>>();
    for (const auto& playlist : soundManager->getPlaylists()) {
        playlist_model->push_back(static_cast<slint::SharedString>(playlist.filename().string()));
    }

    ui->set_playlists(playlist_model);
}

void UiManager::setupEvents(EventManager* eventManager, SoundManager* soundManager) {

    auto ui_weak = slint::ComponentWeakHandle(ui);

    ui->on_toggle_music([eventManager, soundManager, ui_weak]() {
        if (const auto ui_strong = ui_weak.lock()) {

            const auto& ui = *ui_strong;

            Event event{ .eventData = 0, .eventTarget = SoundTarget, .eventId = -1 };

            if (soundManager->getPlayingStatus()) { // playing
                event.eventId = PauseSound;
                ui->set_toggle_button(ICON_PLAY);
                ui->set_is_playing(false);
            }
            else { // paused
                event.eventId = PlaySound;
                ui->set_toggle_button(ICON_PAUSE);
                ui->set_is_playing(true);
            }
            eventManager->fireEvent(event);
        }
    });

    ui->on_playlist_set([eventManager, ui_weak](const int index) {
        eventManager->fireEvent({ .eventData = index, .eventTarget = SoundTarget, .eventId = ReloadPlaylist });
    });

    ui->on_song_set([eventManager, ui_weak](const int index) {
        eventManager->fireEvent({ .eventData = index, .eventTarget = SoundTarget, .eventId = SelectSound });
    });

    ui->on_next_song([eventManager, soundManager, ui_weak]() {
        eventManager->fireEvent({ .eventData = 0, .eventTarget = SoundTarget, .eventId = NextSound });
    });
    ui->on_previous_song([eventManager, soundManager, ui_weak]() {
        eventManager->fireEvent({ .eventData = 0, .eventTarget = SoundTarget, .eventId = PreviousSound });
    });

    // Volume
    ui->on_volume_changed([eventManager, soundManager, ui_weak](const float volume) {
        eventManager->fireEvent({ .eventData = volume, .eventTarget = SoundTarget, .eventId = SetVolume });
    });
}

slint::ComponentWeakHandle<OpenPlayerUI> UiManager::getWeakHandle() {
    return slint::ComponentWeakHandle<OpenPlayerUI>(ui);
}

void UiManager::pollEvents(const Event event, EventManager* eventManager,SoundManager* soundManager) { // we have to use slint::invoke_from_event_loop because func is on backround thread

    switch (event.eventId) {
        case ReloadPlaylist: {
            // reload playlist

            struct TrackSnapshot {
                std::string name;
                slint::Image image;
            };

            std::vector<TrackSnapshot> tracks;
            const size_t count = soundManager->getPlaylistSize();
            tracks.reserve(count);

            for (int i = 0; i < count; ++i) {
                if (auto* s = soundManager->getSound(i)) {
                    tracks.push_back({ .name = s->getSoundName(), .image = s->getCoverImage()});
                }
            }

            auto ui_weak = getWeakHandle();

            if (tracks.empty()) {
                std::cout << "Checked playlist as empty.\n";

                slint::invoke_from_event_loop([ui_weak]() {
                    if (const auto ui_strong_opt = ui_weak.lock()) {
                        const auto& ui_strong = *ui_strong_opt;

                        ui_strong->set_current_sound("Playlist empty...");
                        ui_strong->set_current_image(slint::Image{});
                        ui_strong->set_toggle_button(ICON_PLAY);
                        ui_strong->set_is_playing(false);

                        ui_strong->set_pendingSongs(std::make_shared<slint::VectorModel<PendingSong>>());

                        ui_strong->set_current_time_text(slint::SharedString("0:00"));
                        ui_strong->set_total_time_text(slint::SharedString("0:00"));
                    }
                });
            }
            else {
                const float duration = soundManager->getSoundDuration();
                const auto durationStr = soundManager->getSoundDurationString();

                slint::invoke_from_event_loop([ui_weak, tracks = std::move(tracks), duration, durationStr]() {
                    if (const auto ui_strong_opt = ui_weak.lock()) {
                        const auto& ui_strong = *ui_strong_opt;

                        // load current/first song in playlist
                        ui_strong->set_current_sound(slint::SharedString(tracks[0].name));
                        ui_strong->set_current_image(tracks[0].image);
                        ui_strong->set_toggle_button(ICON_PLAY);
                        ui_strong->set_is_playing(false);

                        // load all other songs (if any)
                        const auto pendingSongs = std::make_shared<slint::VectorModel<PendingSong>>();
                        for (const auto& track : tracks) {
                            pendingSongs->push_back(PendingSong{
                                .image = track.image,
                                .title = static_cast<slint::SharedString>(track.name)
                            });
                        }
                        ui_strong->set_pendingSongs(pendingSongs);

                        ui_strong->set_current_time_text(slint::SharedString("0:00"));
                        ui_strong->set_total_time_text(durationStr);
                    }
                });
            }
        } break;

        case SelectSound: {
            // set song or select sound
            const int index = std::get<int>(event.eventData);
            auto* selectedSound = soundManager->getSound(index);

            if (selectedSound == nullptr) {
                std::cerr << "Selected sound -> invalid index " << index << '\n';
                break;
            }

            const std::string soundName = selectedSound->getSoundName();
            const slint::Image soundImage = selectedSound->getCoverImage();
            const auto durationStr = soundManager->getSoundDurationString();

            auto ui_weak = getWeakHandle();

            slint::invoke_from_event_loop([ui_weak, soundName, soundImage, durationStr]() {
                if (const auto ui_strong_opt = ui_weak.lock()) {

                    const auto& ui_strong = *ui_strong_opt;

                    ui_strong->set_current_sound(slint::SharedString(soundName));
                    ui_strong->set_current_image(soundImage);

                    ui_strong->set_current_time_text(slint::SharedString("0:00"));
                    ui_strong->set_total_time_text(durationStr);
                }
            });
        } break;

        case StartProgressTimer: {
            auto ui_weak = getWeakHandle();

            slint::invoke_from_event_loop([this, ui_weak, soundManager]() {
                progressCheckTimer.start(
                    slint::TimerMode::Repeated,
                    std::chrono::milliseconds(200),
                    [ui_weak, soundManager]() {
                        if (const auto ui_strong_opt = ui_weak.lock()) {
                            const auto& ui_strong = *ui_strong_opt;
                            ui_strong->set_current_time_text(soundManager->getSoundProgressString());
                        }
                    }
                );
            });
        } break;

        case StopProgressTimer: {
            auto ui_weak = getWeakHandle();

            slint::invoke_from_event_loop([this, ui_weak, soundManager]() {
                progressCheckTimer.stop();
            });
        } break;
    }
}

void UiManager::Run() {
    ui->run();
}
