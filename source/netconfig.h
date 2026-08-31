#pragma once

#include <gctypes.h>

// Layout of /shared2/sys/net/02/config.dat, reverse-engineered by the
// OfflineNetworkEnabler project (wiibrew.org). Kept byte-for-byte compatible
// so this tool never disturbs SSIDs, keys, proxy settings, or other fields
// it doesn't touch.

#define NET_CONFIG_PATH "/shared2/sys/net/02/config.dat"
#define NET_CONFIG_SIZE 7004

#define WIFI_SLOT_COUNT 3

#define SELECTED_BIT 7 // 1 -> default connection
#define ENABLED_BIT  5 // 1 -> connection enabled (passed the connection test)
#define PROXY_BIT    4 // 1 -> proxy server is used
#define IP_BIT       2 // 1 -> IP obtained via DHCP
#define DNS_BIT      1 // 1 -> DNS obtained via DHCP
#define MODE_BIT     0 // 1 -> wired, 0 -> wireless

#define BIT_CHECK(var, pos) (!!((var) & (1 << (pos))))
#define BIT_SET(var, pos)   ((var) |= (1 << (pos)))
#define BIT_CLEAR(var, pos) ((var) &= ~(1 << (pos)))

typedef struct __attribute__((packed)) _connection {
    u8 selected;
    u8 padding_1[3];

    u8 ip[4];
    u8 netmask[4];
    u8 gateway[4];
    u8 dns1[4];
    u8 dns2[4];

    u8 padding_2[2];
    u16 mtu;

    u8 padding_3[8];

    u8 use_proxy;
    u8 use_proxy_userandpass;
    u8 padding_p1[2];
    u8 proxy_name[255];
    u8 padding_p2;
    u16 proxy_port;
    u8 proxy_username[32];
    u8 padding_p3;
    u8 proxy_password[32];

    u8 padding_4;

    u8 use_proxy_copy;
    u8 use_proxy_userandpass_copy;
    u8 padding_p1_copy[2];
    u8 proxy_name_copy[255];
    u8 padding_p2_copy;
    u16 proxy_port_copy;
    u8 proxy_username_copy[32];
    u8 padding_p3_copy;
    u8 proxy_password_copy[32];

    u8 padding_5[1297];

    u8 ssid[32];
    u8 padding_6;
    u8 ssid_length;
    u8 padding_7[2];

    u8 padding_8;
    u8 encryption;
    u8 padding_9[2];

    u8 padding_10;
    u8 key_length;
    u8 padding_11[2];

    u8 key[64];

    u8 padding_12[236];
} connection_t;

typedef struct __attribute__((packed)) _netconfig {
    u8 header[8];
    connection_t connection[WIFI_SLOT_COUNT];
} netconfig_t;

_Static_assert(sizeof(connection_t) == 2332, "connection_t layout drifted");
_Static_assert(sizeof(netconfig_t) == NET_CONFIG_SIZE, "netconfig_t layout drifted");

// Loads /shared2/sys/net/02/config.dat into *out via ISFS. Returns an ISFS
// error code (0 on success).
s32 NetConfigLoad(netconfig_t *out);

// Writes *config back to /shared2/sys/net/02/config.dat via ISFS.
s32 NetConfigSave(const netconfig_t *config);

// True if the slot has any connection data set up (SSID/IP fields non-zero).
bool NetConfigSlotInUse(const netconfig_t *config, int slot);

// Sets manual primary/secondary DNS on a slot without touching anything
// else. Clears the "DNS via DHCP" flag so the manual values stick.
void NetConfigSetDns(netconfig_t *config, int slot, const u8 primary[4], const u8 secondary[4]);
