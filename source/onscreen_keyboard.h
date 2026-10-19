#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <gccore.h>

// Tell the keyboard which video mode and framebuffer the app's console uses.
// Call once after the video is set up.
void OnScreenKeyboard_SetVideo(GXRModeObj *rmode, void *xfb);

// The Wii-style on-screen keyboard from libwiiosk. It takes the screen over
// for the length of one edit, then restores the console. It is driven by
// Controller_Scan()'s libwiiinput devices: Wii Remote pointer and buttons,
// Classic Controller, GameCube pad, USB mouse and a USB keyboard that types
// directly into the same text box.
//
// Returns true if the user confirmed with a non-empty result, false if they
// cancelled or confirmed with nothing typed.
bool OnScreenKeyboard(const char *prompt, char *output, size_t capacity);
