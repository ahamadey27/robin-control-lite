# Release Paperwork — Robin Control Lite

Personal admin checklist for shipping v1.0. The technical/build/test plan lives in `spec.md`. This file is **only the bureaucratic side** — accounts to set up, certs to buy, documents to write, listings to file.

---

## Required before launch

### Apple Developer Program — $99/yr (the only mandatory paid expense)
- [x] Enroll at https://developer.apple.com/programs/ (allow 24–48h for approval)
- [ ] Generate **Developer ID Application** certificate in Xcode → Settings → Accounts (signs `.vst3` / `.component` / `.app`)
- [ ] Generate **Developer ID Installer** certificate (signs the `.pkg` installer)
- [ ] Create an **app-specific password** for `notarytool`: appleid.apple.com → Sign-In and Security → App-Specific Passwords
- [ ] Back up cert `.p12` exports + app-specific password to your password manager (lose these = revoke + reissue)

### Identity / legal
- [x] Trademark search on "Robin Control Lite":
  - [x] USPTO TESS — https://tmsearch.uspto.gov
  - [x] EUIPO eSearch — https://www.tmdn.org/tmview
  - [x] KVR Audio + Plugin Boutique + Google for existing products
- [x] Confirm `conduit.dsp` domain ownership + DNS access
- [x] Set up `hello@conduitdsp.com` (forward to your real inbox)
- [x] Draft a 1-page EULA covering: free distribution, no warranty, no reverse-engineering, copyright reservation, VST/AU trademark attributions
- [x] One-line privacy statement (only relevant if you collect any data — mailing list, analytics, crash reports)

### JUCE license
- [x] Re-read current Personal license terms at https://juce.com/get-juce
  - Confirm revenue threshold (~$40k/yr last check), no splash screen, closed-source permitted
- [x] No payment — just confirm you're in compliance

### GitHub repo
- [ ] Decide repo visibility (public vs. private until launch)
- [ ] Add `LICENSE` file at repo root for **your** code (separate from JUCE's license)
- [ ] Add `EULA.md` (the document you drafted above) at repo root

---

## Channels — free, file at launch

- [ ] **KVR Audio listing** — https://www.kvraudio.com/get-listed (free, ~24h moderation, requires short description + screenshots + version + formats + system requirements)
- [ ] **Download page on conduit.dsp** — screenshots, per-platform install instructions, link to EULA, support email

---

## Skipped for v1.0 — revisit when relevant

- [ ] **Windows code signing certificate** (~$200–500/yr) — improves UX but not required; v1.0 ships unsigned with a SmartScreen workaround note
- [ ] **Avid Developer registration + AAX SDK** — only needed when AAX support lands (v1.1+); free signup, free signing for free plugins via Avid's program
- [ ] **Plugin Boutique listing** — requires partner agreement; KVR alone is enough for v1.0
- [ ] **Mailing list provider** (ConvertKit / Buttondown / Listmonk) — only if you plan to email users

---

## Recurring / calendar reminders

- [ ] Apple Developer Program renewal (set calendar reminder 30 days before expiry — lapse breaks future notarizations)
- [ ] Re-check JUCE license terms each major JUCE upgrade
- [ ] Re-check trademark status periodically (every 12 months) if "Robin Control Lite" gains visibility
