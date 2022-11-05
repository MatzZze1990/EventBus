//
// Created by MatzZze on 05.11.2022.
//

#ifndef EVENTBUS_EVENTHANDLER_H
#define EVENTBUS_EVENTHANDLER_H

#include <typeinfo>
#include <memory>

namespace EventBus
{
    template<class TEvent>
    class EventHandler
    {
    public:
        virtual void onEvent(std::shared_ptr<TEvent> &event) = 0;
        virtual const std::type_info &getEventType() const;
    };

} // EventBus

#endif //EVENTBUS_EVENTHANDLER_H
