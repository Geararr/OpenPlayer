#include "eventManager.hpp"
#include <iostream>

void EventManager::fireEvent(const Event event) {
    std::lock_guard<std::mutex> lock(eventQueueMutex);

    eventQueue.push(event);

    eventQueueCV.notify_one();
}

Event EventManager::getEvent() {

    auto token = std::unique_lock<std::mutex>(eventQueueMutex);

    eventQueueCV.wait(token, [this] { return !eventQueue.empty(); });

    const Event event = eventQueue.front();

    eventQueue.pop();

    return event;
}

bool EventManager::isEventQueueEmpty() {
    auto token = std::unique_lock<std::mutex>(eventQueueMutex);

    const bool empty = eventQueue.empty();

    return empty;
}

std::mutex& EventManager::getMutex() {
    return eventQueueMutex;
}

std::condition_variable& EventManager::getCV() {
    return eventQueueCV;
}

void EventManager::END() {
    constexpr Event endEvent{ .eventData = -1, .eventTarget = -1, .eventId = -1 };

    fireEvent(endEvent);
}