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

        template<class TEvent>
        EBHandlerID registerHandler(std::unique_ptr<EventHandler<TEvent>> handler);
        void unregisterHandler(EBHandlerID id);

        void fire(std::shared_ptr<Event> &event);
        void fireAndForget(std::shared_ptr<Event> &event);
    protected:
        void handlingThread();
        void dispatchEvent(std::shared_ptr<Event> &event);

        EBHandlerID idCounter;
        std::unordered_map<EBHandlerID, std::unique_ptr<EventHandler<Event>>> registrations;
        std::mutex registrationLock;
        std::mutex condMtx;
        std::mutex queueMtx;
        std::condition_variable eventCond;
        std::unique_ptr<std::thread> threadPtr;
        std::vector<std::shared_ptr<Event>> eventQueue;
        bool stop;

    private:
        EventBus();
        static std::unique_ptr<EventBus> instance;
    };

} // EventBus

#endif //EVENTBUS_EVENTBUS_H
