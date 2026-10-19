# Wii DNS Switcher

**Version:** 1.1 · **Coder:** quatric

A Wii Homebrew Channel app for quickly switching the DNS used by existing
Wi-Fi connections. Wii port of
[3DS DNS Switcher](https://github.com/quatric/3DS-DNS-Switcher).

## Download

Grab the latest build from the [latest release](../../releases/latest): `Wii-DNS-Switcher.zip`. The loose `dns-switcher.dol` and `dns-switcher-xz.dol` are there too, for `wiiload`. Every push to the main branch refreshes it, and tagged versions get their own entry on the [releases page](../../releases).

1. Extract `Wii-DNS-Switcher.zip` to the root of your SD card (or USB drive).
2. Open Wii DNS Switcher from the Homebrew Channel.

## Features

- Saves named primary-DNS profiles on the SD card.
- Shows the profile list on every launch, including first setup.
- Press `1` to choose and remember the target: all configured Wi-Fi slots or
  connection slot 1, 2, or 3.
- Press `A` to apply the highlighted profile immediately.
- Always uses `1.1.1.1` as the secondary DNS.
- Exits automatically after a successful change.
- Preserves SSIDs, passwords, DHCP, proxy, and other connection settings.

Profiles are stored at `sd:/wii/dns-switcher/profiles.txt`, and the target is
stored at `sd:/wii/dns-switcher/settings.txt`.

## Installation

Copy `dist/apps/dns-switcher` (built via `make package`) to `apps/` on your
SD card and launch it from the Homebrew Channel.

## Controls

Navigation works with a Wiimote, Wiimote + Nunchuk, Classic Controller, GameCube
pad, or a USB keyboard — all normalized onto the same button set by
[libwiiinput](https://github.com/quatric/libwiiinput)
(see [source/controller.c](source/controller.c)).
Building with `WITH_WIIDRC=1` / `WITH_WUPC=1` / `WITH_SICKSAXIS=1` also adds
Wii U GamePad, Wii U Pro Controller, and DualShock 3 support respectively
(see Building below).

- D-Pad Up/Down / Keyboard Up/Down: choose a profile or **Add DNS profile**
- `A` / Keyboard Enter / Space: select/apply
- `1` / Keyboard `1` / Tab: cycle and save the Wi-Fi-slot target
- `2` / Keyboard `2` / Delete: delete the highlighted profile
- `HOME` / Keyboard Esc: exit without applying

Adding a profile opens the Wii-style on-screen keyboard from
[libwiiosk](https://github.com/quatric/libwiiosk)
([source/onscreen_keyboard.c](source/onscreen_keyboard.c)). Point with the Wii
Remote or a USB mouse and press `A` (or the left mouse button) to type, or use
the D-Pad or a stick to move between keys. `B` backspaces, `+` confirms and
`HOME` cancels. A connected USB keyboard also types directly into the same
field at any time (Enter to confirm, Esc to cancel). Clone with
`--recurse-submodules` so libwiiosk is fetched. You can also skip the UI entirely and edit
`sd:/wii/dns-switcher/profiles.txt` by hand; each line is
`name<TAB>dns.ip.address`.

## Building

Install a current devkitPPC/libogc toolchain (devkitPro) and run `make` from
the project root. `make package` additionally stages `dist/apps/dns-switcher`
for copying to an SD card, with `dns-switcher-xz.dol` (the same program packed
into a self-extracting `.dol` a fraction of the size) as `boot.dol`.

The first build downloads and builds [dolxz](https://github.com/FIX94/dolxz), which needs a host C compiler and liblzma (Debian/Ubuntu: `apt-get install liblzma-dev pkg-config`; macOS: `brew install xz`).
`make NO_DOLXZ=1` packages the plain `dns-switcher.dol` instead.

For the optional Wii U GamePad / Wii U Pro Controller / DualShock 3 support,
first run `./scripts/build_controller_portlibs.sh` to build the needed
portlibs into `portlibs/ppc`, then build with the matching flag(s), e.g.:

```bash
./scripts/build_controller_portlibs.sh
make WITH_WIIDRC=1 WITH_WUPC=1 WITH_SICKSAXIS=1
```

The `/shared2/sys/net/02/config.dat` layout and CRC-free field offsets are
based on the OfflineNetworkEnabler project (wiibrew.org), reverse-engineered
by studying manually-edited Wi-Fi connections. This tool reads/writes that
file directly via `ISFS_Open`/`ISFS_Read`/`ISFS_Write`, unlike the 3DS
version which goes through the `cfgu` service — the Wii has no equivalent
userland Wi-Fi config API.

## Important

This software directly modifies the console's NAND-stored Wi-Fi connection
data. Keep a backup of your network settings (e.g. via a NAND backup tool)
and use it at your own risk. It only ever touches the DNS fields of slots
that already have a connection configured — SSID, password, IP mode, and
proxy settings are left untouched.
