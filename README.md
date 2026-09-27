# PagePet for PagePet Reader

**A reading-powered virtual companion for PagePet Reader, an independent CrossPoint fork.**

PagePet is an independent project by 7box-studio. The idea is simple: reading helps a small pixel companion grow. The game stays out of the way while a book is open, and the companion can appear on a dedicated screen or the E-Ink sleep screen.

## Status

This repository contains a device-independent C++ gameplay core. It validates page turns, tracks reading progress and achievements, derives levels and mood, and encodes a versioned save record. The core is integrated into the 7box-studio PagePet Reader firmware through a thin adapter. Device validation and a stable flashable firmware release are still pending.

PagePet Reader is a separate firmware fork based on the [CrossPoint Reader project](https://github.com/crosspoint-reader/crosspoint-reader). The library stays independent of device storage, display, and input code; those responsibilities belong to the firmware adapter. Firmware updates are required for runtime changes and compatibility fixes, while the companion save remains on the SD card.

English is the initial game language. Gameplay state contains no display text, so other languages can be added without changing saved data.

## Code layout

| Part | Responsibility |
| --- | --- |
| `companion_state` | Persistent state and page-turn event types |
| `companion_engine` | Page validation, streaks, achievements, unsaved changes |
| `progression` | Level and mood calculations |
| `state_codec` | Versioned save record and checksum validation |
| `character_art` | Seven 16×16 monochrome character sprites and packed bitmap output |

The character set includes a wanderer, bibliomancer, wizard, archivist, orc,
elf, and knight. Set `CompanionState::character_class` to select one; the choice
is stored in the existing save record. `renderCharacter()` produces a 16×16
1-bit bitmap that a future firmware screen adapter can scale and display.
Fantasy is the current visual style. The save record also has a separate style
field for a future settings choice such as cute animals; reading progress stays
the same when the artwork changes. Cute animal sprites and a settings screen
have not been implemented.

The gameplay core has no dynamic allocation or firmware dependencies. Device storage, screen rendering, and CrossPoint event hooks belong in separate adapters.
The `library.json` manifest lets PlatformIO compile this repository as a
library when it is checked out under CrossPoint's `lib/` directory.

The current working plan is documented in [`docs/ROADMAP.md`](docs/ROADMAP.md),
and the expected firmware boundary is defined in
[`docs/INTEGRATION_CONTRACT.md`](docs/INTEGRATION_CONTRACT.md).
The CrossPoint sync, firmware release, and web installation gates are described
in [`docs/RELEASE_PIPELINE.md`](docs/RELEASE_PIPELINE.md).

## Run host tests

```sh
g++ -std=c++11 -Wall -Wextra -Werror -Iinclude src/*.cpp tests/companion_engine_test.cpp -o /tmp/pagepet-test
/tmp/pagepet-test
```

PagePet is independent of the [CrossPoint Reader project](https://github.com/crosspoint-reader/crosspoint-reader) and is not affiliated with its maintainers or Xteink.

## License

PagePet is available under the [MIT License](LICENSE).
