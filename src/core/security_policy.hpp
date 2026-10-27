#pragma once

#include <QString>
#include <QUrl>
#include <QDir>
#include <QFileInfo>

namespace ionexa::core {

class SecurityPolicy {
public:
    // Navigation boundary verification
    [[nodiscard]] static bool isSafePreviewUrl(const QString& urlString);
    [[nodiscard]] static bool isLoopbackHost(const QString& host);
    [[nodiscard]] static bool isDisallowedScheme(const QString& scheme);

    // Path traversal and directory boundary enforcement
    [[nodiscard]] static bool isPathWithinDirectory(const QString& basePath, const QString& candidatePath);
    [[nodiscard]] static bool containsTraversalSequences(const QString& path);

    // Port range validation
    [[nodiscard]] static bool isSafeDevPort(int port);

    // Sensitive data masking
    [[nodiscard]] static QString maskToken(const QString& token);
};

} // namespace ionexa::core
