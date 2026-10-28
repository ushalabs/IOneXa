#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <memory>
#include <functional>

namespace ionexa::engines {

enum class ProviderAvailabilityStatus {
    Available,
    Waitlisted,
    Unauthenticated,
    PaymentRequired,
    UnsupportedOnHost,
    NetworkError,
    Unknown
};

struct ProviderAvailability {
    ProviderAvailabilityStatus status{ProviderAvailabilityStatus::Unknown};
    std::string providerName;
    std::string message;
    bool requiresInteractiveLogin{false};
    bool isEntitled{false};
};

struct SimulatorDeviceInfo {
    std::string id;
    std::string name;
    std::string runtimeVersion; // e.g. "iOS 18.0"
    int pixelWidth{0};
    int pixelHeight{0};
    double screenScale{3.0};
    bool supportsTouch{true};
    bool supportsRotation{true};
};

enum class NativeSimulatorState {
    NoSession,
    Validating,
    NeedSignIn,
    BlockedByEntitlement,
    PaymentApprovalRequired,
    BuildRequired,
    Building,
    BuildFailed,
    BuildReady,
    StartingSimulator,
    SimulatorReady,
    Installing,
    Launching,
    NativeAppRunning,
    DevServerUnavailable,
    RemoteDisconnected,
    AppCrashed,
    ProviderUnavailable,
    UserCanceled,
    Stopping,
    Stopped,
    CleanupFailed
};

inline std::string nativeSimulatorStateToString(NativeSimulatorState state) {
    switch (state) {
    case NativeSimulatorState::NoSession: return "NoSession";
    case NativeSimulatorState::Validating: return "Validating";
    case NativeSimulatorState::NeedSignIn: return "NeedSignIn";
    case NativeSimulatorState::BlockedByEntitlement: return "BlockedByEntitlement";
    case NativeSimulatorState::PaymentApprovalRequired: return "PaymentApprovalRequired";
    case NativeSimulatorState::BuildRequired: return "BuildRequired";
    case NativeSimulatorState::Building: return "Building";
    case NativeSimulatorState::BuildFailed: return "BuildFailed";
    case NativeSimulatorState::BuildReady: return "BuildReady";
    case NativeSimulatorState::StartingSimulator: return "StartingSimulator";
    case NativeSimulatorState::SimulatorReady: return "SimulatorReady";
    case NativeSimulatorState::Installing: return "Installing";
    case NativeSimulatorState::Launching: return "Launching";
    case NativeSimulatorState::NativeAppRunning: return "NativeAppRunning";
    case NativeSimulatorState::DevServerUnavailable: return "DevServerUnavailable";
    case NativeSimulatorState::RemoteDisconnected: return "RemoteDisconnected";
    case NativeSimulatorState::AppCrashed: return "AppCrashed";
    case NativeSimulatorState::ProviderUnavailable: return "ProviderUnavailable";
    case NativeSimulatorState::UserCanceled: return "UserCanceled";
    case NativeSimulatorState::Stopping: return "Stopping";
    case NativeSimulatorState::Stopped: return "Stopped";
    case NativeSimulatorState::CleanupFailed: return "CleanupFailed";
    default: return "Unknown";
    }
}

struct SimulatorSessionInfo {
    std::string sessionId;
    std::string providerName;
    NativeSimulatorState state{NativeSimulatorState::NoSession};
    std::string deviceId;
    std::string deviceName;
    std::string osVersion;
    std::string installedBundleId;
    std::string buildId;
    std::string streamViewerUrl;
    bool isEmbeddedViewerSupported{false};
    std::chrono::system_clock::time_point startedAt;
};

struct ProviderLogEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string level; // "info", "warn", "error"
    std::string source; // "provider", "os", "app"
    std::string message;
};

struct ScreenshotResult {
    bool success{false};
    std::vector<uint8_t> imageData; // PNG or JPEG bytes
    std::string mimeType{"image/png"};
    std::string provenance;
    std::string errorMessage;
};

struct ApprovedViewerInfo {
    bool canEmbedInWebView2{false};
    std::string viewerUrl;
    std::string externalBrowserUrl;
    std::string authNotice;
    std::string termsNotice;
};

class ISimulatorProvider {
public:
    virtual ~ISimulatorProvider() = default;

    [[nodiscard]] virtual std::string providerId() const = 0;
    [[nodiscard]] virtual std::string displayName() const = 0;

    virtual ProviderAvailability probeAvailability() = 0;
    virtual std::vector<SimulatorDeviceInfo> getSupportedDevices() = 0;

    virtual bool startSession(const std::string& buildArtifactPathOrId,
                              const std::string& targetDeviceId,
                              SimulatorSessionInfo& outSession) = 0;

    virtual NativeSimulatorState pollSessionState(const std::string& sessionId) = 0;

    virtual bool launchApplication(const std::string& sessionId,
                                   const std::string& bundleId,
                                   const std::optional<std::string>& nativeDevServerUrl) = 0;

    virtual SimulatorSessionInfo getSessionInfo(const std::string& sessionId) = 0;
    virtual ApprovedViewerInfo getApprovedViewer(const std::string& sessionId) = 0;

    virtual ScreenshotResult captureScreenshot(const std::string& sessionId) = 0;
    virtual std::vector<ProviderLogEntry> retrieveLogs(const std::string& sessionId) = 0;

    virtual bool stopSession(const std::string& sessionId) = 0;
};

struct BuildPrerequisites {
    bool hasAppJson{false};
    bool hasBundleIdentifier{false};
    std::string bundleIdentifier;
    bool hasEasJson{false};
    bool hasSimulatorProfile{false};
    std::string simulatorProfileName;
    bool isExpoDevClientInstalled{false};
    std::string expoSdkVersion;
};

struct BuildSubmissionResult {
    bool success{false};
    std::string buildId;
    std::string platform{"ios"};
    std::string profile;
    std::string logsUrl;
    std::string errorMessage;
};

enum class BuildStatus {
    Queued,
    Building,
    Succeeded,
    Failed,
    Canceled,
    Unknown
};

struct BuildPollResult {
    std::string buildId;
    BuildStatus status{BuildStatus::Unknown};
    std::string artifactUrl;
    std::string errorMessage;
};

class IBuildProvider {
public:
    virtual ~IBuildProvider() = default;

    [[nodiscard]] virtual std::string providerId() const = 0;
    virtual BuildPrerequisites checkNativeBuildPrerequisites(const std::string& projectDir) = 0;
    virtual BuildSubmissionResult submitSimulatorBuild(const std::string& projectDir, const std::string& profile) = 0;
    virtual BuildPollResult pollBuild(const std::string& buildId) = 0;
    virtual std::string downloadSimulatorArtifact(const std::string& buildId, const std::string& destDirectory) = 0;
};

struct DevServerStartResult {
    bool success{false};
    int localPort{0};
    int processId{0};
    std::string localUrl;
    std::string errorMessage;
};

struct TunnelEgressResult {
    bool success{false};
    std::string publicUrl; // ngrok / expo tunnel endpoint reachable from cloud simulator
    std::string errorMessage;
};

class INativeDevServer {
public:
    virtual ~INativeDevServer() = default;

    virtual DevServerStartResult startNativeMetro(const std::string& projectDir, int preferredPort) = 0;
    virtual TunnelEgressResult makeAccessibleToRemoteSimulator(int localPort) = 0;
    virtual bool awaitClientConnection(int timeoutMs) = 0;
    virtual bool stopOwnedProcessOnly() = 0;
};

} // namespace ionexa::engines
