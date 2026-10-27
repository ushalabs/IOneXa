#pragma once

#include <string>
#include <memory>
#include "core/session_state.hpp"
#include "core/capability_manifest.hpp"

namespace ionexa::engines {

struct ProjectInspection {
    bool isExpoProject{false};
    std::string expoSdkVersion;
    std::string packageManager;
    bool hasWebDependencies{false};
    std::vector<std::string> missingDependencies;
    std::string entryPoint;
};

struct PrepareResult {
    bool success{false};
    std::string message;
};

struct RunningSession {
    int processId{0};
    int listeningPort{0};
    std::string loopbackUrl;
    bool isReady{false};
};

struct StopResult {
    bool success{false};
    int exitCode{0};
    std::string message;
};

struct ReloadResult {
    bool success{false};
    std::string message;
};

class IRuntimeEngine {
public:
    virtual ~IRuntimeEngine() = default;

    [[nodiscard]] virtual std::string engineName() const = 0;
    [[nodiscard]] virtual ProjectInspection inspect(const std::string& projectPath) = 0;
    virtual PrepareResult prepare(const std::string& projectPath) = 0;
    virtual RunningSession start(const std::string& projectPath, int targetPort) = 0;
    virtual StopResult stop() = 0;
    virtual ReloadResult reload() = 0;
    [[nodiscard]] virtual core::CapabilityManifest capabilities(const std::string& projectPath) = 0;
};

} // namespace ionexa::engines
