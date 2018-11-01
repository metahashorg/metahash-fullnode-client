#include "stopProgram.h"

#include <signal.h>
#include <iostream>

std::atomic<bool> isStopSignalCalled(false);

void initializeStopProgram() {
    signal(SIGTERM, stopHandler);
    signal(SIGABRT, stopHandler);
    signal(SIGINT, stopHandler);
}

void stopHandler(int sig) {
    signal(SIGTERM, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    
    std::cout << "Stop signal received " + std::to_string(sig) << std::endl;
    isStopSignalCalled = true;
}

void checkStopSignal(const std::atomic<bool> &checkedVariable) {
    if (checkedVariable.load()) {
        throw StopException();
    }
}

void checkStopSignal() {
    checkStopSignal(isStopSignalCalled);
}

void sleep(const seconds &dur) {
    const time_point beginTime = ::now();
    while (true) {
        checkStopSignal();
        const time_point now = ::now();
        if (now - beginTime >= dur) {
            break;
        }
        std::this_thread::sleep_for(1s);
    }
}


void sleepMs(const milliseconds &dur) {
    std::this_thread::sleep_for(dur);
    checkStopSignal();
}

void whileTrue() {
    try {
        while (true) {
            sleep(1s);
            checkStopSignal();
        }
    } catch (const StopException &e) {
        return;
    }
}
