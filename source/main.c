#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/isfs.h>
#include <fat.h>

#include "controller.h"
#include "netconfig.h"
#include "onscreen_keyboard.h"

#define PROFILE_DIR  "sd:/wii/dns-switcher"
#define PROFILE_PATH "sd:/wii/dns-switcher/profiles.txt"
#define SETTINGS_PATH "sd:/wii/dns-switcher/settings.txt"
#define MAX_PROFILES 12

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

typedef struct {
    char name[33];
    char dns[16];
} DnsProfile;

static DnsProfile profiles[MAX_PROFILES];
static int profileCount = 0;

static const char *TargetName(int target) {
    static const char *names[] = {
        "All configured Wi-Fi slots",
        "Connection slot 1",
        "Connection slot 2",
        "Connection slot 3"
    };
    if (target < 0 || target > 3) target = 0;
    return names[target];
}

static int LoadTarget(void) {
    FILE *f = fopen(SETTINGS_PATH, "r");
    if (!f) return 0;
    int target = 0;
    int ok = fscanf(f, "target=%d", &target) == 1 && target >= 0 && target <= 3;
    fclose(f);
    return ok ? target : 0;
}

static void SaveTarget(int target) {
    mkdir("sd:/wii", 0777);
    mkdir(PROFILE_DIR, 0777);
    FILE *f = fopen(SETTINGS_PATH, "w");
    if (!f) return;
    fprintf(f, "target=%d\n", target);
    fclose(f);
}

static bool ParseIpv4(const char *text, u8 out[4]) {
    unsigned v[4];
    char tail;
    if (sscanf(text, "%u.%u.%u.%u%c", &v[0], &v[1], &v[2], &v[3], &tail) != 4)
        return false;
    for (int i = 0; i < 4; ++i) {
        if (v[i] > 255) return false;
        out[i] = (u8)v[i];
    }
    return true;
}

static void SanitizeName(char *name) {
    for (; *name; ++name)
        if (*name == '\t' || *name == '\r' || *name == '\n') *name = ' ';
}

static void LoadProfiles(void) {
    profileCount = 0;
    FILE *f = fopen(PROFILE_PATH, "r");
    if (!f) return;

    char line[96];
    while (profileCount < MAX_PROFILES && fgets(line, sizeof(line), f)) {
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        *tab++ = '\0';
        tab[strcspn(tab, "\r\n")] = '\0';
        u8 parsed[4];
        if (line[0] && ParseIpv4(tab, parsed)) {
            snprintf(profiles[profileCount].name, sizeof(profiles[profileCount].name), "%.32s", line);
            snprintf(profiles[profileCount].dns, sizeof(profiles[profileCount].dns), "%.15s", tab);
            profileCount++;
        }
    }
    fclose(f);
}

static bool SaveProfiles(void) {
    mkdir("sd:/wii", 0777);
    mkdir(PROFILE_DIR, 0777);
    FILE *f = fopen(PROFILE_PATH, "w");
    if (!f) return false;
    for (int i = 0; i < profileCount; ++i)
        fprintf(f, "%s\t%s\n", profiles[i].name, profiles[i].dns);
    bool ok = fclose(f) == 0;
    return ok;
}

static void PresentFrame(void) {
    VIDEO_WaitVSync();
}

static void ClearConsole(void) {
    printf("\x1b[2J");
    printf("\x1b[1;1H");
}

static bool AddProfile(void) {
    if (profileCount >= MAX_PROFILES) return false;
    char name[33] = {0};
    char dns[16] = {0};
    u8 parsed[4];

    if (!OnScreenKeyboard("Profile name:", name, sizeof(name))) return false;
    if (!OnScreenKeyboard("Primary DNS (e.g. 1.1.1.1):", dns, sizeof(dns))) return false;
    if (!ParseIpv4(dns, parsed)) {
        ClearConsole();
        printf("Invalid IPv4 address.\n\nPress A to return.\n");
        while (1) {
            if (Controller_Scan(CONTROLLER_BUTTONS_DOWN) & WPAD_BUTTON_A) break;
            PresentFrame();
        }
        return false;
    }

    SanitizeName(name);
    snprintf(profiles[profileCount].name, sizeof(profiles[profileCount].name), "%s", name);
    snprintf(profiles[profileCount].dns, sizeof(profiles[profileCount].dns), "%s", dns);
    profileCount++;
    if (!SaveProfiles()) {
        profileCount--;
        return false;
    }
    return true;
}

// Returns the chosen profile index, or -1 to exit without applying.
static int ChooseProfile(int *target) {
    int selected = 0;
    bool redraw = true;

    while (1) {
        int addIndex = profileCount;
        if (redraw) {
            ClearConsole();
            printf("Wii DNS Switcher\n\nSaved DNS profiles:\n\n");
            for (int i = 0; i < profileCount; ++i)
                printf("%c %s  [%s]\n", i == selected ? '>' : ' ', profiles[i].name, profiles[i].dns);
            printf("%c + Add DNS profile\n", selected == addIndex ? '>' : ' ');
            printf("\nTarget: %s\n", TargetName(*target));
            printf("A: select   1: change target\n");
            printf("2: delete profile          HOME: exit\n");
            redraw = false;
        }

        u32 down = Controller_Scan(CONTROLLER_BUTTONS_DOWN);

        if ((down & WPAD_BUTTON_UP) && selected > 0) { selected--; redraw = true; }
        if ((down & WPAD_BUTTON_DOWN) && selected < addIndex) { selected++; redraw = true; }
        if (down & WPAD_BUTTON_A) {
            if (selected < profileCount) return selected;
            if (AddProfile()) selected = profileCount - 1;
            redraw = true;
        }
        if (down & WPAD_BUTTON_1) {
            *target = (*target + 1) % 4;
            SaveTarget(*target);
            redraw = true;
        }
        if ((down & WPAD_BUTTON_2) && selected < profileCount) {
            for (int i = selected; i < profileCount - 1; ++i)
                profiles[i] = profiles[i + 1];
            profileCount--;
            SaveProfiles();
            if (selected > profileCount) selected = profileCount;
            redraw = true;
        }
        if (down & WPAD_BUTTON_HOME) return -1;

        PresentFrame();
    }
}

static s32 ApplyProfile(const DnsProfile *profile, int target, int *changed) {
    u8 primary[4];
    if (!ParseIpv4(profile->dns, primary)) return ISFS_EINVAL;
    static const u8 secondary[4] = {1, 1, 1, 1}; // always 1.1.1.1, matching the 3DS switcher

    netconfig_t config;
    s32 rc = NetConfigLoad(&config);
    if (rc != ISFS_OK) return rc;

    *changed = 0;
    int first = target == 0 ? 0 : target - 1;
    int last = target == 0 ? WIFI_SLOT_COUNT - 1 : target - 1;
    for (int slot = first; slot <= last; ++slot) {
        if (!NetConfigSlotInUse(&config, slot)) continue;
        NetConfigSetDns(&config, slot, primary, secondary);
        (*changed)++;
    }

    if (*changed == 0) return ISFS_EINVAL;
    return NetConfigSave(&config);
}

int main(void) {
    VIDEO_Init();
    Controller_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    fatInitDefault();

    s32 isfsRc = ISFS_Initialize();
    if (isfsRc != ISFS_OK) {
        printf("Could not initialize ISFS: %d\n\n", (int)isfsRc);
        printf("This tool needs NAND access via ISFS; run it from the Homebrew\n");
        printf("Channel or another loader that grants it.\n");
        goto done;
    }

    LoadProfiles();
    int target = LoadTarget();

    while (1) {
        int profileIndex = ChooseProfile(&target);
        if (profileIndex < 0) break;

        int changed = 0;
        s32 rc = ApplyProfile(&profiles[profileIndex], target, &changed);
        ClearConsole();
        if (rc == ISFS_OK) {
            printf("Applied %s to %d Wi-Fi slot%s.\n", profiles[profileIndex].name, changed,
                   changed == 1 ? "" : "s");
            PresentFrame();
            usleep(700000);
            break;
        }

        printf("DNS write failed: %d\n\nPress A to return.\n", (int)rc);
        while (1) {
            if (Controller_Scan(CONTROLLER_BUTTONS_DOWN) & WPAD_BUTTON_A) break;
            PresentFrame();
        }
    }

    ISFS_Deinitialize();

done:
    Controller_Shutdown();
    return 0;
}
