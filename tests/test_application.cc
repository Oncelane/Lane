#include "init/application.h"
#include "init/module.h"

int main(int argc, char** argv) {
    lane::Application app;
    if (app.init(argc, argv)) {
        return app.run();
    }
    return 0;
}
