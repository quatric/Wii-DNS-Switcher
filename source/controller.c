// Controller and keyboard input via libinput-wii. See
// scripts/build_controller_portlibs.sh for the optional WITH_WIIDRC /
// WITH_WUPC / WITH_SICKSAXIS portlibs.

#include <ogcsys.h>
#include <wiiuse/wpad.h>
#include <input_wii.h>

#include "controller.h"

void Controller_Init(void) {
    // Buttons only: the switcher has no pointer and never used a mouse.
    INPUT_InitEx(INPUT_INIT_NO_POINTER | INPUT_INIT_NO_MOUSE);

    // Keyboard-as-controller additions on top of the library defaults
    // (arrows, Enter = A, Backspace = B, X = 1, Y = 2, Esc/Home = HOME).
    INPUT_KeyboardMapSet(0x2C, WPAD_BUTTON_A);     // Space
    INPUT_KeyboardMapSet(0x2B, WPAD_BUTTON_1);     // Tab
    INPUT_KeyboardMapSet(0x1E, WPAD_BUTTON_1);     // 1
    INPUT_KeyboardMapSet(0x4C, WPAD_BUTTON_2);     // Delete
    INPUT_KeyboardMapSet(0x1F, WPAD_BUTTON_2);     // 2
    INPUT_KeyboardMapSet(0x2E, WPAD_BUTTON_PLUS);  // = / +
    INPUT_KeyboardMapSet(0x2D, WPAD_BUTTON_MINUS); // -
}

void Controller_Shutdown(void) {
    INPUT_Shutdown();
}

u32 Controller_Scan(u8 downOrHeld) {
    INPUT_Update();
    return downOrHeld == CONTROLLER_BUTTONS_DOWN ? INPUT_ButtonsDown() : INPUT_ButtonsHeld();
}
