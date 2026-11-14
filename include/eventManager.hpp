#pragma once

#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <utility>
#include "EVENTS.h"

struct Event {
    int eventTarget;
    int eventId;
    int eventData;
};

class EventManager
{
    private:

        // 0 = eventTarget 1 = eventId 2 = eventData
        //1, 1, 0 = audio - toggle sound - no data
        //std::vector<int> currentEvent = {0, 0, 0};
        std::queue<Event> eventQueue;
        std::mutex eventQueueMutex;
        std::condition_variable eventQueueCV;

    public:
        void fireEvent(Event event);
        Event getEvent(EventTarget eventTarget);

        bool isEventQueueEmpty();

        std::mutex& getMutex();
        std::condition_variable& getCV();

        void END();
};