# PagePet roadmap

This document is the working plan for the first device release. Product
choices can be changed without changing the save format unless explicitly
noted.

## Working MVP

- A local reading companion for CrossPoint on the Xteink X3.
- Confirmed forward page changes award XP; button presses and invalid jumps do
  not.
- Levels, mood, reading streaks, three achievements, and a character choice.
- Fantasy artwork and English UI first.
- A companion screen inside the reader is the first device UI target.
- Sleep-screen rendering, cute animals, equipment progression, and online
  features are post-MVP until the firmware integration is understood.

## Delivery phases

1. **Core foundation** — gameplay rules, save format, art API, and host tests.
2. **CrossPoint adapter** — translate reader events, time, lifecycle, and
   storage into the device-independent core.
3. **First device screen** — show the companion, level, XP, mood, and session
   progress with E-Ink-safe redraw behavior.
4. **Persistence hardening** — atomic or double-slot writes, recovery after
   power loss, and save migrations.
5. **Hardware validation** — long reading sessions, sleep/wake, low battery,
   missing clock, false page events, and visual legibility.
6. **Release packaging** — sync and release gates, fork OTA, firmware build
   instructions, SD-card assets, installation/recovery guide, changelog, and
   a landing page that links to a verified firmware image.
7. **Post-MVP** — sleep-screen support, additional art styles, equipment, and
   further language support.

## Current checkpoint

The device-independent core is implemented and host-tested. The 7box-studio
CrossPoint fork is available in the neighboring `crosspoint-reader` checkout.
The next dependency is an integrated firmware build and X3 hardware validation.
The update and publication gates are in [RELEASE_PIPELINE.md](RELEASE_PIPELINE.md).

## Definition of done for MVP

- A clean CrossPoint build can initialize PagePet and load a valid save.
- Only eligible page events change progress.
- Progress survives normal restart and an interrupted save does not destroy
  the last valid record.
- The companion screen is readable and does not interfere with ordinary
  reading or sleep/wake behavior.
- A new user can install the build and understand how to recover/reset data.
- The firmware's update check stays on the PagePet release channel and cannot
  silently replace the game with an official CrossPoint image.
