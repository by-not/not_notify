# not_notify

> **Language:** English | [Español](README.es.md)

A modular, slot-based visual notification HUD library for **SA-MP** and **open.mp** featuring 5 independent screen channels.

## Included Channels

1. **Notifications (`ShowNotify`)** — Bottom-left card with icon, title, and message.
2. **Supplies (`ShowNotifySupplies`)** — Top-right compact pickup/resource feed.
3. **Ads (`ShowNotifyADS`)** — Bottom-center prominent announcements.
4. **Messages (`ShowMessage`)** — Center above chat with custom font/shadow/outline.
5. **Low Messages (`ShowLowMessage`)** — Bottom-center below chat with custom font/shadow/outline.

## Installation

```pawn
#include <a_samp>

// Optional overrides before including:
#define MAX_NORMAL_NOTIFY        5
#define MAX_NOTIFY_SUPPLIES      3
#define MAX_NOTIFY_ADS           2
#define MAX_NOTIFY_MESSAGES      4
#define MAX_NOTIFY_LOW_MESSAGES  3

#include <not_notify>
```

## Quick Start

```pawn
new id = ShowNotify(playerid, "~g~Success", "Mission completed!", 5000, "ld_beat:chit");
UpdateNotifyText(playerid, id, "~y~Reward", "You received $5,000");
DestroyNotify(playerid, id);
```

## Credits

- Written and refactored by **not**.
- Character width table by **Nero_3D** (`td-string-width`).

## License

Released under the [MIT License](LICENSE).
