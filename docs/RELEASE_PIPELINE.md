# PagePet Reader release pipeline

PagePet runs inside the standalone PagePet Reader firmware image. A firmware
update therefore replaces the whole application image, including the reader
and companion. Keeping PagePet as a Git submodule helps source management but
does not protect an installed game from an incompatible firmware image.

The PagePet gameplay core and the firmware adapter evolve together. The core
must remain device-independent; hardware, storage, display, input, and update
logic belong to PagePet Reader. Do not make a release depend on an upstream
reader project accepting a plugin or event hook.

## Source and release flow

1. Pin the FreeInk SDK and PagePet core revisions in the PagePet Reader
   firmware repository. A dependency update must be reviewed together with
   the adapter and storage changes it enables.
2. The firmware checks run the PagePet core tests, the X3 build, and the
   adapter gates below. Failed checks stop the merge and release.
3. On a passing change, review reader events, storage, display, lifecycle, and
   update-channel code. Test the candidate image on an X3 before marking it
   stable. A passing build cannot prove behavior on hardware.
4. Publish a PagePet Reader release with the firmware and dependency commits
   in its notes. Give the image an unambiguous X3 board label and record its
   checksum.
5. The PagePet website lists only stable PagePet Reader releases, their device
   label, checksum, and installation/recovery instructions.

## Required release gates

- PagePet host tests, including valid/invalid page sequences, timer boundaries,
  missing clock, session restart, and save validation.
- A clean PagePet Reader firmware build for the X3 environment with the pinned
  PagePet revision compiled in. Compilation of a standalone PagePet library is
  not sufficient.
- Adapter tests using simulated PagePet Reader events for TXT (and future book
  formats): ordinary forward page, duplicate render, backward page, jump,
  chapter boundary, book change, sleep/wake, and restart. The event stream
  must yield the expected XP and save state.
- Persistence tests with a fake storage layer: write failure, interrupted
  write, one corrupt slot, both corrupt slots, and migration from supported
  save versions. A failed write must leave the engine dirty.
- An update-channel check asserting that the PagePet Reader build queries the
  PagePet Reader release feed and rejects or does not offer a firmware image
  without PagePet.
- A release artifact check for device identity, image size, version string,
  PagePet marker/version, checksum, and presence of the expected `.bin`.

The adapter, persistence, and OTA gates become executable when those parts
exist. Until then, their absence blocks a stable firmware release.

## Website and installation

The website must serve or link an immutable, checked PagePet Reader image. The
installation page should name the X3 image, checksum, recovery path, and the
fact that the companion save lives independently on the SD card. A later
one-click flasher on the PagePet website needs a browser/device compatibility
check and an explicit firmware manifest for the X3.
