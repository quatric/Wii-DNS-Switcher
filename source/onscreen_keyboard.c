// The Wii system keyboard look-alike from libwiiosk. The app draws a text
// console, so this takes the screen over for one edit (as OSK_Prompt does),
// then puts the console back. Input comes from libwiiinput, which already
// owns the USB keyboard, so the pointer, mouse and keyboard all work.

#include <malloc.h>
#include <string.h>

#include <gccore.h>
#include <ogc/conf.h>
#include <input_wii.h>
#include <wiiosk.h>
#include <wiiosk_input_wii.h>

#include "onscreen_keyboard.h"

#define FIFO_SIZE (256 * 1024)

static GXRModeObj *vmode;
static void *vxfb;
static void *gp_fifo;

void OnScreenKeyboard_SetVideo(GXRModeObj *rmode, void *xfb) {
    vmode = rmode;
    vxfb = xfb;
}

static void SetupVideo(GXRModeObj *rmode) {
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    u32 xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, (rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE);
    GX_SetPixelFmt(rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetCopyClear((GXColor){ 0x20, 0x24, 0x2C, 0xFF }, GX_MAX_Z24);
}

bool OnScreenKeyboard(const char *prompt, char *output, size_t capacity) {
    output[0] = '\0';
    if (!vmode || !vxfb || capacity < 2) return false;

    u32 fbSize = VIDEO_GetFrameBufferSize(vmode);
    void *saved = malloc(fbSize);
    void *spare = memalign(32, fbSize);
    if (!saved || !spare) {
        free(saved);
        free(spare);
        return false;
    }
    memcpy(saved, vxfb, fbSize);
    DCInvalidateRange(spare, fbSize);
    void *fbs[2] = { vxfb, MEM_K0_TO_K1(spare) };

    osk_config cfg;
    OSK_DefaultConfig(&cfg);
    cfg.title = prompt;
    cfg.max_chars = (int)capacity - 1 < OSK_MAX_CHARS ? (int)capacity - 1 : OSK_MAX_CHARS;
    cfg.layout = OSK_LAYOUT_QWERTY;
    cfg.screen_width = vmode->fbWidth;
    cfg.screen_height = vmode->efbHeight;
    cfg.flags = OSK_FLAG_GX_READY | OSK_FLAG_NO_ENTER | OSK_FLAG_REQUIRE_TEXT;
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) cfg.flags |= OSK_FLAG_WIDESCREEN;
    osk_keyboard *osk = OSK_Create(&cfg);
    if (!osk) {
        free(saved);
        free(spare);
        return false;
    }

    if (!gp_fifo) {
        gp_fifo = memalign(32, FIFO_SIZE);
        memset(gp_fifo, 0, FIFO_SIZE);
        GX_Init(gp_fifo, FIFO_SIZE);
    }
    SetupVideo(vmode);
    INPUT_SetScreenSize(vmode->fbWidth, vmode->efbHeight);
    GX_CopyDisp(fbs[1], GX_TRUE);
    GX_DrawDone();

    osk_status st;
    int cur = 1;
    for (;;) {
        INPUT_Update();
        osk_input in;
        OSK_ReadInputWii(&in);
        st = OSK_Update(osk, &in);

        GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
        OSK_Draw(osk);
        GX_DrawDone();
        GX_SetColorUpdate(GX_TRUE);
        GX_CopyDisp(fbs[cur], GX_TRUE);
        GX_Flush();
        VIDEO_SetNextFramebuffer(fbs[cur]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        cur ^= 1;
        if (st != OSK_STATUS_EDITING) break;
    }

    bool ok = st == OSK_STATUS_DONE;
    if (ok) OSK_CopyText(osk, output, capacity);
    OSK_Destroy(osk);

    memcpy(vxfb, saved, fbSize);
    DCFlushRange(vxfb, fbSize);
    VIDEO_SetNextFramebuffer(vxfb);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    VIDEO_WaitVSync();
    free(saved);
    free(spare);
    return ok && output[0] != '\0';
}
