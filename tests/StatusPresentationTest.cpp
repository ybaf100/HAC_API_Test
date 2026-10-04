#include "StatusPresentation.hpp"
#include <cstdlib>
#include <iostream>

namespace {
int checks = 0;
void check(bool condition, char const* expression, int line) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL at line " << line << ": " << expression << '\n';
        std::exit(1);
    }
}
#define CHECK(expr) check((expr), #expr, __LINE__)

hac::Snapshot snapshot(hac::State state) {
    hac::Snapshot result;
    result.state = state;
    result.hasUnknownProviders = state == hac::State::Unknown;
    return result;
}

// Only simulates the consumer-facing Result interface for presentation tests.
// Real inter-mod Geode event dispatch must be verified in Geometry Dash.
struct QueryResult {
    bool error = false;
    hac::Snapshot value;
    bool isErr() const { return error; }
    hac::Snapshot const& unwrap() const { return value; }
};
}

int main() {
    using hac::State;
    using hac_test::present;
    auto active = snapshot(State::Active);
    active.anyHackEnabled = true;
    CHECK(present(active).text == "HAC: HACK ENABLED");
    CHECK(present(active).color.r == 255);
    active.hasUnknownProviders = true;
    CHECK(present(active).text == "HAC: HACK ENABLED");
    auto partial = snapshot(State::Unknown);
    partial.anyHackEnabled = true;
    CHECK(present(partial).text == "HAC: HACK ENABLED");
    partial.anyHackEnabled = false;
    CHECK(present(partial).text == "HAC: UNKNOWN");
    CHECK(present(snapshot(State::Inactive)).text == "HAC: INACTIVE");
    CHECK(present(snapshot(State::NotPresent)).text == "HAC: NOT PRESENT");
    CHECK(present(snapshot(State::Unknown)).text == "HAC: UNKNOWN");
    auto inactive = snapshot(State::Inactive);
    inactive.hasUnknownProviders = true;
    CHECK(present(inactive).text == "HAC: UNKNOWN");
    inactive.hasUnknownProviders = false;
    inactive.prohibitedInAttempt = true;
    CHECK(present(inactive).text == "HAC: INACTIVE");
    CHECK(present(inactive).history == "This attempt: hack observed");
    inactive.prohibitedInAttempt = false;
    CHECK(present(inactive).history.empty());
    active.prohibitedInAttempt = true;
    CHECK(present(active).history == "This attempt: hack observed");
    // Toggle off: current status stops claiming active; history remains.
    active.anyHackEnabled = false;
    active.state = State::Unknown;
    CHECK(present(active).text == "HAC: UNKNOWN");
    CHECK(!present(active).history.empty());
    // A new attempt clears the historical warning supplied by HAC.
    active.prohibitedInAttempt = false;
    CHECK(present(active).history.empty());
    auto query = hac_test::queryStatus([&] { return QueryResult{false, inactive}; });
    CHECK(query.text == "HAC: INACTIVE");
    query = hac_test::queryStatus([] { return QueryResult{true, {}}; });
    CHECK(query.text == "HAC: API UNAVAILABLE");
    CHECK(query.history.empty());
    auto incompatible = inactive;
    incompatible.apiVersion = hac::API_VERSION + 1;
    query = hac_test::queryStatus([&] { return QueryResult{false, incompatible}; });
    CHECK(query.text == "HAC: API UNAVAILABLE");
    auto badState = snapshot(static_cast<State>(255));
    CHECK(present(badState).text == "HAC: UNKNOWN");
    CHECK(hac::API_VERSION == 2);
    std::cout << "Passed " << checks << " consumer presentation checks\n";
}
