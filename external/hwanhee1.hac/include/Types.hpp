#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hac {

inline constexpr std::uint32_t API_VERSION = 2;
inline constexpr char POLICY_ID[] = "cr-aredl-2026-10-04-mh-qol-red";
inline constexpr char POLICY_SOURCE[] = "https://aredl.net/guidelines";

// Inactive only describes the providers represented by this snapshot. It is
// never proof that a client, an external injector, or an unmonitored mod is safe.
enum class State : std::uint8_t { NotPresent, Inactive, Active, Unknown };
enum class Coverage : std::uint8_t { ProviderAPI, KnownFeatures, Unsupported, FullBuiltinConfiguration };
enum class EvidenceKind : std::uint8_t { RuntimeAPI, Configuration };
enum class Policy : std::uint8_t { Allowed, Prohibited, Conditional, Unmapped, NotApplicable };
enum class Mode : std::uint8_t { Classic, Platformer };

struct Feature {
    std::string id;
    std::string name;
    EvidenceKind evidence = EvidenceKind::RuntimeAPI;
    State state = State::Unknown;
    Policy policy = Policy::Unmapped;
    std::string value;
    std::string source;
    std::string detail;
};

struct ProviderSnapshot {
    std::string id;
    std::string name;
    std::string version;
    State state = State::Unknown;
    Coverage coverage = Coverage::Unsupported;
    std::vector<Feature> enabledFeatures;
    std::vector<Feature> attemptFeatures;
    bool attemptHistoryAvailable = false;
    bool cheatedInAttempt = false;
    std::string detail;
    // All observations, including allowed features and unresolved conditions.
    // enabledFeatures contains known prohibited enabled features under policyId
    // (AREDL plus CR's explicit all-QOL-red-feature rule).
    std::vector<Feature> features;
    bool hasUnknownFeatures = false;
    bool providerReportedEnabled = false;
    std::uint32_t catalogSize = 0;
};

struct Snapshot {
    std::uint32_t apiVersion = API_VERSION;
    std::uint64_t revision = 0;
    std::uint64_t sampledAtMonotonicMs = 0;
    State state = State::Unknown;
    bool anyHackEnabled = false;
    bool hasUnknownProviders = true;
    bool providerReportedAttemptCheat = false;
    std::vector<ProviderSnapshot> providers;
    std::string policyId = POLICY_ID;
    std::string policySource = POLICY_SOURCE;
    Mode mode = Mode::Classic;
    bool prohibitedInAttempt = false;
    std::vector<Feature> attemptFeatures;
};

} // namespace hac
