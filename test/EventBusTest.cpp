//
// Created by MatzZze on 05.11.2022.
//

#include <gtest/gtest.h>
#include "src/EventBus.h"
#include <typeinfo>

class testEvent : public EventBus::Event
{
public:
    testEvent(int x) : x(x) {}
    virtual ~testEvent() = default;
    const std::type_info &getType() override
    {
        return typeid(testEvent);
    }

    int getX() const {return x;}

protected:
    int x;
};

class testEvent2 : public EventBus::Event
{
public:
    testEvent2(int x) : x(x) {}
    virtual ~testEvent2() = default;
    const std::type_info &getType() override
    {
        return typeid(testEvent2);
    }

    int getX() const {return x;}

protected:
    int x;
};

class testHandler : public EventBus::EventHandler<testEvent>
{
public:
    testHandler() : x(0) {}
    void onEvent(std::shared_ptr<testEvent> &event) override
    {
        auto casted = std::static_pointer_cast<testEvent>(event);
        this->x += casted->getX();
    }


    int getX() const {return  this->x;}

protected:
    int x;
};

class testHandler2 : public EventBus::EventHandler<testEvent2>
{
public:
    testHandler2() : x(0) {}
    void onEvent(std::shared_ptr<testEvent2> &event) override
    {
        auto casted = std::static_pointer_cast<testEvent2>(event);
        this->x += casted->getX();
    }

    int getX() const {return  this->x;}

protected:
    int x;
};

class asyncHandler : public EventBus::EventHandler<testEvent>
{
public:
    asyncHandler(std::condition_variable &cv, std::mutex &mtx, bool &processed) :cv(cv), mtx(mtx), processed(processed), x(0) {}
    void onEvent(std::shared_ptr<testEvent> &event) override
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto casted = std::static_pointer_cast<testEvent>(event);
        this->x += casted->getX();
        processed = true;
        this->cv.notify_one();
    }

    int getX() const {return  this->x;}

protected:
    std::condition_variable &cv;
    std::mutex &mtx;
    bool &processed;
    int x;
};

class asyncHandler2 : public EventBus::EventHandler<testEvent>
{
public:
    asyncHandler2(std::condition_variable &cv, std::mutex &mtx, bool &processed) :cv(cv), mtx(mtx), processed(processed), x(0) {}
    void onEvent(std::shared_ptr<testEvent> &event) override
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto casted = std::static_pointer_cast<testEvent>(event);
        this->x += casted->getX();
        processed = true;
        this->cv.notify_one();
    }

    int getX() const {return  this->x;}

protected:
    std::condition_variable &cv;
    std::mutex &mtx;
    bool &processed;
    int x;
    int y;
};

void waitThread(std::condition_variable &cv, std::mutex &mtx, bool &processed)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] {return processed;});
}

TEST(EventBusTest, simpleTest)
{
    auto handler = std::make_shared<testHandler>();
    auto baseHandler = std::static_pointer_cast<EventBus::EventHandlerBase>(handler);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler);
    auto event = std::make_shared<testEvent>(5);
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);
    EventBus::EventBus::getInstance()->fire(baseEvent);

    EXPECT_EQ(handler->getX(), 5);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::cleanUp();
}

TEST(EventBusTest, testMultipleEventsOnSameHandler)
{
    auto handler = std::make_shared<testHandler>();
    auto baseHandler = std::static_pointer_cast<EventBus::EventHandlerBase>(handler);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler);
    auto event = std::make_shared<testEvent>(5);
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);
    EventBus::EventBus::getInstance()->fire(baseEvent);
    EventBus::EventBus::getInstance()->fire(baseEvent);
    EXPECT_EQ(handler->getX(), 10);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::getInstance()->fire(baseEvent);
    EXPECT_EQ(handler->getX(), 10);
    EventBus::EventBus::cleanUp();
}

TEST(EventBusTest, testMultipleHandler)
{
    auto handler1 = std::make_shared<testHandler>();
    auto handler2 = std::make_shared<testHandler>();
    auto baseHandler1 = std::static_pointer_cast<EventBus::EventHandlerBase>(handler1);
    auto baseHandler2 = std::static_pointer_cast<EventBus::EventHandlerBase>(handler2);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler1);
    auto handle2ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler2);
    auto event = std::make_shared<testEvent>(5);
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);
    EventBus::EventBus::getInstance()->fire(baseEvent);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 5);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::getInstance()->fire(baseEvent);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 10);
    EventBus::EventBus::cleanUp();
}

TEST(EventBusTest, testDifferentHandler)
{
    auto handler1 = std::make_shared<testHandler>();
    auto handler2 = std::make_shared<testHandler2>();
    auto baseHandler1 = std::static_pointer_cast<EventBus::EventHandlerBase>(handler1);
    auto baseHandler2 = std::static_pointer_cast<EventBus::EventHandlerBase>(handler2);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler1);
    auto handle2ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler2);
    auto event1 = std::make_shared<testEvent>(5);
    auto baseEvent1 = std::static_pointer_cast<EventBus::Event>(event1);
    auto event2 = std::make_shared<testEvent2>(3);
    auto baseEvent2 = std::static_pointer_cast<EventBus::Event>(event2);
    EventBus::EventBus::getInstance()->fire(baseEvent1);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 0);
    EventBus::EventBus::getInstance()->fire(baseEvent2);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 3);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::getInstance()->fire(baseEvent1);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 3);
    EventBus::EventBus::getInstance()->fire(baseEvent2);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 6);
    EventBus::EventBus::getInstance()->unregisterHandler(handle2ID);
    EventBus::EventBus::getInstance()->fire(baseEvent2);
    EXPECT_EQ(handler1->getX(), 5);
    EXPECT_EQ(handler2->getX(), 6);
    EventBus::EventBus::cleanUp();
}

TEST(EventBusTest, simpleFireAndForget)
{
    std::mutex mtx;
    std::condition_variable cv;
    bool processed = false;
    auto handler = std::make_shared<asyncHandler>(cv, mtx, processed);
    auto baseHandler = std::static_pointer_cast<EventBus::EventHandlerBase>(handler);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler);
    auto event = std::make_shared<testEvent>(5);
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);

    std::unique_lock<std::mutex> lock(mtx);
    EventBus::EventBus::getInstance()->fireAndForget(baseEvent);
    cv.wait(lock);
    EXPECT_EQ(handler->getX(), 5);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::cleanUp();
}

TEST(EventBusTest, differentAsyncHandlerForSameEventTest)
{
    std::mutex mtx1;
    std::mutex mtx2;
    std::condition_variable cv1;
    std::condition_variable cv2;
    bool processed1 = false;
    bool processed2 = false;
    auto handler = std::make_shared<asyncHandler>(cv1, mtx1, processed1);
    auto baseHandler = std::static_pointer_cast<EventBus::EventHandlerBase>(handler);
    auto handle1ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler);
    auto handler2 = std::make_shared<asyncHandler2>(cv2, mtx2, processed2);
    auto baseHandler2 = std::static_pointer_cast<EventBus::EventHandlerBase>(handler2);
    auto handle2ID = EventBus::EventBus::getInstance()->registerHandler(baseHandler2);
    auto event = std::make_shared<testEvent>(5);
    auto baseEvent = std::static_pointer_cast<EventBus::Event>(event);
    
    EventBus::EventBus::getInstance()->fireAndForget(baseEvent);
    std::thread t1(waitThread, std::ref(cv1), std::ref(mtx1), std::ref(processed1));
    std::thread t2(waitThread, std::ref(cv2), std::ref(mtx2), std::ref(processed2));
    t1.join();
    t2.join();
    processed1 = false;
    processed2 = false;
    EXPECT_EQ(handler->getX(), 5);
    EXPECT_EQ(handler2->getX(), 5);
    EventBus::EventBus::getInstance()->unregisterHandler(handle1ID);
    EventBus::EventBus::getInstance()->fireAndForget(baseEvent);
    std::thread t3(waitThread, std::ref(cv2), std::ref(mtx2), std::ref(processed2));
    t3.join();
    processed2 = false;
    EXPECT_EQ(handler->getX(), 5);
    EXPECT_EQ(handler2->getX(), 10);
    EventBus::EventBus::getInstance()->unregisterHandler(handle2ID);
    EventBus::EventBus::getInstance()->fireAndForget(baseEvent);
    EXPECT_EQ(handler->getX(), 5);
    EXPECT_EQ(handler2->getX(), 10);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EventBus::EventBus::cleanUp();
}
