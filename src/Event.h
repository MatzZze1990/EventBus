//
// Created by MatzZze on 05.11.2022.
//

#ifndef EVENTBUS_EVENT_H
#define EVENTBUS_EVENT_H

namespace EventBus
{
    class Event
    {
    public:
        virtual const std::type_info &getType() = 0;
    };
}

#endif //EVENTBUS_EVENT_H
