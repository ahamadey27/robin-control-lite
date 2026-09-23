#pragma once

#include <moonbase_licensing/moonbase_licensing.h>

namespace rcl
{
// Public verification material from the product's Moonbase implementation guide,
// 2026-09-22. This is not an API secret and cannot issue licenses.
inline moonbase::juce_integration::ActivationConfig makeMoonbaseConfig()
{
    moonbase::juce_integration::ActivationConfig config;
    config.endpoint = "https://conduitdsp.moonbase.sh";
    config.productId = "robin-control-lite";
    config.publicKey = R"pem(-----BEGIN RSA PUBLIC KEY-----
MIIBCgKCAQEAkzPrccUuOf6oVkHqMeHjkOBJvvl6TpRcKonikee+Y0+5YdRb5AKy
lbP8tB1m7qkNuii9I2P2alu7NjT3j7OSjXlRVbaENQC4HvyJ+2oDe8JB0mrZrnR4
cqpLEJcV3pc2Z9d66IGYehAF0OiifvTnNKjlrFjahTRFQ85s+qt5X7aCo8I7kVFU
NGoazARDrDmabxhQahCaEJcuPqn0/nPfaPBv5uqYyVSkAoi8qipOyTVZRPlX/VwX
919OEmBXKC1uPijsENWsb2zs48DgGUgO2RQR2mMq2JDg8e7By27C84pZ4oRGwZhV
bwpi4NOAHdSwhDyeqjDkcLToIaQ1dBgg6wIDAQAB
-----END RSA PUBLIC KEY-----)pem";
    config.productName = "Robin Control Lite";
    config.manufacturerName = "conduit.dsp";
    config.applicationVersion = JucePlugin_VersionString;
    config.licenseFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Conduit DSP").getChildFile("Robin Control Lite").getChildFile("license.mb");

    // Matches the current dashboard. Seat count and trial eligibility remain
    // server-controlled; no trial entitlement is manufactured in this client.
    config.enableOffline = true;
    config.enableUpdatePrompt = false;
    config.autoPresentUpdate = false;
    config.analytics.enabled = false;
    config.analytics.includeHostInfo = false;
    config.analytics.includeLocaleInfo = false;
    // Alex selected 90 days on 2026-09-22. Permanent offline activations
    // do not use this online-license grace period.
    config.onlineGracePeriod = std::chrono::hours(24 * 90);
    config.onlineCheckInterval = std::chrono::minutes(5);
    config.httpConnectTimeout = std::chrono::seconds(5);
    config.httpRequestTimeout = std::chrono::seconds(15);
    config.strings.welcomeBody = "Activate your free Robin Control Lite license through conduit.dsp.";
    config.accent = juce::Colour(0xff2d7a7a);
    config.palette.backgroundTop = juce::Colour(0xff29241e);
    config.palette.backgroundBottom = juce::Colour(0xff14110e);
    config.palette.panelTop = juce::Colour(0xff26211b);
    config.palette.panelBottom = juce::Colour(0xff191510);
    config.palette.textPrimary = juce::Colour(0xffece5d4);
    config.palette.textBody = juce::Colour(0xffc0b8a8);
    config.palette.link = juce::Colour(0xff85bcbc);
    return config;
}
}
