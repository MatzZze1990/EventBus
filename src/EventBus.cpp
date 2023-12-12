//
// Created by MatzZze on 05.11.2022.
//

#include "EventBus.h"
#include "Event.h"

using EventBus::EBHandlerID;
using EventBus::Event;

std::unique_ptr<EventBus::EventBus> EventBus::EventBus::instance;

EventBus::EventBus::EventBus()
{
    this->idCounter = 1;
    this->stop = false;
    bool initialized = false;
    std::unique_lock<std::mutex> lock(EventBus::threadMtx);
    this->threadPtr = std::make_unique<std::thread>(&EventBus::handlingThread, this, std::ref(initialized));
    EventBus::threadCond.wait(lock, [&]{return initialized;});
}

EventBus::EventBus::~EventBus()
{
    //scope necessary to release locks before threadPtr->join()
    {
        std::lock_guard<std::mutex> lockEvent(this->queueMtx);
        this->stop = true;
        std::lock_guard<std::mutex> lockCond(this->condMtx);
        this->eventCond.notify_one();
    }

    this->threadPtr->join();
    std::unique_lock<std::mutex> lockRegistrations(this->registrationLock);
    std::unique_lock<std::mutex> lockTmpInsertions(this->tmpRegistrationsMtx);
    std::unique_lock<std::mutex> lockTmpRemovals(this->tmpRegistrationsRemovalsMtx);
    this->registrations.clear();
    this->temporaryRegistrations.clear();
    this->temporaryRegistrationsRemovals.clear();
}

EventBus::EventBus *EventBus::EventBus::getInstance()
{
    if (!EventBus::instance)
    {
        EventBus::instance = std::unique_ptr<EventBus>(new EventBus());
    }

    return EventBus::instance.get();
}

void EventBus::EventBus::cleanUp()
{
    EventBus::instance.reset();
}

void EventBus::EventBus::unregisterHandler(const EBHandlerID id)
{
    std::lock_guard<std::mutex> lock(this->tmpRegistrationsRemovalsMtx);
    this->temporaryRegistrationsRemovals.push_back(id);
}

void EventBus::EventBus::dispatchEvent(std::shared_ptr<Event> &event) const
{
    for (const auto &registration : registrations)
    {
        if (event->getType() == registration.second->getEventType())
        {
            registration.second->dispatch(event);
        }
    }
}

void EventBus::EventBus::fire(std::shared_ptr<Event> &event)
{
    std::lock_guard<std::mutex> lock(this->registrationLock);
    handleInsertionsAndRemovals();
    dispatchEvent(event);
}

void EventBus::EventBus::fireAndForget(const std::shared_ptr<Event> &event)
{
    std::lock_guard<std::mutex> queueLock(this->queueMtx);
    std::lock_guard<std::mutex> condLock(this->condMtx);
    this->eventQueue.push_back(event);
    this->eventCond.notify_one();
}

void EventBus::EventBus::handlingThread(bool &initialized)
{
    {
        std::lock_guard<std::mutex> lock(EventBus::threadMtx);
        initialized = true;
        EventBus::threadCond.notify_all();
    }

    while(!this->stop)
    {
        std::unique_lock<std::mutex> lock(this->condMtx);
        this->eventCond.wait(lock, [&]{ return !eventQueue.empty() || stop; });
        if (this->stop)
        {
            break;
        }

        handleInsertionsAndRemovals();
        std::lock_guard<std::mutex> queueLock(this->queueMtx);
        std::lock_guard<std::mutex> registrationsLock(this->registrationLock);
        for (auto &event: this->eventQueue)
        {
            this->dispatchEvent(event);
        }

        this->eventQueue.clear();
    }
}

void EventBus::EventBus::handleInsertionsAndRemovals()
{
    std::lock_guard<std::mutex> tmpInsertionsLock(tmpRegistrationsMtx);
    for (auto &insertion : temporaryRegistrations)
    {
        registrations.insert(insertion);
    }

    temporaryRegistrations.clear();
    std::lock_guard<std::mutex> tmpRemovalsLock(tmpRegistrationsRemovalsMtx);
    for (auto &removal : temporaryRegistrationsRemovals)
    {
        registrations.erase(removal);
    }

    temporaryRegistrationsRemovals.clear();
}

EBHandlerID EventBus::EventBus::registerHandler(std::shared_ptr<EventHandlerBase> &handler)
{
    std::lock_guard<std::mutex> lock(this->tmpRegistrationsMtx);
    const auto insertion = std::make_pair(this->idCounter, handler);
    this->temporaryRegistrations.push_back(insertion);
    return this->idCounter++;
}
