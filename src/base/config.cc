#include "base/config.h"

namespace lane {

    ConfigVarBase::ConfigVarBase(const std::string& name,
                                 const std::string& description)
        : m_name(name),
          m_description(description) {}

    static void __ListAllYamlNode(
        std::string prefix,
        const YAML::Node& node,
        std::vector<std::pair<std::string, YAML::Node>>& output) {
        output.push_back(std::make_pair(prefix, node));

        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end(); it++) {
                __ListAllYamlNode((prefix.empty() ? prefix : prefix + ".") +
                                      it->first.Scalar(),
                                  it->second,
                                  output);
            }
        }
    }

    void ConfigVarManager::loadFromFile(const std::string& val) {
        std::vector<std::pair<std::string, YAML::Node>> nodes;
        YAML::Node root = YAML::LoadFile(val);
        __ListAllYamlNode("", root, nodes);

        for (auto it = nodes.begin(); it != nodes.end(); it++) {
            ConfigVarBase::ptr var = lookUpConfigVarBaseByName(it->first);
            if (var != nullptr) {
                std::stringstream ss("");
                ss << it->second;
                var->fromString(ss.str());
                // LANE_LOG_DEBUG(LANE_LOG_NAME("system")) << var->toString();
            }
        }
    }
    void ConfigVarManager::loadFromConfDir(const std::string& path,
                                           bool force) {
        std::string absoulte_path =
            lane::EnvMgr::GetInstance()->getAbsolutePath(path);
        std::vector<std::string> files;
        FSUtil::ListAllFile(files, absoulte_path, ".yml");

        for (auto& i : files) {
            try {
                loadFromFile(i);
                LANE_LOG_INFO(LANE_LOG_NAME("system"))
                    << "LoadConfFile file=" << i << " ok";
            } catch (...) {
                LANE_LOG_ERROR(LANE_LOG_NAME("system"))
                    << "LoadConfFile file=" << i << " failed";
            }
        }
    }
    ConfigVarBase::ptr ConfigVarManager::lookUpConfigVarBaseByName(
        const std::string& val) {
        MutexType::ReadLock rlock(m_mutex);
        auto res = m_cvs.find(val);
        if (res == m_cvs.end()) {
            return nullptr;
        }

        return res->second;
    }
}  // namespace lane