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

        // 0 = event-type 1 = event 2 = event datas
        //110 = audio - toggle sound - no data
        int currentEvent = 000;

    public:
        void fireEvent(int eventData);

        int getEvent();
};