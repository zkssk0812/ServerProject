#pragma once
#ifndef _ThreadSafeQueue

#include <windows.h>
#include <queue>

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {
        InitializeCriticalSection(&m_cs);
        InitializeConditionVariable(&m_cv);
    }
    ~ThreadSafeQueue() {
        DeleteCriticalSection(&m_cs);
    }

    void Push(T value) {
        EnterCriticalSection(&m_cs);

        m_queue.push(std::move(value));

        LeaveCriticalSection(&m_cs);


        WakeConditionVariable(&m_cv);
    }

    T Pop() {

        EnterCriticalSection(&m_cs);


        while (m_queue.empty()) {

            SleepConditionVariableCS(&m_cv, &m_cs, INFINITE);
        }


        T value = std::move(m_queue.front());
        m_queue.pop();


        LeaveCriticalSection(&m_cs);

        return value;
    }

private:
    std::queue<T> m_queue;
    CRITICAL_SECTION m_cs;
    CONDITION_VARIABLE m_cv;
};

#endif 
