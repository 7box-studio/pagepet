# PagePet for CrossPoint

**A reading-powered virtual pet for CrossPoint on the Xteink X3.**

PagePet is an independent project by 7box-studio. The idea is simple: reading helps a small pixel companion grow. The game stays out of the way while a book is open, and the companion can appear on a dedicated screen or the E-Ink sleep screen.

## Status

This repository currently contains a device-independent C++ gameplay core. It validates page turns, tracks reading progress and achievements, derives levels and mood, and encodes a versioned save record. It has not yet been integrated into CrossPoint or tested on an X3. There is no flashable firmware release.

CrossPoint's current SD plugin interface does not run native gameplay code during reading or draw a companion on the sleep screen. A future device release will therefore need a CrossPoint-based firmware build that includes the PagePet runtime. Once that runtime exists, art and configuration can be distributed separately on the SD card. Firmware updates will still be needed for runtime changes and compatibility fixes.

English is the initial game language. Gameplay state contains no display text, so other languages can be added without changing saved data.

## Code layout

| Part | Responsibility |
| --- | --- |
| `companion_state` | Persistent state and page-turn event types |
| `companion_engine` | Page validation, streaks, achievements, unsaved changes |
| `progression` | Level and mood calculations |
| `state_codec` | Versioned save record and checksum validation |

The gameplay core has no dynamic allocation or firmware dependencies. Device storage, screen rendering, and CrossPoint event hooks belong in separate adapters.

## Run host tests

```sh
g++ -std=c++11 -Wall -Wextra -Werror -Iinclude src/*.cpp tests/companion_engine_test.cpp -o /tmp/pagepet-test
/tmp/pagepet-test
```

PagePet is independent of the [CrossPoint Reader project](https://github.com/crosspoint-reader/crosspoint-reader) and is not affiliated with its maintainers or Xteink.

## License

PagePet is available under the [MIT License](LICENSE).
