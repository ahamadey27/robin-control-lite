# Privacy

The canonical privacy policy for everything Conduit DSP — the website, the
download flow, and any future paid products — lives at:

**https://conduitdsp.com/privacy-policy/**

This file is a short repo-side note about the plugin itself.

## What Robin Control Lite (the plugin) does with your data

The existing public builds and DRM-free Beta 1 do not connect to the internet.

**Moonbase-enabled 2.0.0** connects to Conduit DSP's
Moonbase tenant for license activation, validation, and deactivation. The SDK
uses a device fingerprint and device label to bind the license to a machine.
Activation happens in a browser; the plugin stores a signed license locally,
including the license holder's name and email returned by Moonbase. Network
requests also identify the SDK/JUCE/OS software versions through a User-Agent.
Optional SDK analytics, host/locale metadata collection, and update prompts
are disabled. There is no sample, preset, project, or crash-report upload.

The repository EULA now describes this v2.0.0 behavior. The canonical website
privacy policy still needs reconciliation before public launch; updating this
file does not publish a website policy. See `FINAL_RELEASE_2.0.0.md`.

## What the download flow does with your data

Moonbase handles the private v2.0.0 license/delivery flow, account portal and
transactional license emails. The test delivery email/download flow has been
verified. Public self-service free-license acquisition and migration from the
older website flow still need launch configuration. The older website uses
MailerLite for email subscriptions; receiving a license is not permission to
subscribe the customer to marketing. See the canonical policy above.

## Contact

Questions: hello@conduitdsp.com

Conduit DSP LLC — last updated 2026-09-22
