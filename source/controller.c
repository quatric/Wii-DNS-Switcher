// Ported from larsenv/GlowWii's DetectInput()/map_classic()/map_drc()/
// poll_ds3(). See scripts/build_controller_portlibs.sh for the optional
// WITH_WIIDRC / WITH_WUPC / WITH_SICKSAXIS portlibs this needs.

#include <ogcsys.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#ifdef HAVE_WIIDRC
#include <wiidrc/wiidrc.h>
#endif
#ifdef HAVE_WUPC
#include <wupc/wupc.h>
#endif
#ifdef HAVE_SICKSAXIS
#include <sicksaxis.h>
#endif

#include <wiikeyboard/keyboard.h>
#include <wiikeyboard/wsksymdef.h>

#include "controller.h"

static u32 kbd_held = 0;

#ifdef HAVE_SICKSAXIS
static struct ss_device ds3;
static int ds3_initialized;
static u32 ds3_previous;
#endif

static u32 MapClassic(u32 buttons) {
    u32 out = 0;
    if (buttons & WPAD_CLASSIC_BUTTON_ZR) out |= WPAD_BUTTON_PLUS;
    if (buttons & WPAD_CLASSIC_BUTTON_ZL) out |= WPAD_BUTTON_MINUS;
    if (buttons & WPAD_CLASSIC_BUTTON_PLUS)  out |= WPAD_BUTTON_PLUS;
    if (buttons & WPAD_CLASSIC_BUTTON_MINUS) out |= WPAD_BUTTON_MINUS;
    if (buttons & WPAD_CLASSIC_BUTTON_A)     out |= WPAD_BUTTON_A;
    if (buttons & WPAD_CLASSIC_BUTTON_B)     out |= WPAD_BUTTON_B;
    if (buttons & WPAD_CLASSIC_BUTTON_X)     out |= WPAD_BUTTON_2;
    if (buttons & WPAD_CLASSIC_BUTTON_Y)     out |= WPAD_BUTTON_1;
    if (buttons & WPAD_CLASSIC_BUTTON_HOME)  out |= WPAD_BUTTON_HOME;
    if (buttons & WPAD_CLASSIC_BUTTON_UP)    out |= WPAD_BUTTON_UP;
    if (buttons & WPAD_CLASSIC_BUTTON_DOWN)  out |= WPAD_BUTTON_DOWN;
    if (buttons & WPAD_CLASSIC_BUTTON_LEFT)  out |= WPAD_BUTTON_LEFT;
    if (buttons & WPAD_CLASSIC_BUTTON_RIGHT) out |= WPAD_BUTTON_RIGHT;
    return out;
}

#ifdef HAVE_WIIDRC
static u32 MapDrc(u32 buttons) {
    u32 out = 0;
    if (buttons & (WIIDRC_BUTTON_R | WIIDRC_BUTTON_ZR)) out |= WPAD_BUTTON_PLUS;
    if (buttons & (WIIDRC_BUTTON_L | WIIDRC_BUTTON_ZL)) out |= WPAD_BUTTON_MINUS;
    if (buttons & WIIDRC_BUTTON_PLUS)  out |= WPAD_BUTTON_PLUS;
    if (buttons & WIIDRC_BUTTON_MINUS) out |= WPAD_BUTTON_MINUS;
    if (buttons & WIIDRC_BUTTON_A)     out |= WPAD_BUTTON_A;
    if (buttons & WIIDRC_BUTTON_B)     out |= WPAD_BUTTON_B;
    if (buttons & WIIDRC_BUTTON_X)     out |= WPAD_BUTTON_2;
    if (buttons & WIIDRC_BUTTON_Y)     out |= WPAD_BUTTON_1;
    if (buttons & WIIDRC_BUTTON_HOME)  out |= WPAD_BUTTON_HOME;
    if (buttons & WIIDRC_BUTTON_UP)    out |= WPAD_BUTTON_UP;
    if (buttons & WIIDRC_BUTTON_DOWN)  out |= WPAD_BUTTON_DOWN;
    if (buttons & WIIDRC_BUTTON_LEFT)  out |= WPAD_BUTTON_LEFT;
    if (buttons & WIIDRC_BUTTON_RIGHT) out |= WPAD_BUTTON_RIGHT;
    return out;
}
#endif

#ifdef HAVE_SICKSAXIS
static u32 PollDs3(int down) {
    if (!ds3_initialized) return 0;
    if (!ss_is_connected(&ds3)) {
        ds3_previous = 0;
        if (ss_open(&ds3) > 0) ss_start_reading(&ds3);
        else return 0;
    }
    const struct SS_BUTTONS *b = &ds3.pad.buttons;
    u32 held = 0;
    if (b->start || b->R1 || b->R2) held |= WPAD_BUTTON_PLUS;
    if (b->select || b->L1 || b->L2) held |= WPAD_BUTTON_MINUS;
    if (b->cross) held |= WPAD_BUTTON_A;
    if (b->circle) held |= WPAD_BUTTON_B;
    if (b->triangle) held |= WPAD_BUTTON_2;
    if (b->square) held |= WPAD_BUTTON_1;
    if (b->PS) held |= WPAD_BUTTON_HOME;
    if (b->up) held |= WPAD_BUTTON_UP;
    if (b->down) held |= WPAD_BUTTON_DOWN;
    if (b->left) held |= WPAD_BUTTON_LEFT;
    if (b->right) held |= WPAD_BUTTON_RIGHT;
    u32 pressed = held & ~ds3_previous;
    ds3_previous = held;
    return down ? pressed : held;
}
#endif

static u32 MapKeyboardKey(const keyboard_event *event) {
    u32 button = 0;

    switch (event->symbol) {
    case KS_Up:
        button |= WPAD_BUTTON_UP;
        break;
    case KS_Down:
        button |= WPAD_BUTTON_DOWN;
        break;
    case KS_Left:
        button |= WPAD_BUTTON_LEFT;
        break;
    case KS_Right:
        button |= WPAD_BUTTON_RIGHT;
        break;
    case KS_Return:
    case KS_KP_Enter:
    case KS_space:
        button |= WPAD_BUTTON_A;
        break;
    case KS_BackSpace:
        button |= WPAD_BUTTON_B;
        break;
    case KS_Escape:
    case KS_Home:
        button |= WPAD_BUTTON_HOME;
        break;
    case KS_Tab:
    case KS_1:
    case KS_x:
    case KS_X:
        button |= WPAD_BUTTON_1;
        break;
    case KS_Delete:
    case KS_2:
    case KS_y:
    case KS_Y:
        button |= WPAD_BUTTON_2;
        break;
    case KS_plus:
    case KS_equal:
        button |= WPAD_BUTTON_PLUS;
        break;
    case KS_minus:
        button |= WPAD_BUTTON_MINUS;
        break;
    default:
        break;
    }

    if (!button) {
        switch (event->keycode) {
        case 0x52: // Up
            button |= WPAD_BUTTON_UP;
            break;
        case 0x51: // Down
            button |= WPAD_BUTTON_DOWN;
            break;
        case 0x50: // Left
            button |= WPAD_BUTTON_LEFT;
            break;
        case 0x4F: // Right
            button |= WPAD_BUTTON_RIGHT;
            break;
        case 0x28: // Enter
        case 0x58: // Keypad Enter
        case 0x2C: // Space
            button |= WPAD_BUTTON_A;
            break;
        case 0x2A: // Backspace
            button |= WPAD_BUTTON_B;
            break;
        case 0x29: // Escape
        case 0x4A: // Home
            button |= WPAD_BUTTON_HOME;
            break;
        case 0x2B: // Tab
        case 0x1E: // 1
        case 0x1B: // X
            button |= WPAD_BUTTON_1;
            break;
        case 0x4C: // Delete
        case 0x1F: // 2
        case 0x1C: // Y
            button |= WPAD_BUTTON_2;
            break;
        default:
            break;
        }
    }

    return button;
}

void Controller_Init(void) {
    PAD_Init();
    WPAD_Init();
#ifdef HAVE_WIIDRC
    WiiDRC_Init();
#endif
#ifdef HAVE_WUPC
    WUPC_Init();
#endif
#ifdef HAVE_SICKSAXIS
    if (ss_init() >= 0 && ss_initialize(&ds3) >= 0) {
        ds3_initialized = 1;
        if (ss_open(&ds3) > 0) ss_start_reading(&ds3);
    }
#endif
    KEYBOARD_Init(NULL);
}

void Controller_Shutdown(void) {
    KEYBOARD_Deinit();
#ifdef HAVE_SICKSAXIS
    if (ds3_initialized) {
        if (ss_is_connected(&ds3)) ss_stop_reading(&ds3);
        ss_close(&ds3);
    }
#endif
#ifdef HAVE_WUPC
    WUPC_Shutdown();
#endif
}

u32 Controller_Scan(u8 downOrHeld) {
    const int down = downOrHeld == CONTROLLER_BUTTONS_DOWN;
    u32 pressed = 0, buttons = 0, gcpressed = 0;

#ifdef HAVE_WUPC
    WUPC_UpdateButtonStats();
    for (int chan = 0; chan < 4; chan++)
        buttons |= down ? WUPC_ButtonsDown(chan) : WUPC_ButtonsHeld(chan);
    pressed |= MapClassic(buttons);
#endif

    buttons = 0;
    if (WPAD_ScanPads() > WPAD_ERR_NONE) {
        for (int chan = 0; chan < 4; chan++)
            buttons |= down ? WPAD_ButtonsDown(chan) : WPAD_ButtonsHeld(chan);
        pressed |= buttons | MapClassic(buttons);
    }

#ifdef HAVE_WIIDRC
    if (WiiDRC_Inited() && WiiDRC_Connected() && WiiDRC_ScanPads()) {
        buttons = down ? WiiDRC_ButtonsDown() : WiiDRC_ButtonsHeld();
        pressed |= MapDrc(buttons);
    }
#endif
#ifdef HAVE_SICKSAXIS
    pressed |= PollDs3(down);
#endif

    if (PAD_ScanPads() > PAD_ERR_NONE) {
        for (int chan = 0; chan < 4; chan++)
            gcpressed |= down ? PAD_ButtonsDown(chan) : PAD_ButtonsHeld(chan);
        if (gcpressed & PAD_TRIGGER_R) pressed |= WPAD_BUTTON_PLUS;
        if (gcpressed & PAD_TRIGGER_L) pressed |= WPAD_BUTTON_MINUS;
        if (gcpressed & PAD_BUTTON_A) pressed |= WPAD_BUTTON_A;
        if (gcpressed & PAD_BUTTON_B) pressed |= WPAD_BUTTON_B;
        if (gcpressed & PAD_BUTTON_X) pressed |= WPAD_BUTTON_2;
        if (gcpressed & PAD_BUTTON_Y) pressed |= WPAD_BUTTON_1;
        if (gcpressed & PAD_BUTTON_MENU) pressed |= WPAD_BUTTON_HOME;
        if (gcpressed & PAD_BUTTON_UP) pressed |= WPAD_BUTTON_UP;
        if (gcpressed & PAD_BUTTON_DOWN) pressed |= WPAD_BUTTON_DOWN;
        if (gcpressed & PAD_BUTTON_LEFT) pressed |= WPAD_BUTTON_LEFT;
        if (gcpressed & PAD_BUTTON_RIGHT) pressed |= WPAD_BUTTON_RIGHT;
    }

    keyboard_event kbevt;
    u32 kbd_down = 0;
    while (KEYBOARD_GetEvent(&kbevt) > 0) {
        u32 btn = MapKeyboardKey(&kbevt);
        if (kbevt.type == KEYBOARD_PRESSED) {
            kbd_down |= btn;
            kbd_held |= btn;
        } else if (kbevt.type == KEYBOARD_RELEASED) {
            kbd_held &= ~btn;
        }
    }
    pressed |= down ? kbd_down : kbd_held;

    return pressed;
}
