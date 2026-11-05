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

            if (ui->get_toggle_button() == ICON_PLAY) {
                if (soundManager->getSound(0) == nullptr) {
                    std::cout << "Playlist empty/No sound selected." << std::endl;
                }
                else {
                    eventManager->fireEvent(1, 1, 0);
                    std::cout << "Playing\n";
                    ui->set_toggle_button(ICON_PAUSE);
                }
            }
            else {
                if (soundManager->getSound(0) == nullptr) {
                    std::cout << "Playlist empty/No sound selected." << std::endl;
                }
                else {
                    eventManager->fireEvent(1, 1, 0);
                    std::cout << "Paused\n";
                    ui->set_toggle_button(ICON_PLAY);
                }
            }
        }
    });

    ui->on_playlist_set([eventManager, soundManager, ui_weak](const int index) {
        if (const auto ui_strong = ui_weak.lock()) {
            eventManager->fireEvent(1, 6, index);

            const auto& ui = *ui_strong;

            if (soundManager->getSound(0) == nullptr) {
                std::cout << "Checked playlist as empty.\n";
                ui->set_current_sound("Playlist empty...");
                ui->set_current_image(slint::Image{});
                ui->set_toggle_button(ICON_PLAY);
            }
            else {
                ui->set_toggle_button(ICON_PAUSE);

                const auto song_model = std::make_shared<slint::VectorModel<PendingSong>>();
                song_model->push_back(PendingSong{
                    .image = soundManager->getSound(0)->getCoverImage(),
                    .title = soundManager->getSound(0)->getSoundName().data()
                });

                ui->set_pendingSongs(song_model);
            }
        }
    });
}

void UiManager::Run() {
    ui->run();
}
