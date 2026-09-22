#pragma once

// Projucer remains the unlicensed Beta 1 mirror. CMake selects the DRM build.
#ifndef RCL_ENABLE_MOONBASE
#define RCL_ENABLE_MOONBASE 0
#endif

#if RCL_ENABLE_MOONBASE
#include "MoonbaseConfig.h"

namespace rcl
{
// Shared by processors in this plugin binary, never owned by an editor. Each
// process/format uses the same persisted license; the SDK locks file updates.
class MoonbaseLicense final : private juce::AsyncUpdater,
                              private juce::Timer,
                              private juce::ChangeListener
{
public:
    MoonbaseLicense();
    explicit MoonbaseLicense(moonbase::juce_integration::ActivationConfig);
    ~MoonbaseLicense() override;
    bool isLicensed() const noexcept { return licensed.load(std::memory_order_acquire); }
    moonbase::juce_integration::ActivationController& controller() noexcept { return activation; }

private:
    void handleAsyncUpdate() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    static bool validateCachedLicense(const moonbase::juce_integration::ActivationConfig&);

    const moonbase::juce_integration::ActivationConfig config;
    std::atomic<bool> licensed { validateCachedLicense(config) };
    moonbase::juce_integration::ActivationController activation { config };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoonbaseLicense)
};
}
#endif
