/* //////////////////////////////////////////////////////////////////////////////////////
 * imports
 */
#include <chrono>
#include <cstdio>

#include "base/fiber.h"
#include "base/iomanager.h"
#include "base/laneGo.h"
#include "base/waitGroup.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * globals
 */
const int COUNT = 10000000;

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementaiton
 */


void switchtask(int count) {

    int i = 0;
    while (true) {

        i++;
        if (i > count) {
            break;
        }
        lane::Fiber::YieldToReady();
    }
}

void TestMain() {
    // get coroutine count
    auto cocount = 1000;

    // init duration
    auto start = std::chrono::high_resolution_clock::now();
    auto wg = new lane::WaitGroup;
    // create coroutine task
    auto i = 0;
    while (true) {

        i++;
        if (i > cocount - 1) {
            break;
        }
        wg->add(1);
        co[=]() {
            switchtask(COUNT / cocount);
            wg->done();
        };
    }
    // computing time
    switchtask(COUNT / cocount);
    wg->wait();

    std::chrono::duration<double> elapsed =
        std::chrono::high_resolution_clock::now() - start;
    // LANE_LOG_INFO(LANE_LOG_ROOT()) << " Test spend " << elapsed.count() << "
    // s";

    // trace

    info() << "switch[" << cocount << "]: lane: " << COUNT << " switches in "
           << elapsed.count() * 1000 << " ms, "
           << int((COUNT / elapsed.count()))
           << " switches per "
              "second";
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * main
 */
int main() {

    // single cpu
    lane::IOManager iom(1, "main", false);
    iom.addTask(TestMain);
}
