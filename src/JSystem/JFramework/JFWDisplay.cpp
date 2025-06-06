//
// Generated by dtk
// Translation Unit: JFWDisplay.cpp
//

#include "JSystem/JFramework/JFWDisplay.h"
#include "JSystem/J2DGraph/J2DOrthoGraph.h"
#include "JSystem/JUtility/JUTAssert.h"
#include "JSystem/JUtility/JUTConsole.h"
#include "JSystem/JUtility/JUTDbPrint.h"
#include "JSystem/JUtility/JUTProcBar.h"
#include "dolphin/gx/GX.h"
#include "dolphin/os/OS.h"
#include "global.h"

JFWDisplay* JFWDisplay::sManager = NULL;

u8 JFWAutoAbortGfx = 1;
Mtx e_mtx = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
};
static GXTexObj clear_z_tobj;
u8 clear_z_TX[64] ALIGN_DECL(32) = {
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};


/* 802551C0-8025527C       .text ctor_subroutine__10JFWDisplayFPC16_GXRenderModeObjb */
void JFWDisplay::ctor_subroutine(const GXRenderModeObj* mode, bool enableAlpha) {
    mEnableAlpha = enableAlpha;
    mClamp = 3;

    mClearColor.r = 0;
    mClearColor.g = 0;
    mClearColor.b = 0;
    mClearColor.a = 0;

    mZClear = 0xFFFFFF;

    mpRenderMode = mode != 0 ? mode : JUTVideo::getManager()->getRenderMode();

    mGamma = 0;
    mpFader = NULL;
    mFrameRate = 1;
    mTickRate = 0;
    mCombinationRatio = 0.0f;
    field_0x34 = 0;
    field_0x30 = OSGetTick();
    field_0x38 = 0;
    field_0x3C = 0;
    field_0x3E = 0;
    mDrawDoneMethod = Sync;
    clearEfb_init();
    JUTProcBar::create();
    JUTProcBar::clear();
}

/* 8025527C-802552EC       .text __ct__10JFWDisplayFPC16_GXRenderModeObjP7JKRHeapQ26JUTXfb10EXfbNumberb */
JFWDisplay::JFWDisplay(const GXRenderModeObj* mode, JKRHeap* p_heap, JUTXfb::EXfbNumber xfb_num, bool enableAlpha) {
    ctor_subroutine(mode, enableAlpha);
    mXfbManager = JUTXfb::createManager(mode, p_heap, xfb_num);
}

/* 802552EC-80255354       .text __dt__10JFWDisplayFv */
JFWDisplay::~JFWDisplay() {
    waitBlanking(2);
    JUTProcBar::destroy();
    JUTXfb::destroyManager();
}

/* 80255354-802553EC       .text createManager__10JFWDisplayFP7JKRHeapQ26JUTXfb10EXfbNumberb */
JFWDisplay* JFWDisplay::createManager(JKRHeap* p_heap, JUTXfb::EXfbNumber xfb_num, bool enableAlpha) {
    JUT_CONFIRM(VERSION_SELECT(244, 244, 243, 243), sManager == NULL);
    if(sManager == 0) {
        sManager = new JFWDisplay(0, p_heap, xfb_num, enableAlpha);
    }

    return sManager;
}

static void dummyFunc() {
    OSReport("sManager");
    OSReport("mTemporarySingle");
}

/* 802553EC-80255444       .text callDirectDraw__Fv */
void callDirectDraw() {
    JUTChangeFrameBuffer(JUTXfb::getManager()->getDrawingXfb(), JUTVideo::getManager()->getEfbHeight(), JUTVideo::getManager()->getFbWidth());
    JUTAssertion::flushMessage();
}

/* 80255444-80255528       .text prepareCopyDisp__10JFWDisplayFv */
void JFWDisplay::prepareCopyDisp() {
    GXRenderModeObj* renderObj = JUTVideo::getManager()->getRenderMode();
    u16 width, height;
    JUTVideo::getManager()->getBounds(width, height);
    u16 xfb_height = JUTVideo::getManager()->getXfbHeight();
#if VERSION > VERSION_JPN
    f32 y_scaleF = GXGetYScaleFactor(height, xfb_height);
#endif

    GXSetCopyClear(mClearColor, mZClear);
    GXSetDispCopySrc(0, 0, width, height);
    GXSetDispCopyDst(width, xfb_height);
#if VERSION <= VERSION_JPN
    GXSetDispCopyYScale(xfb_height / (f32)height);
#else
    GXSetDispCopyYScale(y_scaleF);
#endif
    VIFlush();
    GXSetCopyFilter((GXBool)renderObj->antialiasing, renderObj->sample_pattern, GX_ENABLE, renderObj->vfilter);
    GXSetCopyClamp((GXFBClamp)mClamp);
    GXSetDispCopyGamma((GXGamma)mGamma);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    if (mEnableAlpha) {
        GXSetAlphaUpdate(GX_ENABLE);
    }
}

/* 80255528-80255570       .text drawendXfb_single__10JFWDisplayFv */
void JFWDisplay::drawendXfb_single() {
    JUTXfb* manager = JUTXfb::getManager();
    if (manager->getDrawingXfbIndex() >= 0) {
        prepareCopyDisp();
        JFWGXDrawDoneAutoAbort();
        GXFlush();
        manager->setDrawnXfbIndex(manager->getDrawingXfbIndex());
    }
}

/* 80255570-80255658       .text exchangeXfb_double__10JFWDisplayFv */
void JFWDisplay::exchangeXfb_double() {
    JUTXfb* xfbMng = JUTXfb::getManager();

    if(xfbMng->getDrawnXfbIndex() == xfbMng->getDisplayingXfbIndex()) {
        if(0 <= xfbMng->getDrawingXfbIndex()) {
            prepareCopyDisp();
            GXCopyDisp(xfbMng->getDrawingXfb(), (GXBool)1);
            if (mDrawDoneMethod == Sync) {
                GXDrawDone();
                JUTVideo::dummyNoDrawWait();
            } else {
                JUTVideo::drawDoneStart();
            }

            if (mDrawDoneMethod == Sync) {
                callDirectDraw();
            }
        }

        s16 idx = xfbMng->getDrawingXfbIndex();
        xfbMng->setDrawnXfbIndex(idx);
        xfbMng->setDrawingXfbIndex(idx >= 0 ? idx ^ 1 : 0);
    }
    else {
        clearEfb(mClearColor);
        if(xfbMng->getDrawingXfbIndex() < 0) {
            xfbMng->setDrawingXfbIndex(0);
        }
    }
}

/* 80255658-802556D0       .text exchangeXfb_triple__10JFWDisplayFv */
void JFWDisplay::exchangeXfb_triple() {
    JUTXfb* xfbMng = JUTXfb::getManager();

    if (xfbMng->getDrawingXfbIndex() >= 0) {
        callDirectDraw();
    }

    xfbMng->setDrawnXfbIndex(xfbMng->getDrawingXfbIndex());

    s16 drawing_idx = xfbMng->getDrawingXfbIndex() + 1;
    do {
        if (drawing_idx >= 3 || drawing_idx < 0) {
            drawing_idx = 0;
        }
    } while (drawing_idx == xfbMng->getDisplayingXfbIndex());
    xfbMng->setDrawingXfbIndex(drawing_idx);
}

/* 802556D0-80255730       .text copyXfb_triple__10JFWDisplayFv */
void JFWDisplay::copyXfb_triple() {
    JUTXfb* xfbMng = JUTXfb::getManager();

    if (xfbMng->getDrawingXfbIndex() >= 0) {
        prepareCopyDisp();
        GXCopyDisp(xfbMng->getDrawingXfb(), GX_TRUE);
        GXPixModeSync();
    }
}

/* 80255730-802557C0       .text preGX__10JFWDisplayFv */
void JFWDisplay::preGX() {
    GXInvalidateTexAll();
    GXInvalidateVtxCache();

    if (mpRenderMode->antialiasing) {
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
        GXSetDither(GX_ENABLE);
    } else {
        if (mEnableAlpha) {
            GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
            GXSetDither(GX_ENABLE);
        } else {
            GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
            GXSetDither(GX_DISABLE);
        }
    }
}

/* 802557C0-802558CC       .text endGX__10JFWDisplayFv */
void JFWDisplay::endGX() {
    u16 width, height;
    JUTVideo::getManager()->getBounds(width, height);

    J2DOrthoGraph ortho(0.0f, 0.0f, width, height, -1.0f, 1.0f);

    if (mpFader != NULL) {
        ortho.setPort();
        mpFader->control();
    }
    ortho.setPort();
    JUTDbPrint::getManager()->flush();

    if (JUTConsoleManager::getManager() != NULL) {
        ortho.setPort();
        JUTConsoleManager::getManager()->draw();
    }

    ortho.setPort();
    JUTProcBar::getManager()->draw();

    if (mDrawDoneMethod != Sync || JUTXfb::getManager()->getBufferNum() == 1) {
        JUTAssertion::flushMessage_dbPrint();
    }
    GXFlush();
}

/* 802558CC-80255AB8       .text beginRender__10JFWDisplayFv */
void JFWDisplay::beginRender() {
    JUTProcBar::getManager()->wholeLoopEnd();
    JUTProcBar::getManager()->wholeLoopStart();
    JUTProcBar::getManager()->idleStart();

    waitForTick(mTickRate, mFrameRate);
    JUTVideo::getManager()->waitRetraceIfNeed();

    OSTick tick = OSGetTick();
    field_0x34 = tick - field_0x30;
    field_0x30 = tick;
    field_0x38 = field_0x30 - JUTVideo::getVideoLastTick();
    
    JUTProcBar::getManager()->idleEnd();

    if (true) {
        JUTProcBar::getManager()->gpStart();

        JUTXfb* xfbMng = JUTXfb::getManager();
        switch (xfbMng->getBufferNum()) {
        case 1:
            if (xfbMng->getSDrawingFlag() != 2) {
                xfbMng->setSDrawingFlag(1);
                clearEfb(mClearColor);
            } else {
                xfbMng->setSDrawingFlag(1);
            }
            xfbMng->setDrawingXfbIndex(field_0x3C);
            break;
        case 2:
            exchangeXfb_double();
            break;
        case 3:
            exchangeXfb_triple();
            break;
        default:
            break;
        }
    }

    preGX();
}

/* 80255AB8-80255B58       .text endRender__10JFWDisplayFv */
void JFWDisplay::endRender() {
    endGX();

    switch (JUTXfb::getManager()->getBufferNum()) {
    case 1:
        drawendXfb_single();
    case 2:
        break;
    case 3:
        copyXfb_triple();
    default:
        break;
    }

    JUTProcBar::getManager()->cpuStart();
    calcCombinationRatio();
}

/* 80255B58-80255CE4       .text endFrame__10JFWDisplayFv */
void JFWDisplay::endFrame() {
    JUTProcBar::getManager()->cpuEnd();

    JUTProcBar::getManager()->gpWaitStart();
    switch (JUTXfb::getManager()->getBufferNum()) {
    case 1:
        break;
    case 2:
        JFWGXDrawDoneAutoAbort();
        GXFlush();
        break;
    case 3:
        JFWGXDrawDoneAutoAbort();
        GXFlush();
        break;
    default:
        break;
    }

    JUTProcBar::getManager()->gpWaitEnd();
    JUTProcBar::getManager()->gpEnd();

    static u32 prevFrame = VIGetRetraceCount();
    u32 retrace_cnt = VIGetRetraceCount();
    JUTProcBar::getManager()->setCostFrame(retrace_cnt - prevFrame);
    prevFrame = retrace_cnt;
}

/* 80255CE4-80255D34       .text waitBlanking__10JFWDisplayFi */
void JFWDisplay::waitBlanking(int duration) {
    while (duration-- > 0) {
        waitForTick(mTickRate, mFrameRate);
    }
}

/* 80255D34-80255E54       .text waitForTick__FUlUs */
void waitForTick(u32 p1, u16 p2) {
    if (p1 != 0) {
        static s64 nextTick = OSGetTime();
        s64 time = OSGetTime();
        while (time < nextTick) {
            JFWDisplay::getManager()->threadSleep((nextTick - time));
            time = OSGetTime();
        }
        nextTick = time + p1;
    }
    else {
        static u32 nextCount = VIGetRetraceCount();
        u32 uVar1 = (p2 == 0) ? 1 : p2;
        OSMessage msg;
        do {
            if (!OSReceiveMessage(JUTVideo::getManager()->getMessageQueue(), &msg, OS_MESSAGE_BLOCK)) {
                msg = 0;
            }
        } while (((int)msg - (int)nextCount) < 0);
        nextCount = (int)msg + uVar1;
    }
}

/* 80255E54-80255E78       .text JFWThreadAlarmHandler__FP7OSAlarmP9OSContext */
void JFWThreadAlarmHandler(OSAlarm* p_alarm, OSContext* p_ctx) {
    JFWAlarm* alarm = static_cast<JFWAlarm*>(p_alarm);
    OSResumeThread(alarm->getThread());
}

/* 80255E78-80255EEC       .text threadSleep__10JFWDisplayFx */
void JFWDisplay::threadSleep(s64 time) {
    JFWAlarm alarm;
    OSCreateAlarm(&alarm);
    alarm.setThread(OSGetCurrentThread());
    s32 status = OSDisableInterrupts();
    OSSetAlarm(&alarm, time, JFWThreadAlarmHandler);
    OSSuspendThread(alarm.getThread());
    OSRestoreInterrupts(status);
}

/* 80255EEC-80255F60       .text clearEfb_init__10JFWDisplayFv */
void JFWDisplay::clearEfb_init() {
    GXInitTexObj(&clear_z_tobj, &clear_z_TX, 4, 4, GX_TF_Z24X8, GX_REPEAT,GX_REPEAT, false);
    GXInitTexObjLOD(&clear_z_tobj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
}

/* 80255F60-80255FA0       .text clearEfb__10JFWDisplayF8_GXColor */
void JFWDisplay::clearEfb(GXColor color) {
    int height = mpRenderMode->efb_height;
    int width = mpRenderMode->fb_width;

    clearEfb(0, 0, width, height, color);
}

/* 80255FA0-8025631C       .text clearEfb__10JFWDisplayFiiii8_GXColor */
void JFWDisplay::clearEfb(int param_0, int param_1, int param_2, int param_3, GXColor color) {
    Mtx44 mtx;
    u16 height = mpRenderMode->efb_height;
    u16 width = mpRenderMode->fb_width;

    C_MTXOrtho(mtx, 0.0f, height, 0.0f, width, 0.0f, 1.0f);
    GXSetProjection(mtx, GX_ORTHOGRAPHIC);
    GXSetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
    GXSetScissor(0, 0, width, height);

    GXLoadPosMtxImm(e_mtx, GX_PNMTX0);
    GXSetCurrentMtx(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGB, GX_RGBX8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_CLR_RGBA, GX_RGB565, 0);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60);
    GXLoadTexObj(&clear_z_tobj, GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
    GXSetZCompLoc(GX_DISABLE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);

    if (mEnableAlpha) {
        GXSetAlphaUpdate(GX_ENABLE);
        GXSetDstAlpha(GX_ENABLE, 0x00);
    }
    GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_ENABLE);
    GXSetCullMode(GX_CULL_BACK);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2u16(param_0, param_1);
    GXTexCoord2u8(0, 0);

    GXPosition2u16(param_0 + param_2, param_1);
    GXTexCoord2u8(1, 0);

    GXPosition2u16(param_0 + param_2, param_1 + param_3);
    GXTexCoord2u8(1, 1);

    GXPosition2u16(param_0, param_1 + param_3);
    GXTexCoord2u8(0, 1);

    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GXSetZCompLoc(GX_ENABLE);
    if (mEnableAlpha) {
        GXSetDstAlpha(GX_DISABLE, 0x00);
    }
}

/* 8025631C-802563A8       .text calcCombinationRatio__10JFWDisplayFv */
void JFWDisplay::calcCombinationRatio() {
    u32 vidInterval = JUTVideo::getVideoInterval();
    s32 unk34 = field_0x34 * 2;

    s32 i = vidInterval;
    for (; i < unk34; i += vidInterval) {
    }

    s32 tmp = (i - unk34) - field_0x38;
    if (tmp < 0) {
        tmp += vidInterval;
    }
    mCombinationRatio = (f32)tmp / (f32)field_0x34;
    if (mCombinationRatio > 1.0f) {
        mCombinationRatio = 1.0f;
    }
}

/* 802563A8-8025640C       .text JFWGXDrawDoneAutoAbort__Fv */
void JFWGXDrawDoneAutoAbort() {
    if(JFWAutoAbortGfx != 0) {
        OSAlarm alarm;
        OSCreateAlarm(&alarm);
        OSSetAlarm(&alarm, OS_TIMER_CLOCK, JFWGXAbortAlarmHandler);
        GXDrawDone();
        OSCancelAlarm(&alarm);
    }
    else {
        GXDrawDone();
    }
}

/* 8025640C-802564D4       .text JFWGXAbortAlarmHandler__FP7OSAlarmP9OSContext */
void JFWGXAbortAlarmHandler(OSAlarm*, OSContext*) {
    diagnoseGpHang();
    if(JFWAutoAbortGfx != 1) {
        OSReport("自動復帰しません\n");
        JUT_WARN(VERSION_SELECT(1351, 1351, 1350, 1350), "GP FREEZE!");
        JUT_ASSERT(VERSION_SELECT(1352, 1352, 1351, 1351), 0);
    }
    else {
        OSReport("GXAbortFrame() を呼び出し、復帰します\n");
        JUT_WARN(VERSION_SELECT(1356, 1356, 1355, 1355), "GP FREEZE! AUTO RESUME");
        GXAbortFrame();
        GXSetDrawDone();
    }
}

/* 802564D4-802566B8       .text diagnoseGpHang__Fv */
void diagnoseGpHang() {
    u32 sp28;
    u32 sp24;
    u32 sp20;
    u32 sp1C;
    u32 sp18;
    u32 sp14;
    u32 sp10;
    u32 spC;
    bool readIdle;
    bool commandIdle;
    bool sp8;

    GXReadXfRasMetric(&sp24, &sp28, &sp1C, &sp20);
    GXReadXfRasMetric(&sp14, &sp18, &spC, &sp10);

    u32 temp_r31 = sp28 == sp18;
    u32 temp_r30 = sp24 == sp14;
    u32 temp_r0 = sp20 != sp10;
    u32 temp_r0_2 = sp1C != spC;

    GXGetGPStatus((GXBool*)&sp8, (GXBool*)&sp8, (GXBool*)&readIdle, (GXBool*)&commandIdle,
                  (GXBool*)&sp8);

    OSReport("GP status %d%d%d%d%d%d --> ", readIdle, commandIdle, temp_r31, temp_r30, temp_r0,
             temp_r0_2);

    if (!temp_r30 && temp_r0) {
        OSReport("GP hang due to XF stall bug.\n");
    } else if (!temp_r31 && temp_r30 && temp_r0) {
        OSReport("GP hang due to unterminated primitive.\n");
    } else if (!commandIdle && temp_r31 && temp_r30 && temp_r0) {
        OSReport("GP hang due to illegal instruction.\n");
    } else if (readIdle && commandIdle && temp_r31 && temp_r30 && temp_r0 && temp_r0_2) {
        OSReport("GP appears to be not hung (waiting for input).\n");
    } else {
        OSReport("GP is in unknown state.\n");
    }
}
