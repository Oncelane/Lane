#include "init/application.h"

#include <signal.h>
#include <unistd.h>

#include "base/config.h"
#include "base/log.h"
#include "base/worker.h"
#include "http/httpserver.h"
#include "http/servlet.h"
#include "init/daemon.h"
#include "init/env.h"
#include "init/module.h"
#include "net/tcpserver.h"
namespace lane {

    static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");

    static lane::ConfigVar<std::string>::ptr g_server_work_path =
        lane::ConfigVarMgr::GetInstance()->lookUp(
            "server.work_path",
            std::string("/apps/work/lane"),
            "server work path");

    static lane::ConfigVar<std::string>::ptr g_server_pid_file =
        lane::ConfigVarMgr::GetInstance()->lookUp(
            "server.pid_file", std::string("lane.pid"), "server pid file");

    static lane::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf =
        lane::ConfigVarMgr::GetInstance()->lookUp(
            "servers", std::vector<TcpServerConf>(), "http server config");

    Application* Application::s_instance = nullptr;

    Application::Application() {
        s_instance = this;
    }

    bool Application::init(int argc, char** argv) {
        m_argc = argc;
        m_argv = argv;

        lane::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
        lane::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
        lane::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
        lane::EnvMgr::GetInstance()->addHelp("p", "print help");

        bool is_print_help = false;
        if (!lane::EnvMgr::GetInstance()->init(argc, argv)) {
            is_print_help = true;
        }

        if (lane::EnvMgr::GetInstance()->has("p")) {
            is_print_help = true;
        }
        // m_cwd + conf;
        std::string conf_path = lane::EnvMgr::GetInstance()->getConfigPath();
        LANE_LOG_INFO(g_logger) << "load conf path:" << conf_path;
        lane::ConfigVarMgr::GetInstance()->loadFromConfDir(conf_path);

        ModuleMgr::GetInstance()->init();
        std::vector<Module::ptr> modules;
        ModuleMgr::GetInstance()->listAll(modules);

        for (auto i : modules) {
            i->onBeforeArgsParse(argc, argv);
        }

        if (is_print_help) {
            lane::EnvMgr::GetInstance()->printHelp();
            return false;
        }

        for (auto i : modules) {
            i->onAfterArgsParse(argc, argv);
        }
        modules.clear();

        int run_type = 0;
        if (lane::EnvMgr::GetInstance()->has("s")) {
            run_type = 1;
        }
        if (lane::EnvMgr::GetInstance()->has("d")) {
            run_type = 2;
        }

        if (run_type == 0) {
            lane::EnvMgr::GetInstance()->printHelp();
            return false;
        }

        g_logger->setLevel(LogLevel::INFO);
        return true;
    }

    bool Application::run() {
        bool is_daemon = lane::EnvMgr::GetInstance()->has("d");
        return start_daemon(m_argc,
                            m_argv,
                            std::bind(&Application::main,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2),
                            is_daemon);
    }

    int Application::main(int argc, char** argv) {
        signal(SIGPIPE, SIG_IGN);
        LANE_LOG_INFO(g_logger) << "main";
        std::string conf_path = lane::EnvMgr::GetInstance()->getConfigPath();
        // 是否冗余？
        lane::ConfigVarMgr::GetInstance()->loadFromConfDir(conf_path, true);
        {
            std::string pidfile = g_server_work_path->getValue() + "/" +
                                  g_server_pid_file->getValue();
            std::ofstream ofs(pidfile);
            if (!ofs) {
                LANE_LOG_ERROR(g_logger)
                    << "open pidfile " << pidfile << " failed";
                return false;
            }
            ofs << getpid();
        }

        m_mainIOManager.reset(new lane::IOManager(1, "main", true));
        m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
        // block
        m_mainIOManager->addTimer(
            2000,
            []() {
                // LANE_LOG_INFO(g_logger) << "hello";
            },
            true);
        m_mainIOManager->stop();
        return 0;
    }

    int Application::run_fiber() {
        std::vector<Module::ptr> modules;
        ModuleMgr::GetInstance()->listAll(modules);
        bool has_error = false;
        for (auto& i : modules) {
            if (!i->onLoad()) {
                LANE_LOG_ERROR(g_logger) << "module name=" << i->getName()
                                         << " version=" << i->getVersion()
                                         << " filename=" << i->getFilename();
                has_error = true;
            }
        }
        if (has_error) {
            _exit(0);
        }

        lane::WorkerMgr::GetInstance()->init();

        auto http_confs = g_servers_conf->getValue();
        std::vector<TcpServer::ptr> svrs;
        for (auto& i : http_confs) {
            LANE_LOG_DEBUG(g_logger)
                << std::endl
                << LexicalCast<TcpServerConf, std::string>()(i);

            std::vector<Address::ptr> address;
            for (auto& a : i.address) {
                size_t pos = a.find(":");
                if (pos == std::string::npos) {
                    LANE_LOG_ERROR(g_logger) << "invalid address: " << a;
                    continue;
                }
                int32_t port = atoi(a.substr(pos + 1).c_str());
                // 127.0.0.1
                auto addr = lane::Address::LookupAny(a);
                if (addr) {
                    address.push_back(addr);
                    continue;
                }
                std::vector<std::pair<Address::ptr, uint32_t> > result;
                if (lane::Address::GetInterfaceAddresses(result,
                                                         a.substr(0, pos))) {
                    for (auto& x : result) {
                        auto ipaddr =
                            std::dynamic_pointer_cast<IPAddress>(x.first);
                        if (ipaddr) {
                            ipaddr->setPort(port);
                        }
                        address.push_back(ipaddr);
                    }
                    continue;
                }
            }
            IOManager* accept_worker = lane::IOManager::GetThis();
            IOManager* io_worker = lane::IOManager::GetThis();
            IOManager* process_worker = lane::IOManager::GetThis();
            if (!i.accept_worker.empty()) {
                accept_worker = lane::WorkerMgr::GetInstance()
                                    ->getAsIOManager(i.accept_worker)
                                    .get();
                if (!accept_worker) {
                    LANE_LOG_ERROR(g_logger)
                        << "accept_worker: " << i.accept_worker
                        << " not exists";
                    _exit(0);
                }
            }
            if (!i.io_worker.empty()) {
                io_worker = lane::WorkerMgr::GetInstance()
                                ->getAsIOManager(i.io_worker)
                                .get();
                if (!io_worker) {
                    LANE_LOG_ERROR(g_logger)
                        << "io_worker: " << i.io_worker << " not exists";
                    _exit(0);
                }
            }
            if (!i.process_worker.empty()) {
                process_worker = lane::WorkerMgr::GetInstance()
                                     ->getAsIOManager(i.process_worker)
                                     .get();
                if (!process_worker) {
                    LANE_LOG_ERROR(g_logger)
                        << "process_worker: " << i.process_worker
                        << " not exists";
                    _exit(0);
                }
            }

            TcpServer::ptr server;
            if (i.type == "http") {
                server.reset(new lane::http::HttpServer(
                    i.keepalive, process_worker, io_worker, accept_worker));
                server->setType(i.type);
            } else {
                LANE_LOG_ERROR(g_logger)
                    << "invalid server type=" << i.type
                    << LexicalCast<TcpServerConf, std::string>()(i);
                _exit(0);
            }
            if (!i.name.empty()) {
                server->setName(i.name);
            }
            std::vector<Address::ptr> fails;
            if (!server->bind(address, fails, i.ssl)) {
                for (auto& x : fails) {
                    LANE_LOG_ERROR(g_logger) << "bind address fail:" << *x;
                }
                _exit(0);
            }

            server->setConf(i);
            // server->start();
            m_servers[i.type].push_back(server);
            svrs.push_back(server);
        }

        for (auto& i : modules) {
            i->onServerReady();
        }

        for (auto& i : svrs) {
            i->start();
        }

        for (auto& i : modules) {
            i->onServerUp();
        }
        return 0;
    }

    bool Application::getServer(const std::string& type,
                                std::vector<TcpServer::ptr>& svrs) {
        auto it = m_servers.find(type);
        if (it == m_servers.end()) {
            return false;
        }
        svrs = it->second;
        return true;
    }

    void Application::listAllServer(
        std::map<std::string, std::vector<TcpServer::ptr> >& servers) {
        servers = m_servers;
    }

}  // namespace lane
