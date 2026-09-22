#include "MoonbaseLicense.h"

#if RCL_ENABLE_MOONBASE
namespace rcl
{
bool MoonbaseLicense::validateCachedLicense(const moonbase::juce_integration::ActivationConfig& config)
{
    // Local-only bootstrap on processor construction: session restore and
    // offline rendering must not wait for the editor or a network round trip.
    // Trust only JWT claims re-verified by the SDK, never the JSON store fields.
    try
    {
        const auto file = config.resolvedLicenseFile();
        if (! file.existsAsFile())
            return false;
        moonbase::file_license_store store(std::filesystem::u8path(file.getFullPathName().toStdString()));
        const auto saved = store.load_local_license();
        if (! saved)
            return false;
        moonbase::license_validator validator(config.toLicensingOptions(), config.resolvedDeviceIdResolver());
        const auto verified = validator.validate_token(saved->token);
        const auto age = std::chrono::system_clock::now() - verified.validated_at;
        return verified.method == moonbase::activation_method::offline
            || (age >= std::chrono::seconds::zero() && age <= config.onlineGracePeriod);
    }
    catch (const std::exception&)
    {
        return false;
    }
}

MoonbaseLicense::MoonbaseLicense()
    : MoonbaseLicense(makeMoonbaseConfig())
{
}

MoonbaseLicense::MoonbaseLicense(moonbase::juce_integration::ActivationConfig settings)
    : config(std::move(settings))
{
    // Hosts may construct processors on a worker. Controller state/UI belongs
    // to the message thread, while audio reads only the published atomic.
    triggerAsyncUpdate();
}

MoonbaseLicense::~MoonbaseLicense()
{
    cancelPendingUpdate();
    stopTimer();
    activation.removeChangeListener(this);
    // The SDK controller cancels and joins its own network workers on teardown.
}

void MoonbaseLicense::handleAsyncUpdate()
{
    activation.addChangeListener(this);
    activation.start();
    startTimer(5 * 60 * 1000);
}

void MoonbaseLicense::changeListenerCallback(juce::ChangeBroadcaster*)
{
    // Loading leaves a locally verified cached license usable until the
    // background check settles. All actual results, including revoke, replace it.
    if (activation.screen() != moonbase::juce_integration::ActivationController::Screen::Loading)
        licensed.store(activation.licensedFlag().load(std::memory_order_acquire), std::memory_order_release);
}

void MoonbaseLicense::timerCallback()
{
    if (activation.isBusy())
        return;
    using Screen = moonbase::juce_integration::ActivationController::Screen;
    switch (activation.screen())
    {
        case Screen::Details:
        case Screen::Success:
        case Screen::Trial:
        case Screen::Welcome:
        case Screen::Expired:
            // Reload detects activation/removal by another host/format. start()
            // also fails closed for revoked licenses and exhausted grace;
            // SDK refreshLicense() deliberately preserves old state on errors.
            activation.start();
            break;
        default:
            break; // Never interrupt activation, file selection or deactivation.
    }
}
}
#endif
