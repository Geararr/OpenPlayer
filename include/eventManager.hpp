#pragma once

#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <utility>

struct Event {
    int eventTarget;
    int eventId;
    int eventData;
};

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
        //1, 1, 0 = audio - toggle sound - no data
        //std::vector<int> currentEvent = {0, 0, 0};
        std::queue<Event> eventQueue;
        std::mutex eventQueueMutex;
        std::condition_variable eventQueueCV;

    public:
        void fireEvent(Event event);
        Event getEvent(int eventTarget);

        bool isEventQueueEmpty();

        std::mutex& getMutex();
        std::condition_variable& getCV();
};