/*
 * MIT Licence
 * Copyright 2019 by the creator @CodesBay 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

// This enumerator returns with the callback with the reason
// of the timer expiry i.e Timeout / Timer Stopped
enum class TimeOutReason {
    TIMER_EXPIRED,
    SIGNAL_RECEIVED
};

// The Main class which contains the SetTimeout function as well as StopTimer Function
class CppTimer {
    std::mutex timerMutex;
    std::condition_variable condTimer;
    std::cv_status retStatus;
    std::thread timeoutThread;
public:
    // The function to set the timeout in milliseconds. It runs the timer on a saperate
    // thread of execution
    void SetTimeout(const std::chrono::milliseconds & ms, std::function<void(TimeOutReason)> callBack) {
        timeoutThread = std::thread{[=]()
            {
                std::unique_lock<std::mutex> lock(timerMutex);
                retStatus = condTimer.wait_for(lock, ms);
                TimeOutReason tReason;
                if(std::cv_status::timeout == retStatus) {
                    tReason = TimeOutReason::TIMER_EXPIRED;
                }else {
                    tReason = TimeOutReason::SIGNAL_RECEIVED;
                }
                callBack(tReason);
           }
        };
    }
    // This will stop the timer if running. This function needs to be called everytime someone
    // calls the SetTimeout(...). it makes sure that timer thread is clearned properly
    void StopTimer() {
        condTimer.notify_one();
        timeoutThread.join();
    }
};

// The callback function
class CallBackFn {
public:
    static void callback(TimeOutReason tReason) {
        std::cout<<"Callback called.."<<std::endl;
    }
};

// The main test only function. Must be removed before using
int main() {
    // Instantiate CppTimer
    CppTimer ctimer;
    // Set the timerout
    ctimer.SetTimeout(std::chrono::milliseconds(10000), CallBackFn::callback);
    // Wait for sometime
    std::this_thread::sleep_for(std::chrono::seconds(4));
    // Stop the timer
    ctimer.StopTimer();
    return 0;
}
