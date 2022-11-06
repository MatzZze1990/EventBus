# EventBus
A simple EventBus for c++

# Build status
## Debian 10 (x86_64) and Debian 11 (x86_64)
![TeamCity build status](https://teamcity.s4.matzzze.de/app/rest/builds/buildType:id:GitHubProjects_EventBus_LinuxBuild/statusIcon.svg)

## Windows 10 x64
![TeamCity build status](https://teamcity.s4.matzzze.de/app/rest/builds/buildType:id:GitHubProjects_EventBus_WindowsBuild/statusIcon.svg)

# Description
A simple c++ EventBus implementation.
Create custom events and their corresponding event handler. Events can either be sent in blocking mode (```fire()```).
The calling thread will call the registered event handlers and returns when all handlers have processed the event.
There is also an async implementation which is used when firing events via ```fireAndForget()```. The EventBus has an internal thread which gets signaled when an event arrives.
It is safe to call ```registerHandler()``` and ```unregisterHandler()``` from within an event handler.
It is not safe to call ```cleanUp()``` from within an event handler and doing so will result in a deadlock.

# Binary packages
There are currently no binary package supported.

# Compilation
CMake is used as makefile generator.
Note: conan is used for dependency management. (Only googletest for unittests)
## Linux
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

#call ./test to execute unittests
```
## Windows
```shell
mkdir buildx64
cd buildx64
cmake -A x64 -B . -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

#call Release\test.exe to execute unittests
```

# Example usage
## Event defintion
```c++
#include <Event.h>

class exampleEvent : public EventBus::Event
{
public:
    //constructor
    explicit exampleEvent(int x) : x(x) {}
    //destructor
    virtual ~exampleEvent() = default;
    
    // override of getType. Used to direct events to their handler
    const std::type_info &getType() override
    {
        return typeid(testEvent);
    }

    // custom getter
    int getX() const {return x;}

protected:
    
    //custom member
    int x;
};
```

## Example Handler
```c++
#include <EventHandler.h>
#include <exampleEvent.h>

//template argument of EventHandler specifies the event which will be handled
class exampleHandler : public EventBus::EventHandler<exampleEvent>
{
public:
    //constructor
    exampleHandler() : x(0) {}
    
    //destructor
    virtual ~exampleHandler() = default;
    
    //Callback method for incoming events
    void onEvent(std::shared_ptr<exampleEvent> &event) override
    {
        //cast base event to real event object
        auto casted = std::static_pointer_cast<exampleEvent>(event);
        
        //custom data processing
        this->x += casted->getX();
    }
    
    //custom getter
    int getX() const {return  this->x;}

protected:
    //custom member
    int x;
};
```

## Example usage
```c++
#include <EventBus.h>
#include <exampleHandler.h>
    
int main()
{
    //create a new handler
    auto handler = std::make_shared<exampleHandler>();
    
    //cast handler to base ancestor object
    auto baseHandler = std::static_pointer_cast<EventBus::EventHandlerBase>(handler);
    
    //register event handler in event bus
    auto handleID = EventBus::EventBus::getInstance()->registerHandler(baseHandler);
    
    //create an example event
    auto event = std::make_shared<exampleEvent>(5);
    
    //cast event to base ancestor object
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);
    
    //fire the event
    //fire() blocks and the calling thread executes the event
    //for async event handling use fireAndForget()
    EventBus::EventBus::getInstance()->fire(baseEvent);

    //result is now 5
    auto result = handler->getX();
    
    //unregister event handler
    EventBus::EventBus::getInstance()->unregisterHandler(handleID);
    
    //complete cleanup of EventBus
    EventBus::EventBus::cleanUp();
    
    return 0;
}
```