#pragma once

#include "Types.hpp"
#include <Geode/loader/Event.hpp>
#include <Geode/Result.hpp>

namespace hac::api {

// Query from the game's main thread. The bool is a handshake: no listener or
// an incompatible API version is a failure, rather than a clean snapshot.
struct QuerySnapshotEvent final : geode::Event<QuerySnapshotEvent, bool(Snapshot&, std::uint32_t)> {
    using Event::Event;
};

inline geode::Result<Snapshot> getSnapshot() {
    Snapshot snapshot;
    if (!QuerySnapshotEvent().send(snapshot, API_VERSION)) {
        return geode::Err("H-Anticheat API is unavailable on this thread or has an incompatible version");
    }
    if (snapshot.apiVersion != API_VERSION) {
        return geode::Err("H-Anticheat API returned an incompatible snapshot");
    }
    return geode::Ok(std::move(snapshot));
}

// Query and return the current aggregate state without requiring consumers to
// unpack the full snapshot. Unknown is a valid state and is not an error.
inline geode::Result<State> getCurrentState() {
    auto result = getSnapshot();
    if (result.isErr()) return geode::Err(result.unwrapErr());
    return geode::Ok(result.unwrap().state);
}

// A known enabled feature can be returned even when another provider is
// unknown. A negative answer requires a successfully queried inactive snapshot.
inline geode::Result<bool> hasProhibitedFeaturesEnabled() {
    auto result = getSnapshot();
    if (result.isErr()) return geode::Err(result.unwrapErr());
    auto const& snapshot = result.unwrap();
    if (snapshot.state == State::Unknown) {
        return geode::Err("H-Anticheat policy status is unknown; inspect getSnapshot() for coverage and policy details");
    }
    return geode::Ok(snapshot.anyHackEnabled);
}

// API v2 alias: prohibited under snapshot.policyId, including CR's QOL red rule.
inline geode::Result<bool> hasCheatsEnabled() {
    return hasProhibitedFeaturesEnabled();
}

} // namespace hac::api
