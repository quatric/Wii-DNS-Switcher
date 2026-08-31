# Wii DNS Switcher

A Wii Homebrew Channel app for quickly switching the DNS used by existing
Wi-Fi connections. Wii port of
[3DS DNS Switcher](https://github.com/quatric/3DS-DNS-Switcher).

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

- Wiimote D-Pad Up/Down: choose a profile or **Add DNS profile**
- `A`: select/apply
- `1`: cycle and save the Wi-Fi-slot target
- `2`: delete the highlighted profile
- `HOME`: exit without applying

Adding a profile from the on-console UI requires a USB keyboard plugged into
the Wii. Without one, edit `sd:/wii/dns-switcher/profiles.txt` directly —
each line is `name<TAB>dns.ip.address`.

## Building

Install a current devkitPPC/libogc toolchain (devkitPro) and run `make` from
the project root. `make package` additionally stages `dist/apps/dns-switcher`
for copying to an SD card.

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
