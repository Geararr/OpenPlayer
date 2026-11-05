#pragma once

#include <vector>

class EventManager
{
    private:
    
        /*
            type = system-type

            1 - Audio
            2 - UI
        */

        /* 1:
        1 - toggle sound
        2 - increase volume
        3 - decrease volume
        4 - next song
        5 - previous song
        6 - Reload Playlist
        */
        /* 2:
        1 - reload playlist
        2 - next song
        3 - previous song
        4 - change volume
        */

        // 0 = eventTarget 1 = eventId 2 = eventData
        //110 = audio - toggle sound - no data
        std::vector<int> currentEvent = {0, 0, 0};

    public:
        void fireEvent(int eventTarget, int eventId, int eventData);

        std::vector<int> getEvent(int eventTarget);
};