//
// Created by MatzZze on 05.11.2022.
//

#include "EventHandler.h"

template<class TEvent>
const std::type_info &EventBus::EventHandler<TEvent>::getEventType() const
{
    return typeid(TEvent);
}
