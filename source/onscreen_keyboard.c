#include <stdio.h>
#include <string.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <wiikeyboard/keyboard.h>

#include "controller.h"
#include "onscreen_keyboard.h"

static const char *LowerRows[] = {
    "1234567890",
    "qwertyuiop",
    "asdfghjkl",
    "zxcvbnm.-_",
};
static const char *UpperRows[] = {
    "1234567890",
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM.-_",
};
#define ROW_COUNT 4

static void ClampCursor(int *row, int *col, bool shift) {
    const char **rows = shift ? UpperRows : LowerRows;
    if (*row < 0) *row = 0;
    if (*row >= ROW_COUNT) *row = ROW_COUNT - 1;
    int len = (int)strlen(rows[*row]);
    if (*col < 0) *col = 0;
    if (*col >= len) *col = len - 1;
}

static void DrawKeyboard(const char *prompt, const char *output, int row, int col, bool shift) {
    printf("\x1b[2J\x1b[1;1H");
    printf("%s\n\n> %s_\n\n", prompt, output);

    const char **rows = shift ? UpperRows : LowerRows;
    for (int r = 0; r < ROW_COUNT; ++r) {
        for (int c = 0; rows[r][c]; ++c) {
            if (r == row && c == col)
                printf("[%c]", rows[r][c]);
            else
                printf(" %c ", rows[r][c]);
        }
        printf("\n");
    }

    printf("\nD-Pad: move   A: type   B: backspace   2: space\n");
    printf("1: %s   +: confirm   HOME: cancel\n", shift ? "lowercase" : "uppercase/shift");
    printf("(A physical USB/BT keyboard also types directly.)\n");
}

bool OnScreenKeyboard(const char *prompt, char *output, size_t capacity) {
    output[0] = '\0';
    size_t len = 0;
    int row = 1, col = 0; // start on the letter rows
    bool shift = false;
    bool redraw = true;

    KEYBOARD_FlushEvents();

    while (1) {
        if (redraw) {
            DrawKeyboard(prompt, output, row, col, shift);
            redraw = false;
        }

        // Physical keyboard: type straight into the buffer alongside the
        // on-screen grid, so a connected keyboard is just faster, not a
        // separate mode.
        keyboard_event event;
        while (KEYBOARD_GetEvent(&event) > 0) {
            if (event.type != KEYBOARD_PRESSED) continue;

            if (event.symbol == KS_Return || event.symbol == KS_KP_Enter) {
                return len > 0;
            }
            if (event.symbol == KS_Escape) {
                return false;
            }
            if (event.symbol == KS_BackSpace) {
                if (len > 0) { output[--len] = '\0'; redraw = true; }
                continue;
            }
            if (event.symbol == KS_Tab) {
                shift = !shift;
                ClampCursor(&row, &col, shift);
                redraw = true;
                continue;
            }
            if (event.symbol == KS_Up) {
                row--;
                ClampCursor(&row, &col, shift);
                redraw = true;
                continue;
            }
            if (event.symbol == KS_Down) {
                row++;
                ClampCursor(&row, &col, shift);
                redraw = true;
                continue;
            }
            if (event.symbol == KS_Left) {
                col--;
                ClampCursor(&row, &col, shift);
                redraw = true;
                continue;
            }
            if (event.symbol == KS_Right) {
                col++;
                ClampCursor(&row, &col, shift);
                redraw = true;
                continue;
            }
            if (event.symbol >= 0x20 && event.symbol < 0x7F && len + 1 < capacity) {
                output[len++] = (char)event.symbol;
                output[len] = '\0';
                redraw = true;
            }
        }

        u32 pressed = Controller_Scan(CONTROLLER_BUTTONS_DOWN);

        if (pressed & WPAD_BUTTON_UP)    { row--; ClampCursor(&row, &col, shift); redraw = true; }
        if (pressed & WPAD_BUTTON_DOWN)  { row++; ClampCursor(&row, &col, shift); redraw = true; }
        if (pressed & WPAD_BUTTON_LEFT)  { col--; ClampCursor(&row, &col, shift); redraw = true; }
        if (pressed & WPAD_BUTTON_RIGHT) { col++; ClampCursor(&row, &col, shift); redraw = true; }

        if (pressed & WPAD_BUTTON_A) {
            const char **rows = shift ? UpperRows : LowerRows;
            if (len + 1 < capacity) {
                output[len++] = rows[row][col];
                output[len] = '\0';
            }
            redraw = true;
        }
        if (pressed & WPAD_BUTTON_B) {
            if (len > 0) { output[--len] = '\0'; }
            redraw = true;
        }
        if (pressed & WPAD_BUTTON_2) {
            if (len + 1 < capacity) { output[len++] = ' '; output[len] = '\0'; }
            redraw = true;
        }
        if (pressed & WPAD_BUTTON_1) {
            shift = !shift;
            ClampCursor(&row, &col, shift);
            redraw = true;
        }
        if (pressed & WPAD_BUTTON_PLUS) {
            return len > 0;
        }
        if (pressed & WPAD_BUTTON_HOME) {
            return false;
        }

        VIDEO_WaitVSync();
    }
}
