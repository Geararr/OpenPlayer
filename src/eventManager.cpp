#include "eventManager.hpp"
#include <iostream>

void EventManager::fireEvent(const Event event) {
    std::lock_guard<std::mutex> lock(eventQueueMutex);

    eventQueue.push(event);

    eventQueueCV.notify_one();
}

Event EventManager::getEvent(const int eventTarget) {
    Event event{.eventTarget = 0, .eventId = 0, .eventData = 0};

    auto token = std::unique_lock<std::mutex>(eventQueueMutex);

    eventQueueCV.wait(token, [this] { return !eventQueue.empty(); });

    if (eventQueue.front().eventTarget == eventTarget) {
        const int eventId = eventQueue.front().eventId;
        const int eventData = eventQueue.front().eventData;

        eventQueue.pop();

        token.unlock();

        event.eventId = eventId;
        event.eventData = eventData;

        return event;
    }

    return event;
}

bool EventManager::isEventQueueEmpty() {
    return eventQueue.empty();
}

std::mutex& EventManager::getMutex() {
    return eventQueueMutex;
}

std::condition_variable& EventManager::getCV() {
    return eventQueueCV;
}