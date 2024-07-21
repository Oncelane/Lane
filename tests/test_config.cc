#include <vector>

#include "base/config.h"
#include "base/log.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");

void test_config() {
    lane::ConfigVar<int>::ptr _int =
        lane::ConfigVarMgr::GetInstance()->lookUp("a", 17, "int test");
    lane::ConfigVar<std::vector<int>>::ptr _vec =
        lane::ConfigVarMgr::GetInstance()->lookUp(
            "test", std::vector<int>(), "int vec");
    static lane::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
        lane::ConfigVarMgr::GetInstance()->lookUp("tcp_server.read_timeout",
                                                  (uint64_t)(60 * 1000 * 2),
                                                  "tcp server read timeout");
    _vec->addListener([](const std::vector<int>& o, const std::vector<int>& n) {
        LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "hello";
    });
    LANE_LOG_DEBUG(LANE_LOG_ROOT())
        << _vec->getValue().size() << _vec->getDescription();
    lane::ConfigVarMgr::GetInstance()->loadFromFile("test.yml");
    LANE_LOG_DEBUG(LANE_LOG_ROOT())
        << _int->getValue() << _int->getDescription();

    LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "--------------";

    for (size_t i = 0; i < _vec->getValue().size(); i++) {
        LANE_LOG_DEBUG(LANE_LOG_ROOT()) << _vec->getValue()[i];
    }

    LANE_LOG_DEBUG(g_logger) << "-------------------------------------";

    LANE_LOG_DEBUG(g_logger) << g_tcp_server_read_timeout->getValue();
}

void test_yaml() {
    YAML::Node node(YAML::NodeType::Map);

    LANE_LOG_DEBUG(g_logger) << node["test"].IsDefined();

    node["name"] = YAML::Load("123");
    LANE_LOG_DEBUG(g_logger) << node["name"].IsScalar();
}

// LANE_LOG_DEBUG(g_logger) << "hello1";
// lane::ConfigVarMgr::GetInstance()->loadFromFile("log.yml");

// LANE_LOG_INFO(g_logger) << "hello2";

void test_logConfig() {
    lane::ConfigVar<std::vector<int>>::ptr vec =
        lane::ConfigVarMgr::GetInstance()->lookUp(
            "space.vec", std::vector<int>(), "vec test");
    lane::ConfigVar<int>::ptr num = lane::ConfigVarMgr::GetInstance()->lookUp(
        "space.num", int(), "num test");
    lane::ConfigVarMgr::GetInstance()->loadFromFile("test_config.yml");

    LANE_LOG_INFO(g_logger) << vec->toString();
    LANE_LOG_INFO(g_logger) << num->toString();
}
int main() {
    test_logConfig();
    return 0;
}