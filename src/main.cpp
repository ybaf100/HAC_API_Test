#include "StatusPresentation.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/LoadingLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <hwanhee1.hac/include/HAC.hpp>
#include <algorithm>
#include <chrono>
#include <string>

using namespace geode::prelude;

namespace {
bool ready = false;
constexpr int overlayZOrder = 1000000;
constexpr auto queryInterval = std::chrono::milliseconds(100);
}

$on_mod(Loaded) {
    ready = true;
    log::info("HAC API Test loaded: separate consumer of HAC API v{}", hac::API_VERSION);
}

namespace {
struct ConsumerState {
    // Retain only the overlay, not its scene. When the scene is destroyed,
    // getParent() becomes null; raw label pointers stay owned by this node.
    Ref<CCNode> overlay;
    CCLabelBMFont* status = nullptr;
    CCLabelBMFont* statusShadow = nullptr;
    CCLabelBMFont* history = nullptr;
    CCLabelBMFont* historyShadow = nullptr;
    hac_test::StatusPresentation presentation;
    std::chrono::steady_clock::time_point nextQuery{};
    std::string lastDiagnostic;

};

ConsumerState& consumerState() {
    // CCDirector is not a CCNode, so Geode m_fields cannot be used here.
    // Keep one overlay owner for the process lifetime; retain no old scenes.
    static auto* state = new ConsumerState;
    return *state;
}
}

class $modify(HACConsumerDirector, CCDirector) {
public:
    CCLabelBMFont* makeLabel(CCNode* parent, char const* id, bool shadow) {
        auto* label = CCLabelBMFont::create("HAC: UNKNOWN", "bigFont.fnt");
        if (!label) return nullptr;
        label->setID(id);
        label->setAnchorPoint({1.0f, 1.0f});
        if (shadow) {
            label->setColor({0, 0, 0});
            label->setOpacity(205);
        }
        parent->addChild(label, shadow ? 0 : 1);
        return label;
    }

    bool ensureOverlay(CCScene* scene) {
        if (consumerState().overlay && consumerState().overlay->getParent() == scene) return true;
        if (consumerState().overlay) consumerState().overlay->removeFromParent();
        consumerState().overlay = nullptr;
        auto* root = CCNode::create();
        root->setID("hac-api-status-overlay"_spr);
        root->setAnchorPoint({0.0f, 0.0f});
        root->setPosition({0.0f, 0.0f});
        consumerState().statusShadow = makeLabel(root, "hac-status-shadow", true);
        consumerState().status = makeLabel(root, "hac-current-status", false);
        consumerState().historyShadow = makeLabel(root, "hac-history-shadow", true);
        consumerState().history = makeLabel(root, "hac-attempt-history", false);
        if (!consumerState().status || !consumerState().statusShadow ||
            !consumerState().history || !consumerState().historyShadow) return false;
        consumerState().overlay = root;
        // A scene child is screen-space: not attached to the moving game camera
        // or vanilla UILayer, and it does not consume touch/key input.
        scene->addChild(root, overlayZOrder);
        return true;
    }

    void updateLabels() {
        auto const size = getWinSize();
        auto const& view = consumerState().presentation;
        auto const width = std::max(1.0f, std::min(220.0f, size.width - 20.0f));
        consumerState().overlay->setContentSize(size);
        consumerState().overlay->setZOrder(overlayZOrder);
        // Leave room for vanilla's top-right pause button. 0.35 bigFont scale
        // is legible without covering a large portion of a level.
        auto const position = CCPoint{size.width - 10.0f, size.height - 44.0f};
        auto const historyPosition = CCPoint{position.x, position.y - 14.0f};
        for (auto* label : {consumerState().status, consumerState().statusShadow}) {
            if (view.text != label->getString()) label->setString(view.text.c_str());
            label->limitLabelWidth(width, 0.35f, 0.20f);
        }
        for (auto* label : {consumerState().history, consumerState().historyShadow}) {
            if (view.history != label->getString()) label->setString(view.history.c_str());
            label->setVisible(!view.history.empty());
            label->limitLabelWidth(width, 0.25f, 0.18f);
        }
        consumerState().status->setColor({view.color.r, view.color.g, view.color.b});
        consumerState().history->setColor({255, 205, 100});
        consumerState().status->setPosition(position);
        consumerState().statusShadow->setPosition({position.x + 0.8f, position.y - 0.8f});
        consumerState().history->setPosition(historyPosition);
        consumerState().historyShadow->setPosition({historyPosition.x + 0.8f, historyPosition.y - 0.8f});
    }

    void drawScene() {
        auto* scene = getRunningScene();
        // Fonts are not guaranteed during initial loading. No HAC queries or
        // overlay construction until mod load and a non-loading scene.
        if (ready && scene && !scene->getChildByType<LoadingLayer>(0)) {
            auto const now = std::chrono::steady_clock::now();
            if (now >= consumerState().nextQuery) {
                // One public API query per sample, on the rendering/main thread.
                // No provider settings, private detector classes, or cheat hooks.
                consumerState().presentation = hac_test::queryStatus([] {
                    return hac::api::getSnapshot();
                });
                consumerState().nextQuery = now + queryInterval;
                auto const diagnostic = consumerState().presentation.text + " | " +
                    consumerState().presentation.history;
                if (diagnostic != consumerState().lastDiagnostic) {
                    log::info("HAC consumer status: {}", diagnostic);
                    consumerState().lastDiagnostic = diagnostic;
                }
            }
            if (ensureOverlay(scene)) updateLabels();
        }
        // Rendering keeps running when gameplay/scheduled timers are paused.
        // Always call vanilla exactly once and do not alter gameplay state.
        CCDirector::drawScene();
    }
};
