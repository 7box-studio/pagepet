# CrossPoint integration contract

The gameplay core deliberately knows nothing about CrossPoint, Xteink, SD
cards, displays, or input buttons. The firmware adapter should be a thin
translation layer around `CompanionEngine`.

## Startup and restore

1. Construct one `CompanionEngine` instance for the device session.
2. Read exactly `CompanionEngine::kRecordSize` bytes from the PagePet save
   location.
3. Call `decode()`. If validation fails, keep a default state and treat the
   next successful commit as a new valid save.
4. Call `beginSession(monotonic_ms)` when a reading session starts. This resets
   the session page counter and the page anchor; lifetime progress remains
   intact. If a saved session count was nonzero, the reset becomes dirty and
   needs a storage commit.

## Page events

Call `onPageTurn()` only for a reader event that represents the logical page
currently rendered, not for a physical button press. Populate the event as
follows:

| Field | Adapter requirement |
| --- | --- |
| `book_id` | Stable identifier for the opened book during the session. |
| `page_index` | Logical rendered page, increasing by one for a normal forward page. |
| `monotonic_ms` | Milliseconds since boot; a wrapping `uint32_t` is supported. |
| `unix_seconds` | Trusted wall-clock time, or zero when unavailable. |
| `local_day` | Local civil day number, or a negative value when unavailable. |
| `clock_trusted` | True only when the firmware considers the clock reliable. |
| `local_hour` | 0–23 when known, otherwise 255. |
| `weekday` | Monday=0 through Sunday=6 when known, otherwise 255. |

The first event anchors the session and earns no XP. A later event earns XP
only when it is the next page of the same book and arrives 15–300 seconds after
the previous anchor. Invalid events still become the new anchor, allowing the
reader to recover after a jump or book change. A redraw of the same logical
page is ignored without moving the anchor.

## Persistence lifecycle

After `onPageTurn()` returns true, the state is dirty. The adapter should also
commit at a safe lifecycle boundary (for example, book close or sleep entry).
For long sessions, poll `flushDue(monotonic_ms)` and commit when it returns
true.

The safe commit sequence is:

1. Allocate a `kRecordSize` byte buffer owned by the adapter.
2. Call `encode()` into that buffer.
3. Write the complete record using the storage adapter's power-loss-safe
   mechanism.
4. Call `markFlushed()` only after the write is confirmed.

Never mark the engine clean before storage confirms the complete record.

## UI and artwork

The UI may read `engine.state()`, call `levelForXp()` and `moodAt()`, and use
`renderCharacter()` for the selected fantasy sprite. Rendering must not mutate
gameplay state. The current core has no text-resource layer; display strings
belong to the firmware UI so localization does not affect saved data.

## Adapter acceptance checks

- A page button press without a new rendered page does not award XP.
- A page jump, backwards navigation, or book change does not award XP.
- A valid page event across the `uint32_t` monotonic timer wrap still works.
- A failed decode leaves the engine state unchanged.
- A failed storage write leaves the engine dirty for a later retry.
