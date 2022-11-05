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
    this->threadPtr = std::make_unique<std::thread>(&EventBus::handlingThread, this);
}

EventBus::EventBus::~EventBus()
{
    this->stop = true;
    this->eventCond.notify_one();
    this->threadPtr->join();
    std::unique_lock<std::mutex> lock(this->registrationLock);
    this->registrations.clear();
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

template<class TEvent>
EBHandlerID EventBus::EventBus::registerHandler(std::unique_ptr<EventHandler<TEvent>> handler)
{
    static_assert(std::is_base_of<Event, TEvent>::value, "EventHandler template argument is not an derived class of EventBus::Event");
    std::lock_guard<std::mutex> lock(this->registrationLock);
    this->registrations.insert(this->idCounter, handler);
    return this->idCounter++;
}

void EventBus::EventBus::unregisterHandler(EBHandlerID id)
{
    std::lock_guard<std::mutex> lock(this->registrationLock);
    this->registrations.erase(id);
}

void EventBus::EventBus::dispatchEvent(std::shared_ptr<Event> &event)
{
    for (const auto &registration : registrations)
    {
        if (event->getType() == registration.second->getEventType())
        {
            registration.second->onEvent(event);
        }
    }
}

void EventBus::EventBus::fire(std::shared_ptr<Event> &event)
{
    std::lock_guard<std::mutex> lock(this->registrationLock);
    dispatchEvent(event);
}

void EventBus::EventBus::fireAndForget(std::shared_ptr<Event> &event)
{
    std::lock_guard<std::mutex> queueLock(this->queueMtx);
    this->eventQueue.push_back(event);
    this->eventCond.notify_one();
}

void EventBus::EventBus::handlingThread()
{
    while(!this->stop)
    {
        std::unique_lock<std::mutex> lock(this->condMtx);
        if (this->eventCond.wait_for(lock, std::chrono::seconds(5), [&] {return !eventQueue.empty();}))
        {
            std::lock_guard<std::mutex> queueLock(this->queueMtx);
            std::lock_guard<std::mutex> registrationsLock(this->registrationLock);
            for (auto &event: this->eventQueue)
            {
                this->dispatchEvent(event);
            }

            this->eventQueue.clear();
        } // else timeout
    }
}
