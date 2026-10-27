#pragma once

#include <string>
#include <vector>
#include <optional>

namespace ionexa::project {

struct ProjectDetails {
    std::string canonicalPath;
    std::string projectName;
    std::string projectVersion;
    bool isExpoProject{false};
    std::string expoSdkVersion;
    std::string reactNativeVersion;
    std::string packageManager{"npm"};
    bool isExpoRouter{false};
    bool hasWebSupport{false};
    std::vector<std::string> missingWebDependencies;
    std::string remediationCommand;
    bool nodeInstalled{false};
    std::string nodeVersion;
    std::string entryPoint;
    bool isValid{false};
    std::string errorMessage;
};

class ProjectScanner {
public:
    static ProjectDetails scan(const std::string& directoryPath);
    static bool checkNodePrerequisites(std::string& detectedVersion);
};

} // namespace ionexa::project
