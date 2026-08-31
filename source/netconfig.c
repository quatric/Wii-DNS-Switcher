#include <string.h>
#include <ogc/isfs.h>

#include "netconfig.h"

s32 NetConfigLoad(netconfig_t *out) {
    s32 fd = ISFS_Open(NET_CONFIG_PATH, ISFS_OPEN_READ);
    if (fd < 0) return fd;

    s32 ret = ISFS_Read(fd, out, sizeof(netconfig_t));
    ISFS_Close(fd);
    if (ret < 0) return ret;
    if ((u32)ret != sizeof(netconfig_t)) return ISFS_EINVAL;
    return ISFS_OK;
}

s32 NetConfigSave(const netconfig_t *config) {
    s32 fd = ISFS_Open(NET_CONFIG_PATH, ISFS_OPEN_WRITE);
    if (fd < 0) return fd;

    s32 ret = ISFS_Write(fd, config, sizeof(netconfig_t));
    ISFS_Close(fd);
    if (ret < 0) return ret;
    if ((u32)ret != sizeof(netconfig_t)) return ISFS_EINVAL;
    return ISFS_OK;
}

bool NetConfigSlotInUse(const netconfig_t *config, int slot) {
    if (slot < 0 || slot >= WIFI_SLOT_COUNT) return false;
    const connection_t *c = &config->connection[slot];
    return c->selected != 0 || c->ip[0] || c->ip[1] || c->ip[2] || c->ip[3] ||
           c->ssid_length != 0;
}

void NetConfigSetDns(netconfig_t *config, int slot, const u8 primary[4], const u8 secondary[4]) {
    if (slot < 0 || slot >= WIFI_SLOT_COUNT) return;
    connection_t *c = &config->connection[slot];

    BIT_CLEAR(c->selected, DNS_BIT); // manual DNS; DHCP for IP is untouched
    memcpy(c->dns1, primary, 4);
    memcpy(c->dns2, secondary, 4);
}
