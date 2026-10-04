#pragma once

#include <hwanhee1.hac/include/Types.hpp>
#include <cstdint>
#include <string>

namespace hac_test {

struct Color {
    std::uint8_t r, g, b;
};

struct StatusPresentation {
    std::string text = "HAC: API UNAVAILABLE";
    std::string history;
    Color color{255, 120, 120};
};

inline StatusPresentation present(hac::Snapshot const& snapshot) {
    StatusPresentation result;
    // Current state and attempt history must not be conflated: a disabled hack
    // can leave a prohibitedInAttempt history flag behind until the next attempt.
    if (snapshot.anyHackEnabled || snapshot.state == hac::State::Active) {
        result.text = "HAC: HACK ENABLED";
        result.color = {255, 95, 95};
    }
    else if (snapshot.state == hac::State::Unknown || snapshot.hasUnknownProviders) {
        result.text = "HAC: UNKNOWN";
        result.color = {255, 205, 100};
    }
    else if (snapshot.state == hac::State::NotPresent) {
        result.text = "HAC: NOT PRESENT";
        result.color = {180, 190, 205};
    }
    else if (snapshot.state == hac::State::Inactive) {
        // Inactive describes only HAC's monitored providers, not a clean client.
        result.text = "HAC: INACTIVE";
        result.color = {135, 225, 165};
    }
    else {
        result.text = "HAC: UNKNOWN";
        result.color = {255, 205, 100};
    }
    if (snapshot.prohibitedInAttempt) {
        result.history = "This attempt: hack observed";
    }
    return result;
}

// Keep error handling testable while the actual caller uses getSnapshot()
// from the unmodified HAC public header in another mod's binary.
template <class Query>
StatusPresentation queryStatus(Query&& query) {
    auto result = query();
    if (result.isErr()) return {};
    auto const& snapshot = result.unwrap();
    if (snapshot.apiVersion != hac::API_VERSION) return {};
    return present(snapshot);
}

} // namespace hac_test
