//
// Created by MatzZze on 05.11.2022.
//

#ifndef EVENTBUS_EVENTBUS_H
#define EVENTBUS_EVENTBUS_H

#include <memory>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include "EventHandler.h"
#include "Event.h"

namespace EventBus
{

    typedef size_t EBHandlerID;

    class EventBus
    {
    public:
        static EventBus *getInstance();
        static void cleanUp();

        virtual ~EventBus();

        EBHandlerID registerHandler(std::shared_ptr<EventHandlerBase> &handler);
        void unregisterHandler(EBHandlerID id);

        void fire(std::shared_ptr<Event> &event);
        void fireAndForget(std::shared_ptr<Event> &event);
    protected:
        void handlingThread();
        void dispatchEvent(std::shared_ptr<Event> &event);

        EBHandlerID idCounter;
        std::unordered_map<EBHandlerID, std::shared_ptr<EventHandlerBase>> registrations;
        std::mutex registrationLock;
        std::mutex condMtx;
        std::mutex queueMtx;
        std::mutex threadMtx;
        std::condition_variable eventCond;
        std::condition_variable threadCond;
        std::unique_ptr<std::thread> threadPtr;
        std::vector<std::shared_ptr<Event>> eventQueue;
        bool stop;

    private:
        EventBus();
        static std::unique_ptr<EventBus> instance;
    };

} // EventBus

#endif //EVENTBUS_EVENTBUS_H
