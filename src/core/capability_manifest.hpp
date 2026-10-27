#pragma once

#include <string>
#include <vector>

namespace ionexa::core {

enum class CapabilityStatus {
    Supported,
    Partial,
    Unsupported,
    Unverified
};

inline std::string capabilityStatusToString(CapabilityStatus status) {
    switch (status) {
    case CapabilityStatus::Supported: return "supported";
    case CapabilityStatus::Partial: return "partial";
    case CapabilityStatus::Unsupported: return "unsupported";
    case CapabilityStatus::Unverified: return "unverified";
    default: return "unknown";
    }
}

struct CapabilityItem {
    std::string feature;
    CapabilityStatus status;
    std::string evidence;
    std::string notes;
};

class CapabilityManifest {
public:
    void addItem(CapabilityItem item) {
        m_items.push_back(std::move(item));
    }

    [[nodiscard]] const std::vector<CapabilityItem>& items() const {
        return m_items;
    }

    [[nodiscard]] size_t countWithStatus(CapabilityStatus status) const {
        size_t count = 0;
        for (const auto& item : m_items) {
            if (item.status == status) {
                ++count;
            }
        }
        return count;
    }

    void clear() {
        m_items.clear();
    }

private:
    std::vector<CapabilityItem> m_items;
};

} // namespace ionexa::core
