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
    std::unique_lock<std::mutex> lock(this->threadMtx);
    this->threadCond.wait(lock);
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
            registration.second->dispatch(event);
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
    this->threadCond.notify_all();
    while(!this->stop)
    {
        std::unique_lock<std::mutex> lock(this->condMtx);
        if (this->eventCond.wait_for(lock, std::chrono::seconds(5), [&] {return !eventQueue.empty() || stop;}))
        {
            if (this->stop)
            {
                break;
            }

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

EBHandlerID EventBus::EventBus::registerHandler(std::shared_ptr<EventHandlerBase> &handler)
{
    std::lock_guard<std::mutex> lock(this->registrationLock);
    auto insertion = std::make_pair(this->idCounter, std::static_pointer_cast<EventHandlerBase>(handler));
    this->registrations.insert(insertion);
    return this->idCounter++;
}
