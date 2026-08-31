#pragma once

#include <stddef.h>
#include <stdbool.h>

// On-screen QWERTY grid, navigable with any controller Controller_Scan()
// understands (Wiimote, Classic Controller, GameCube pad, and whatever
// optional pads the build enables). A connected keyboard (wiikeyboard's
// KEYBOARD_* symbol stream) can type directly into the same buffer at any
// time, so a physical keyboard still "just works" alongside it.
//
// Returns true if the user confirmed with a non-empty result, false if they
// cancelled (HOME) or confirmed with nothing typed.
bool OnScreenKeyboard(const char *prompt, char *output, size_t capacity);
