#include <stdio.h>
#include <string.h>
#include <ogc/isfs.h>

#include "netconfig.h"

// IOS DMAs straight into/out of these buffers, so ISFS_Read/ISFS_Write need
// 32-byte-aligned memory (and a 32-byte-multiple length is safest). Callers'
// structs live on the stack and are not aligned, so everything bounces
// through this buffer.
#define ISFS_BUF_SIZE ((sizeof(netconfig_t) + 31) & ~31u)
static u8 isfsBuf[ISFS_BUF_SIZE] __attribute__((aligned(32)));

s32 NetConfigLoad(netconfig_t *out) {
    s32 fd = ISFS_Open(NET_CONFIG_PATH, ISFS_OPEN_READ);
    if (fd < 0) return fd;

    memset(isfsBuf, 0, sizeof(isfsBuf));
    s32 ret = ISFS_Read(fd, isfsBuf, sizeof(netconfig_t));
    ISFS_Close(fd);
    if (ret < 0) return ret;
    if ((u32)ret != sizeof(netconfig_t)) return ISFS_EINVAL;
    memcpy(out, isfsBuf, sizeof(netconfig_t));
    return ISFS_OK;
}

s32 NetConfigSave(const netconfig_t *config) {
    s32 fd = ISFS_Open(NET_CONFIG_PATH, ISFS_OPEN_WRITE);
    if (fd < 0) return fd;

    memset(isfsBuf, 0, sizeof(isfsBuf));
    memcpy(isfsBuf, config, sizeof(netconfig_t));
    s32 ret = ISFS_Write(fd, isfsBuf, sizeof(netconfig_t));
    ISFS_Close(fd);
    if (ret < 0) return ret;
    if ((u32)ret != sizeof(netconfig_t)) return ISFS_EINVAL;
    return ISFS_OK;
}

int NetConfigBackup(const netconfig_t *config, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    size_t n = fwrite(config, 1, sizeof(netconfig_t), f);
    int closed = fclose(f);
    return (n == sizeof(netconfig_t) && closed == 0) ? 0 : -1;
}

s32 NetConfigVerify(const netconfig_t *expected) {
    static netconfig_t actual;
    s32 rc = NetConfigLoad(&actual);
    if (rc != ISFS_OK) return rc;
    return memcmp(&actual, expected, sizeof(netconfig_t)) == 0 ? ISFS_OK : ISFS_EINVAL;
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
