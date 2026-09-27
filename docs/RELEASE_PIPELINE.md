# CrossPoint sync and PagePet release pipeline

PagePet runs inside a CrossPoint firmware image. A CrossPoint update therefore
replaces the whole application image, including PagePet. The current fork's OTA
updater checks the official CrossPoint repository in
`src/network/OtaUpdater.cpp`; this must change before distributing PagePet.
Keeping PagePet as a Git submodule helps source management but does not protect
the installed game during an official firmware update.

CrossPoint's planned SD plugins are described in `ROADMAP.md` as web-server
integrations and whitelisted background jobs. The same roadmap and `SCOPE.md`
explicitly exclude interactive games from the core project. That planned
plugin interface does not promise page events, on-device screens, or native
gameplay execution. Do not make the PagePet release depend on a plugin PR being
accepted upstream. A small, general reader-event API may be worth discussing
with maintainers later, but only if it stands on its own as a reading feature.

## Source and release flow

1. Track the official CrossPoint upstream in the 7box-studio fork. A scheduled
   job detects new upstream release tags and opens a sync PR against a pinned
   upstream commit. It must not publish firmware merely because a tag exists.
2. The sync PR runs the existing CrossPoint build and static checks plus the
   PagePet gates below. Failed checks stop the merge and release.
3. On a passing sync PR, review any changed reader event, storage, display,
   lifecycle, and OTA code. Test the candidate image on an X3 before marking
   it stable. A passing build cannot prove behavior on hardware.
4. Publish a PagePet fork release with the CrossPoint base version and both
   source commits in its notes. Give the firmware image an unambiguous X3/X4
   board label and record its checksum.
5. The PagePet website lists only stable fork releases, their CrossPoint base
   version, checksum, and installation/recovery instructions. The first web
   flow can link users to CrossPoint's existing flasher and its Custom `.bin`
   option. A dedicated PagePet web flasher is optional later.

## Required sync PR gates

- PagePet host tests, including valid/invalid page sequences, timer boundaries,
  missing clock, session restart, and save validation.
- A clean CrossPoint firmware build for the X3/X4 environment with the pinned
  PagePet revision compiled in. Compilation of a standalone PagePet library is
  not sufficient.
- Adapter tests using captured or simulated CrossPoint events for EPUB, TXT,
  and XTC: ordinary forward page, duplicate render, backward page, jump,
  chapter boundary, book change, sleep/wake, and restart. The event stream
  must yield the expected XP and save state.
- Persistence tests with a fake storage layer: write failure, interrupted
  write, one corrupt slot, both corrupt slots, and migration from supported
  save versions. A failed write must leave the engine dirty.
- An OTA channel check asserting that the PagePet build queries the PagePet
  release feed and rejects or does not offer a firmware image without PagePet.
  A normal CrossPoint release must not silently remove the game.
- A release artifact check for device identity, image size, version string,
  PagePet marker/version, checksum, and presence of the expected `.bin`.

The adapter, persistence, and OTA gates become executable when those parts
exist. Until then, their absence blocks a stable firmware release.

## Website and installation

The website must serve or link an immutable, checked release image. The
initial installation can use the official CrossPoint web flasher's Custom
`.bin` path; the user downloads the PagePet build and selects it there.
Publishing a PagePet image through CrossPoint's official version picker would
require cooperation from that site's maintainers, so the plan does not depend
on it. A later one-click flasher on the PagePet website needs a browser/device
compatibility check and an explicit firmware manifest for the X3.

The update page should explain that flashing official CrossPoint later will
replace PagePet code, while the PagePet save on SD remains independent of the
firmware image. Returning to PagePet then requires flashing a PagePet build.
