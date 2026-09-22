# Privacy

The canonical privacy policy for everything Conduit DSP — the website, the
download flow, and any future paid products — lives at:

**https://conduitdsp.com/privacy-policy/**

This file is a short repo-side note about the plugin itself.

## What Robin Control Lite (the plugin) does with your data

The existing public builds and DRM-free Beta 1 do not connect to the internet.

The **unreleased Moonbase-enabled 2.0.0 build** connects to Conduit DSP's
Moonbase tenant for license activation, validation, and deactivation. The SDK
uses a device fingerprint and device label to bind the license to a machine.
Activation happens in a browser; the plugin stores a signed license locally,
including the license holder's name and email returned by Moonbase. Network
requests also identify the SDK/JUCE/OS software versions through a User-Agent.
Optional SDK analytics, host/locale metadata collection, and update prompts
are disabled. There is no sample, preset, project, or crash-report upload.

This describes the development implementation. Before distributing it, the
canonical website policy and the EULA's existing offline-only wording must
be reconciled with Moonbase licensing; see `MOONBASE_INTEGRATION.md`.

## What the download flow does with your data

To download Robin Control Lite from conduitdsp.com you provide an email
address. That email is processed by MailerLite on Conduit DSP's behalf, used
to send you the download link and occasional product updates, and is handled
per the privacy policy linked above. You can unsubscribe at any time.

## Contact

Questions: hello@conduitdsp.com

Conduit DSP LLC — last updated 2026-09-22
