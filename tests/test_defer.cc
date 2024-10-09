#include <exception>
#include <memory>
#include <stdexcept>

#include "base/iomanager.h"
#include "base/laneGo.h"
#include "base/log.h"
#include "base/mutex.h"

using namespace lane;

void TestMain() {
    co[]() {

        defer[]() {
            info() << "defer1";
        };
        defer[]() {
            info() << "defer2";
        };
        defer[]() {
            info() << "defer3";
            auto e = recovery();
            if (e.has_value()) {
                info() << "recovery this error " << e.value().what();
            }
        };

        info() << "throw panic";
        panic(std::runtime_error("panic for test"));

        info() << "safe return";
    };
}


int main() {
    lane::IOManager iom(2, "hello", false);
    iom.addTask(TestMain);
}