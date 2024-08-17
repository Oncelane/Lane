#include "base/log.h"
#include "http/httpserver.h"
#include "http/servlet.h"
static lane::Logger::ptr g_logger = LANE_LOG_ROOT();

#define XX(...) #__VA_ARGS__

lane::IOManager::ptr worker;
void                 run() {
    g_logger->setLevel(lane::LogLevel::INFO);
    // lane::http::HttpServer::ptr server(new lane::http::HttpServer(true,
    // worker.get(), lane::IOManager::GetThis()));
    lane::http::HttpServer::ptr server(new lane::http::HttpServer(true));
    lane::Address::ptr addr = lane::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) {
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx",
                   [](lane::http::HttpRequest::ptr  req,
                      lane::http::HttpResponse::ptr rsp,
                      lane::http::HttpSession::ptr  session) {
                       rsp->setBody(req->toString());
                       return 0;
                   });

    sd->addGlobServlet("/sylar/*",
                       [](lane::http::HttpRequest::ptr  req,
                          lane::http::HttpResponse::ptr rsp,
                          lane::http::HttpSession::ptr  session) {
                           rsp->setBody("Glob:\r\n" + req->toString());
                           return 0;
                       });

    sd->addGlobServlet(
        "/sylar/*",
        [](lane::http::HttpRequest::ptr  req,
           lane::http::HttpResponse::ptr rsp,
           lane::http::HttpSession::ptr  session) {
            rsp->setBody(XX(<html><head><title> 404 Not Found</ title></ head>
                                <body><center><h1> 404 Not Found</ h1></ center>
                                <hr><center>               nginx /
                                1.16.0 <
                            / center > </ body></ html>));
            return 0;
        });

    server->start();
}

int main(int argc, char** argv) {
    printf("test\n");
    lane::IOManager iom(4, "main", false);
    // worker.reset(new lane::IOManager(3,  "worker", false));
    iom.start();
    iom.addTask(run);
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::INFO);
    return 0;
}
