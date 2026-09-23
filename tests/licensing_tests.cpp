#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <iostream>

#if JUCE_MAC
void initialiseLicensingTestApplication();
#endif

int main()
{
#if JUCE_MAC
    initialiseLicensingTestApplication();
#endif
    juce::ScopedJuceInitialiser_GUI gui;
    int failures = 0;
    auto check = [&](bool ok, const char* description)
    {
        std::cout << (ok ? "PASS " : "FAIL ") << description << '\n';
        if (! ok) ++failures;
    };
    const juce::File fixtures(RCL_LICENSE_FIXTURES);
    const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getNonexistentChildFile("rcl-license-tests-" + juce::String::charToString(0x00e9)
                                + juce::String::charToString(0x65e5), {}, false);
    directory.createDirectory();
    auto config = rcl::makeMoonbaseConfig();
    check(config.onlineGracePeriod == std::chrono::hours(24 * 90), "Production online grace is 90 days");
    config.endpoint = "https://example.invalid";
    config.productId = "rcl-test-only";
    config.accountId = "rcl-test-only";
    config.publicKey = fixtures.getChildFile("public.pem").loadFileAsString();
    config.licenseFile = directory.getChildFile("license.mb");
    config.deviceIdResolver = std::make_shared<moonbase::static_device_id_resolver>(
        "Test machine", "rcl-test-device");

    moonbase::file_license_store store(std::filesystem::u8path(config.licenseFile.getFullPathName().toStdString()));
    auto writeToken = [&](juce::String token)
    {
        // Deliberately forge the untrusted outer cache metadata. Only the signed
        // JWT must decide entitlement, product, machine, method and freshness.
        moonbase::license cached;
        cached.token = token.trim().toStdString();
        cached.method = moonbase::activation_method::offline;
        cached.validated_at = std::chrono::system_clock::now();
        store.store_local_license(cached);
    };
    {
        rcl::MoonbaseLicense license(config);
        check(! license.isLicensed(), "Missing license starts locked before any message loop/editor");
    }
    for (const auto* name : { "valid-offline", "wrong-product", "wrong-device", "expired", "stale-online" })
    {
        writeToken(fixtures.getChildFile(juce::String(name) + ".jwt").loadFileAsString());
        rcl::MoonbaseLicense license(config);
        check(license.isLicensed() == (juce::String(name) == "valid-offline"), name);
    }
    {
        // Reuse the signed online fixture with a synthetic policy boundary;
        // do not change this workstation's clock to exercise time handling.
        auto boundaryConfig = config;
        const auto age = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()) - std::chrono::seconds(1577836800);
        boundaryConfig.onlineGracePeriod = age + std::chrono::hours(1);
        rcl::MoonbaseLicense withinGrace(boundaryConfig);
        check(withinGrace.isLicensed(), "Signed online license inside grace unlocks locally");
        boundaryConfig.onlineGracePeriod = age - std::chrono::hours(1);
        rcl::MoonbaseLicense beyondGrace(boundaryConfig);
        check(! beyondGrace.isLicensed(), "Online license outside grace stays locked despite forged cache date");
    }
    auto tampered = fixtures.getChildFile("valid-offline.jwt").loadFileAsString().trim();
    const int signatureStart = tampered.lastIndexOfChar('.') + 1;
    tampered = tampered.replaceSection(signatureStart, 1, tampered[signatureStart] == 'A' ? "B" : "A");
    writeToken(tampered);
    {
        rcl::MoonbaseLicense license(config);
        check(! license.isLicensed(), "Tampered signature fails closed");
    }
    config.licenseFile.replaceWithText("{corrupt");
    {
        rcl::MoonbaseLicense license(config);
        check(! license.isLicensed(), "Corrupt cache fails closed without a crash");
    }
    config.licenseFile.replaceWithText(juce::String::repeatedString("x", 1024 * 1024 + 1));
    {
        rcl::MoonbaseLicense license(config);
        check(! license.isLicensed(), "Oversized cache fails closed before JSON parsing");
    }

    moonbase::juce_integration::LicenseGate gate;
    gate.prepare(48000.0);
    gate.reset(false);
    juce::AudioBuffer<float> buffer(2, 512);
    auto fill = [&] { for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 512; ++i) buffer.setSample(ch, i, 1.0f); };
    auto process = [&](bool enabled) { gate.process(buffer.getArrayOfWritePointers(), 2, 512, enabled); };
    fill(); process(false);
    check(buffer.getMagnitude(0, 512) == 0.0f, "Unlicensed audio is silent on every channel");
    fill(); process(true);
    check(buffer.getSample(0, 0) > 0.0f && buffer.getSample(0, 0) < 0.01f
          && buffer.getSample(1, 511) == 1.0f, "Activation fades in without a discontinuity");
    fill(); process(false);
    check(buffer.getSample(0, 0) > 0.99f && buffer.getSample(1, 511) == 0.0f,
          "Deactivation fades to silence");

    // Verify the gate is wired into the actual processor, not just the helper.
    // No dispatch loop or network calls: a real existing user license is respected.
    {
        NewProjectAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        const auto wav = directory.getChildFile("source.wav");
        juce::WavAudioFormat format;
        auto stream = wav.createOutputStream();
        std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.release(), 48000, 1, 16, {}, 0));
        fill();
        writer->writeFromAudioSampleBuffer(buffer, 0, 512);
        writer.reset();
        check(processor.sampleLoader.loadSample(0, wav), "Load a real sample for processor gate test");
        processor.rebuildLoadedIndices();
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
        processor.requestTrigger();
        processor.auditionSample(0);
        processor.processBlock(buffer, midi);
        if (! processor.getLicense().isLicensed())
            check(buffer.getMagnitude(0, 512) == 0.0f && processor.outputPeakLevel.load() == 0.0f,
                  "Actual processor silences MIDI / Trigger / audition with no editor");
        else
            std::cout << "SKIP locked processor assertion: this user already has a valid product license\n";
        processor.releaseResources();
    }

    // Exercise asynchronous controller startup and UI-independent entitlement
    // changes using only synthetic offline tokens. No tenant requests or seats.
    writeToken(fixtures.getChildFile("valid-offline.jwt").loadFileAsString());
    {
        rcl::MoonbaseLicense license(config);
        struct Probe final : juce::Timer
        {
            std::function<void()> tick;
            void timerCallback() override { tick(); }
        } probe;
        int ticks = 0;
        bool completed = false;
        probe.tick = [&]
        {
            using Screen = moonbase::juce_integration::ActivationController::Screen;
            auto& controller = license.controller();
            if (controller.screen() != Screen::Details && ++ticks < 500)
                return;
            check(controller.screen() == Screen::Details, "Background startup restores cached offline license");
            controller.dispatchPendingMessages();
            check(license.isLicensed(), "Controller result preserves cached unlock");
            {
                moonbase::juce_integration::ActivationComponent editor(controller);
            }
            check(license.isLicensed(), "Closing activation UI does not revoke processor entitlement");
            controller.deactivate();
            controller.dispatchPendingMessages();
            check(! license.isLicensed() && ! config.licenseFile.existsAsFile(),
                  "Offline deactivation removes local license and closes gate");
            controller.setOfflineResponse(fixtures.getChildFile("valid-offline.jwt"));
            controller.activateOffline();
            controller.dispatchPendingMessages();
            check(license.isLicensed() && config.licenseFile.existsAsFile(),
                  "Offline response import persists verified license and opens gate");
            completed = true;
            probe.stopTimer();
            juce::MessageManager::getInstance()->stopDispatchLoop();
        };
        probe.startTimer(10);
        juce::MessageManager::getInstance()->runDispatchLoop();
        check(completed, "Asynchronous lifecycle checks completed");
    }
    directory.deleteRecursively();
    return failures == 0 ? 0 : 1;
}
