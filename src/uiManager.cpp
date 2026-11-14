#include "uiManager.hpp"
#include "IconsProto.h"

#include <slint/slint.h>
#include <iostream>

#include <thread>
#include <atomic>
#include <chrono>

// ui->set_current_sound(soundManager->getSound(0)->getSoundName().data());
// ui->set_current_image(soundManager->getSound(0)->getCoverImage());

UiManager::UiManager(SoundManager* soundManager) : ui(OpenPlayerUI::create()) {
    ui->set_current_sound("No song selected...");
    ui->set_current_image(slint::Image{});
    ui->set_toggle_button(ICON_PLAY);

    const auto playlist_model = std::make_shared<slint::VectorModel<Playlist>>();
    for (const auto& playlist : soundManager->getPlaylists()) {
        playlist_model->push_back(Playlist{
            .title = playlist.data()
        });
    }

    ui->set_playlists(playlist_model);
}

void UiManager::setupEvents(EventManager* eventManager, SoundManager* soundManager) {

    auto ui_weak = slint::ComponentWeakHandle(ui);

    ui->on_toggle_music([eventManager, soundManager, ui_weak]() {
        if (const auto ui_strong = ui_weak.lock()) {

            const auto& ui = *ui_strong;

            if (soundManager->getSound(0) != nullptr) {
                Event event{ .eventTarget = SoundTarget, .eventId = -1, .eventData = 0 };

                if (soundManager->getPlayingStatus()) { // playing
                    event.eventId = PauseSound;
                    ui->set_toggle_button(ICON_PLAY);
                }
                else { // paused
                    event.eventId = PlaySound;
                    ui->set_toggle_button(ICON_PAUSE);
                }

                eventManager->fireEvent(event);
            }
            else {
                std::cout << "Playlist empty/No sound selected\n";
            }
        }
    });

    ui->on_playlist_set([eventManager, ui_weak](const int index) {
        if (const auto ui_strong = ui_weak.lock()) {
            const Event event{ .eventTarget = SoundTarget, .eventId = ReloadPlaylist, .eventData = index };
            eventManager->fireEvent(event);
        }
    });

    ui->on_song_set([eventManager, ui_weak](const int index) {
        if (const auto ui_strong = ui_weak.lock()) {
            const auto& ui = *ui_strong;

            const Event event{ .eventTarget = SoundTarget, .eventId = SelectSound, .eventData = index };
            eventManager->fireEvent(event);
        }
    });

    ui->on_next_song([eventManager, soundManager, ui_weak]() {
        if (const auto ui_strong = ui_weak.lock()) {
            if (soundManager->getSound(0) != nullptr) {
                constexpr Event event{ .eventTarget = SoundTarget, .eventId = NextSound, .eventData = 0 };
                eventManager->fireEvent(event);
            }
            else {
                std::cout << "Playlist empty/No sound selected\n";
            }
        }
    });
    ui->on_previous_song([eventManager, soundManager, ui_weak]() {
        if (const auto ui_strong = ui_weak.lock()) {
            if (soundManager->getSound(0) != nullptr) {
                constexpr Event event{ .eventTarget = SoundTarget, .eventId = PreviousSound, .eventData = 0 };
                eventManager->fireEvent(event);
            }
            else {
                std::cout << "Playlist empty/No sound selected\n";
            }
        }
    });
}

slint::ComponentWeakHandle<OpenPlayerUI> UiManager::getWeakHandle() {
    return slint::ComponentWeakHandle<OpenPlayerUI>(ui);
}

void UiManager::pollEvents(const Event event, EventManager* eventManager,SoundManager* soundManager) { // we have to use slint::invoke_from_event_loop because func is on backround thread
    switch (event.eventId) {
        case ReloadPlaylist: // reload playlist

            if (soundManager->getSound(0) == nullptr) {
                std::cout << "Checked playlist as empty.\n";

                auto ui_weak = getWeakHandle();

                slint::invoke_from_event_loop([ui_weak]() {
                    if (const auto ui_strong_opt = ui_weak.lock()) {

                        const auto& ui_strong = *ui_strong_opt;

                        ui_strong->set_current_sound("Playlist empty...");
                        ui_strong->set_current_image(slint::Image{});
                        ui_strong->set_toggle_button(ICON_PLAY);
                    }
                });
            }
            else {
                auto ui_weak = getWeakHandle();

                slint::invoke_from_event_loop([ui_weak, soundManager]() {
                    if (const auto ui_strong_opt = ui_weak.lock()) {

                        // load current/first song in playlist

                        const auto& ui_strong = *ui_strong_opt;

                        const std::string soundName = soundManager->getSound(0)->getSoundName();
                        const slint::Image soundImage = soundManager->getSound(0)->getCoverImage();

                        ui_strong->set_current_sound(slint::SharedString(soundName));
                        ui_strong->set_current_image(soundImage);
                        ui_strong->set_toggle_button(ICON_PLAY);

                        // load all other songs (if any)
                        const auto pendingSongs = std::make_shared<slint::VectorModel<PendingSong>>();

                        for (int i = 0; i < soundManager->getPlaylistSize(); ++i) {
                            const std::string pendingSoundName = soundManager->getSound(i)->getSoundName();
                            const slint::Image pendingSoundImage = soundManager->getSound(i)->getCoverImage();

                            pendingSongs->push_back(PendingSong{
                                .image = pendingSoundImage,
                                .title = slint::SharedString(pendingSoundName)
                            });
                        }

                        ui_strong->set_pendingSongs(pendingSongs);
                    }
                });
            }
            break;

        case SelectSound: // set song or select sound
            auto ui_weak = getWeakHandle();

            slint::invoke_from_event_loop([ui_weak, soundManager, event]() {
                if (const auto ui_strong_opt = ui_weak.lock()) {

                    const auto& ui_strong = *ui_strong_opt;

                    const std::string soundName = soundManager->getSound(event.eventData)->getSoundName();
                    const slint::Image soundImage = soundManager->getSound(event.eventData)->getCoverImage();

                    ui_strong->set_current_sound(slint::SharedString(soundName));
                    ui_strong->set_current_image(soundImage);
                }
            });
        break;
    }
}

void UiManager::Run() {
    ui->run();
}
