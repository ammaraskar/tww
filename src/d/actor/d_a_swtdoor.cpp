/**
 * d_a_swtdoor.cpp
 * Object - Forsaken Fortress tower (Helmaroc King fight) - gray "doors" blocking windows
 */

#include "d/actor/d_a_swtdoor.h"
#include "d/res/res_swtdoor.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_com_inf_game.h"
#include "d/d_procname.h"
#include "d/d_priority.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "SSystem/SComponent/c_lib.h"

/* 00000078-000000C4       .text daSwtdoor_Draw__FP13swtdoor_class */
static BOOL daSwtdoor_Draw(swtdoor_class* i_this) {
    J3DModel * model = i_this->model;
    g_env_light.setLightTevColorType(model, &i_this->tevStr);
    mDoExt_modelUpdateDL(model);
    return TRUE;
}

/* 000000C4-000001BC       .text daSwtdoor_Execute__FP13swtdoor_class */
static BOOL daSwtdoor_Execute(swtdoor_class* i_this) {
    g_env_light.settingTevStruct(TEV_TYPE_BG0, &i_this->current.pos, &i_this->tevStr);
    if (dComIfGs_isSwitch(i_this->mSwitchNo, fopAcM_GetRoomNo(i_this)) && i_this->home.pos.y > -300.0f)
        i_this->home.pos.y -= 10.0f;

    MtxTrans(i_this->current.pos.x, i_this->current.pos.y + i_this->home.pos.y, i_this->current.pos.z, false);
    mDoMtx_YrotM(*calc_mtx, i_this->current.angle.y);
    mDoMtx_XrotM(*calc_mtx, i_this->current.angle.x);
    mDoMtx_ZrotM(*calc_mtx, i_this->current.angle.z);
    i_this->model->setBaseTRMtx(*calc_mtx);
    return TRUE;
}

/* 000001BC-000001C4       .text daSwtdoor_IsDelete__FP13swtdoor_class */
static BOOL daSwtdoor_IsDelete(swtdoor_class* i_this) {
    return TRUE;
}

/* 000001C4-000001F4       .text daSwtdoor_Delete__FP13swtdoor_class */
static BOOL daSwtdoor_Delete(swtdoor_class* i_this) {
    dComIfG_resDelete(&i_this->mPhs, "Swtdoor");
    return TRUE;
}

/* 000001F4-00000260       .text useHeapInit__FP10fopAc_ac_c */
static BOOL useHeapInit(fopAc_ac_c* i_ac) {
    swtdoor_class * i_this = (swtdoor_class *)i_ac;
    J3DModelData * modelData = (J3DModelData *)dComIfG_getObjectRes("Swtdoor", SWTDOOR_BMD_SWTDOOR);
    i_this->model = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (i_this->model == NULL)
        return FALSE;
    return TRUE;
}

/* 00000260-00000374       .text daSwtdoor_Create__FP10fopAc_ac_c */
static cPhs_State daSwtdoor_Create(fopAc_ac_c* i_ac) {
    swtdoor_class * i_this;

    fopAcM_SetupActor(i_ac, swtdoor_class);
    i_this = (swtdoor_class *)i_ac;

    cPhs_State rt = dComIfG_resLoad(&i_this->mPhs, "Swtdoor");
    if (rt == cPhs_ERROR_e)
        return cPhs_ERROR_e;

    if (rt != cPhs_COMPLEATE_e)
        return rt;

    i_this->field_0x29c = (fopAcM_GetParam(i_this) >> 0) & 0xFF;
    if (i_this->field_0x29c == 0xFF)
        i_this->field_0x29c = 0;

    i_this->mSwitchNo = (fopAcM_GetParam(i_this) >> 24) & 0xFF;
    if (fopAcM_entrySolidHeap(i_ac, useHeapInit, 0x3000) == 0)
        return cPhs_ERROR_e;

    fopAcM_SetMin(i_this, -2000.0f, -1000.0f, -2000.0f);
    fopAcM_SetMax(i_this, 2000.0f, 1000.0f, 2000.0f);
    fopAcM_SetMtx(i_this, i_this->model->getBaseTRMtx());
    i_this->home.pos.y = 0.0f;
    return cPhs_COMPLEATE_e;
}

static actor_method_class l_daSwtdoor_Method = {
    (process_method_func)daSwtdoor_Create,
    (process_method_func)daSwtdoor_Delete,
    (process_method_func)daSwtdoor_Execute,
    (process_method_func)daSwtdoor_IsDelete,
    (process_method_func)daSwtdoor_Draw,
};

actor_process_profile_definition g_profile_SWTDOOR = {
    /* LayerID      */ fpcLy_CURRENT_e,
    /* ListID       */ 0x0007,
    /* ListPrio     */ fpcPi_CURRENT_e,
    /* ProcName     */ PROC_SWTDOOR,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(swtdoor_class),
    /* SizeOther    */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Priority     */ PRIO_SWTDOOR,
    /* Actor SubMtd */ &l_daSwtdoor_Method,
    /* Status       */ fopAcStts_CULL_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* CullType     */ fopAc_CULLBOX_CUSTOM_e,
};
