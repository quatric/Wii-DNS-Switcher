#pragma once

#include <gctypes.h>

// Unified controller and USB keyboard polling via libinput-wii. Wiimote
// (+ Nunchuk / Classic Controller), GameCube pad, USB keyboard and, when built
// with the matching WITH_* flag, Wii U Pro Controller / Wii U GamePad / PS3
// controller input all arrive as one WPAD_BUTTON_* bitmask so the rest of the
// app only has to handle one button set.

#define CONTROLLER_BUTTONS_DOWN 0
#define CONTROLLER_BUTTONS_HELD 1

void Controller_Init(void);
void Controller_Shutdown(void);

// Polls every supported controller and returns a WPAD_BUTTON_* mask. Pass
// CONTROLLER_BUTTONS_DOWN for edge-triggered "just pressed" (menu
// navigation) or CONTROLLER_BUTTONS_HELD for "currently held".
u32 Controller_Scan(u8 downOrHeld);
