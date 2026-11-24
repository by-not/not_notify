# not_notify

> **Idioma:** [English](README.md) | Español

Librería modular de notificaciones visuales en pantalla para **SA-MP** y **open.mp** con 5 canales independientes basados en slots y timers.

## Canales incluidos

1. **Notificaciones (`ShowNotify`)** — Esquina inferior izquierda con icono, título y mensaje.
2. **Suministros (`ShowNotifySupplies`)** — Esquina superior derecha compacto para ítems/recursos.
3. **Anuncios (`ShowNotifyADS`)** — Centro-inferior destacado para avisos globales.
4. **Mensajes (`ShowMessage`)** — Centro sobre el chat con fuente, sombra y outline configurables.
5. **Mensajes Bajos (`ShowLowMessage`)** — Centro-inferior bajo el chat con fuente, sombra y outline configurables.

## Instalación

```pawn
#include <a_samp>

// Configuración opcional antes del #include:
#define MAX_NORMAL_NOTIFY        5
#define MAX_NOTIFY_SUPPLIES      3
#define MAX_NOTIFY_ADS           2
#define MAX_NOTIFY_MESSAGES      4
#define MAX_NOTIFY_LOW_MESSAGES  3

#include <not_notify>
```

## Uso rápido

```pawn
new id = ShowNotify(playerid, "~g~Exito", "Mision completada!", 5000, "ld_beat:chit");
UpdateNotifyText(playerid, id, "~y~Recompensa", "Recibiste $5,000");
DestroyNotify(playerid, id);
```

## Créditos

- Escrito y refactorizado por **not**.
- Tabla de anchos de caracteres por **Nero_3D** (`td-string-width`).

## Licencia

Distribuido bajo la [Licencia MIT](LICENSE).
