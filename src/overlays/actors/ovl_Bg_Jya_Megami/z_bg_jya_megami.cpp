#define INTERNAL_SRC_OVERLAYS_ACTORS_OVL_BG_JYA_MEGAMI_Z_BG_JYA_MEGAMI_C
#include "actor_common.h"
#include "z_bg_jya_megami.h"
#include "overlays/effects/ovl_Effect_Ss_Kakera/z_eff_ss_kakera.h"
#include "objects/object_jya_obj/object_jya_obj.h"
#include "def/code_80043480.h"
#include "def/code_8006BA00.h"
#include "def/random.h"
#include "def/sys_matrix.h"
#include "def/z_actor.h"
#include "def/z_bgcheck.h"
#include "def/z_collision_check.h"
#include "def/z_effect_soft_sprite_old_init.h"
#include "def/z_lib.h"
#include "def/z_onepointdemo.h"
#include "def/z_rcp.h"

#define FLAGS 0

void BgJyaMegami_Init(Actor* thisx, GlobalContext* globalCtx);
void BgJyaMegami_Reset(Actor* pthisx, GlobalContext* globalCtx);
void BgJyaMegami_Destroy(Actor* thisx, GlobalContext* globalCtx);
void BgJyaMegami_Update(Actor* thisx, GlobalContext* globalCtx);
void BgJyaMegami_Draw(Actor* thisx, GlobalContext* globalCtx);

void BgJyaMegami_SetupDetectLight(BgJyaMegami* pthis);
void BgJyaMegami_DetectLight(BgJyaMegami* pthis, GlobalContext* globalCtx);
void BgJyaMegami_SetupExplode(BgJyaMegami* pthis);
void BgJyaMegami_Explode(BgJyaMegami* pthis, GlobalContext* globalCtx);

static Vec3f sVec_46 = { 0.0f, 0.0f, 0.0f };


ActorInit Bg_Jya_Megami_InitVars = {
    ACTOR_BG_JYA_MEGAMI,
    ACTORCAT_BG,
    FLAGS,
    OBJECT_JYA_OBJ,
    sizeof(BgJyaMegami),
    (ActorFunc)BgJyaMegami_Init,
    (ActorFunc)BgJyaMegami_Destroy,
    (ActorFunc)BgJyaMegami_Update,
    (ActorFunc)BgJyaMegami_Draw,
    (ActorFunc)BgJyaMegami_Reset,
};

static ColliderJntSphElementInit sJntSphElementsInit[] = {
    {
        {
            ELEMTYPE_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { 0x00200000, 0x00, 0x00 },
            TOUCH_NONE,
            BUMP_ON,
            OCELEM_NONE,
        },
        { 0, { { 0, -600, -200 }, 60 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
}; 

static BgJyaMegamiPieceInit sPiecesInit[] = {
    { { -50.0f, -21.28f, -38.92f }, -1.6f, 0xFED4, 0xFE70, 40 },
    { { -4.32f, -13.44f, -13.76f }, 0.0f, 0x04B0, 0x0190, 42 },
    { { 30.92f, -20.480001f, -28.84f }, 1.6f, 0xFCE0, 0x0320, 44 },
    { { -24.08f, -40.960003f, -21.359999f }, -1.0f, 0xFD44, 0x01F4, 36 },
    { { -44.8f, -73.92f, -49.76f }, -3.2f, 0x03E8, 0xFF38, 30 },
    { { -9.0f, -76.479996f, -13.24f }, -1.0f, 0xFC18, 0x0258, 26 },
    { { -10.240001f, -52.56f, -13.400001f }, 1.6f, 0x0258, 0xFE70, 34 },
    { { 34.04f, -61.72f, -37.04f }, 1.5f, 0x0258, 0x0258, 28 },
    { { 59.48f, -38.399998f, -49.4f }, 1.8f, 0x01F4, 0x0000, 38 },
    { { -19.04f, -112.24f, -35.120003f }, -1.6f, 0x012C, 0x0320, 22 },
    { { 12.24f, -99.04f, -31.64f }, 1.8f, 0xFC18, 0xFE70, 24 },
    { { 49.16f, -81.24f, -55.52f }, 2.4f, 0x02BC, 0x012C, 32 },
    { { 14.759999f, -125.8f, -44.16f }, 0.2f, 0x0320, 0x0258, 20 },
};

static s16 D_8089B14C[] = {
    0x0005, 0x0008, 0x000B, 0x000E, 0x0011, 0x0014, 0x0017, 0x001A,
};

static s16 D_8089B15C[] = {
    0x0012, 0x001A, 0x0022, 0x002A, 0x0032, 0x003C, 0x0046, 0x0050,
};

static s16 D_8089B16C[] = {
    0x0030, 0x002A, 0x0024, 0x0020, 0x001C, 0x0018, 0x0014, 0x0010,
};

static s16 D_8089B17C[] = {
    0x0001,
    0x0003,
    0x0007,
};

static Vec3f sVelocity = { 0.0f, 0.0f, 0.8f };

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneForward, 1200, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 800, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 1200, ICHAIN_STOP),
};

void BgJyaMegami_InitDynaPoly(BgJyaMegami* pthis, GlobalContext* globalCtx, CollisionHeader* collision, s32 flag) {
    s32 pad;
    CollisionHeader* colHeader = NULL;

    DynaPolyActor_Init(&pthis->dyna, flag);
    CollisionHeader_GetVirtual(collision, &colHeader);
    pthis->dyna.bgId = DynaPoly_SetBgActor(globalCtx, &globalCtx->colCtx.dyna, &pthis->dyna.actor, colHeader);
}

void BgJyaMegami_InitCollider(BgJyaMegami* pthis, GlobalContext* globalCtx) {
    s32 pad;

    Collider_InitJntSph(globalCtx, &pthis->collider);
    Collider_SetJntSph(globalCtx, &pthis->collider, &pthis->dyna.actor, &sJntSphInit, &pthis->colliderItem);
}

void BgJyaMegami_SpawnEffect(GlobalContext* globalCtx, Vec3f* pos, Vec3f* velocity, s32 num, s32 arg4) {
    Vec3f spB4;
    s32 i;

    for (i = 0; i < num; i++) {
        s32 idx = ((s16)(Rand_ZeroOne() * 8.0f)) & D_8089B17C[arg4];
        s16 arg5 = ((idx < 5) && (Rand_ZeroOne() < 0.7f)) ? 0x40 : 0x20;
        EffectSsKakera_Spawn(globalCtx, pos, velocity, pos, -90, arg5, D_8089B16C[idx], 4, 0, D_8089B14C[idx], 0, 5,
                             D_8089B15C[idx], KAKERA_COLOR_NONE, OBJECT_JYA_OBJ, gMegami2DL);
        if (Rand_ZeroOne() < 0.45f) {
            Math_Vec3f_Copy(&spB4, pos);
            spB4.z += 25.0f;
            func_80033480(globalCtx, &spB4, 60.0f, 0, D_8089B14C[idx] * 4 + 50, D_8089B14C[idx] * 4 + 70, 1);
        }
    }
}

void BgJyaMegami_SetupSpawnEffect(BgJyaMegami* pthis, GlobalContext* globalCtx, f32 arg2) {
    s32 i;
    Vec3f pos;

    for (i = 0; i < ARRAY_COUNT(pthis->pieces); i++) {
        if (Rand_ZeroOne() < arg2) {
            Math_Vec3f_Sum(&pthis->dyna.actor.world.pos, &sPiecesInit[i].unk_00, &pos);
            pos.z += 15.0f;
            BgJyaMegami_SpawnEffect(globalCtx, &pos, &sVelocity, 1, 0);
        }
    }
}

void BgJyaMegami_Init(Actor* thisx, GlobalContext* globalCtx) {
    BgJyaMegami* pthis = (BgJyaMegami*)thisx;

    BgJyaMegami_InitDynaPoly(pthis, globalCtx, &GMegamiCol, DPM_UNK);
    BgJyaMegami_InitCollider(pthis, globalCtx);
    if (Flags_GetSwitch(globalCtx, pthis->dyna.actor.params & 0x3F)) {
        Actor_Kill(&pthis->dyna.actor);
    } else {
        Actor_ProcessInitChain(&pthis->dyna.actor, sInitChain);
        Actor_SetFocus(&pthis->dyna.actor, -50.0f);
        BgJyaMegami_SetupDetectLight(pthis);
    }
}

void BgJyaMegami_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    BgJyaMegami* pthis = (BgJyaMegami*)thisx;

    DynaPoly_DeleteBgActor(globalCtx, &globalCtx->colCtx.dyna, pthis->dyna.bgId);
    Collider_DestroyJntSph(globalCtx, &pthis->collider);
}

void BgJyaMegami_SetupDetectLight(BgJyaMegami* pthis) {
    pthis->actionFunc = BgJyaMegami_DetectLight;
    pthis->lightTimer = 0;
    pthis->crumbleIndex = 0;
}

void BgJyaMegami_DetectLight(BgJyaMegami* pthis, GlobalContext* globalCtx) {
    if (pthis->collider.base.acFlags & AC_HIT) {
        pthis->lightTimer++;
        pthis->collider.base.acFlags &= ~AC_HIT;
        if (globalCtx->gameplayFrames % 4 == 0) {
            BgJyaMegami_SetupSpawnEffect(pthis, globalCtx, (pthis->crumbleIndex * 0.04f) + 0.05f);
        }
        func_8002F974(&pthis->dyna.actor, NA_SE_EV_FACE_CRUMBLE_SLOW - SFX_FLAG);
    } else if (pthis->lightTimer > 0) {
        pthis->lightTimer--;
    }
    if (pthis->lightTimer > 40) {
        Flags_SetSwitch(globalCtx, pthis->dyna.actor.params & 0x3F);
        BgJyaMegami_SetupExplode(pthis);
        Audio_PlaySoundAtPosition(globalCtx, &pthis->dyna.actor.world.pos, 100, NA_SE_EV_FACE_EXPLOSION);
        OnePointCutscene_Init(globalCtx, 3440, -99, &pthis->dyna.actor, MAIN_CAM);
    } else {
        if (pthis->lightTimer < 8) {
            pthis->crumbleIndex = 0;
        } else if (pthis->lightTimer < 16) {
            pthis->crumbleIndex = 1;
        } else if (pthis->lightTimer < 24) {
            pthis->crumbleIndex = 2;
        } else if (pthis->lightTimer < 32) {
            pthis->crumbleIndex = 3;
        } else {
            pthis->crumbleIndex = 4;
        }
        CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &pthis->collider.base);
    }
}

void BgJyaMegami_SetupExplode(BgJyaMegami* pthis) {
    u32 i;

    pthis->actionFunc = BgJyaMegami_Explode;
    for (i = 0; i < ARRAY_COUNT(pthis->pieces); i++) {
        Math_Vec3f_Copy(&pthis->pieces[i].pos, &pthis->dyna.actor.world.pos);
        pthis->pieces[i].vel.x = sPiecesInit[i].velX;
    }
    pthis->explosionTimer = 0;
}

void BgJyaMegami_Explode(BgJyaMegami* pthis, GlobalContext* globalCtx) {
    BgJyaMegamiPiece* temp;
    u32 i;
    Vec3f sp8C;
    BgJyaMegamiPieceInit* temp2;
    s32 pad;

    pthis->explosionTimer++;
    if (pthis->explosionTimer == 30) {
        Audio_PlaySoundAtPosition(globalCtx, &pthis->dyna.actor.world.pos, 100, NA_SE_EV_FACE_BREAKDOWN);
    }

    for (i = 0; i < ARRAY_COUNT(pthis->pieces); i++) {
        temp = &pthis->pieces[i];
        temp2 = &sPiecesInit[i];
        if (pthis->explosionTimer > temp2->delay) {
            temp->vel.y -= 0.6f;
            if (temp->vel.y < -18.0f) {
                temp->vel.y = -18.0f;
            }
            temp->vel.x *= 0.995f;
            temp->pos.x += temp->vel.x;
            temp->pos.y += temp->vel.y;
            temp->rotVelX += temp2->rotVelX;
            temp->rotVelY += temp2->rotVelY;
            if (Rand_ZeroOne() < 0.067f) {
                Math_Vec3f_Sum(&temp->pos, &temp2->unk_00, &sp8C);
                sp8C.z += 10.0f;
                BgJyaMegami_SpawnEffect(globalCtx, &sp8C, &temp->vel, 3, 2);
            }
        } else if (pthis->explosionTimer == temp2->delay) {
            Math_Vec3f_Sum(&temp->pos, &temp2->unk_00, &sp8C);
            sp8C.z += 10.0f;
            BgJyaMegami_SpawnEffect(globalCtx, &sp8C, &temp->vel, 4, 2);
        }
    }

    if ((pthis->explosionTimer % 4 == 0) && (pthis->explosionTimer > 30) && (pthis->explosionTimer < 80) &&
        (pthis->explosionTimer > 40)) {
        sp8C.x = ((Rand_ZeroOne() - 0.5f) * 90.0f) + pthis->dyna.actor.world.pos.x;
        sp8C.y = (pthis->dyna.actor.world.pos.y - (Rand_ZeroOne() * 80.0f)) - 20.0f;
        sp8C.z = pthis->dyna.actor.world.pos.z - (Rand_ZeroOne() - 0.5f) * 50.0f;
        BgJyaMegami_SpawnEffect(globalCtx, &sp8C, &sVec_46, 1, 0);
    }
    if (pthis->explosionTimer < ARRAY_COUNT(pthis->pieces)) {
        sp8C.x = pthis->dyna.actor.world.pos.x;
        sp8C.y = pthis->dyna.actor.world.pos.y - 60.0f;
        sp8C.z = pthis->dyna.actor.world.pos.z;
        func_80033480(globalCtx, &sp8C, 100.0f, 1, 150, 100, 1);
    }
    if (pthis->explosionTimer == 60) {
        Common_PlaySfx(NA_SE_SY_CORRECT_CHIME);
    }
    if (pthis->explosionTimer >= 100) {
        Actor_Kill(&pthis->dyna.actor);
    }
}

void BgJyaMegami_Update(Actor* thisx, GlobalContext* globalCtx) {
    BgJyaMegami* pthis = (BgJyaMegami*)thisx;

    pthis->actionFunc(pthis, globalCtx);
}

static void* sRightSideCrumbles[] = {
    gMegamiRightCrumble1Tex, gMegamiRightCrumble2Tex, gMegamiRightCrumble3Tex,
    gMegamiRightCrumble4Tex, gMegamiRightCrumble5Tex,
};

static void* sLeftSideCrumbles[] = {
    gMegamiLeftCrumble1Tex, gMegamiLeftCrumble2Tex, gMegamiLeftCrumble3Tex,
    gMegamiLeftCrumble4Tex, gMegamiLeftCrumble5Tex,
};

void BgJyaMegami_DrawFace(BgJyaMegami* pthis, GlobalContext* globalCtx) {
    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 706);

    func_80093D18(globalCtx->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(sRightSideCrumbles[pthis->crumbleIndex]));
    gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(sLeftSideCrumbles[pthis->crumbleIndex]));
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 716),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, gMegami1DL);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 720);
}

static Gfx* sDLists[] = {
    gMegamiPiece1DL,  gMegamiPiece2DL,  gMegamiPiece3DL,  gMegamiPiece4DL, gMegamiPiece5DL,
    gMegamiPiece6DL,  gMegamiPiece7DL,  gMegamiPiece8DL,  gMegamiPiece9DL, gMegamiPiece10DL,
    gMegamiPiece11DL, gMegamiPiece12DL, gMegamiPiece13DL,
};

void BgJyaMegami_DrawExplode(BgJyaMegami* pthis, GlobalContext* globalCtx) {
    s32 pad;
    BgJyaMegamiPiece* piece;
    u32 i;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 746);

    func_80093D18(globalCtx->state.gfxCtx);

    for (i = 0; i < ARRAY_COUNT(pthis->pieces); i++) {
        piece = &pthis->pieces[i];
        Matrix_Translate(piece->pos.x + sPiecesInit[i].unk_00.x, piece->pos.y + sPiecesInit[i].unk_00.y,
                         piece->pos.z + sPiecesInit[i].unk_00.z, MTXMODE_NEW);
        Matrix_RotateY(piece->rotVelY * (M_PI / 0x8000), MTXMODE_APPLY);
        Matrix_RotateX(piece->rotVelX * (M_PI / 0x8000), MTXMODE_APPLY);
        Matrix_Scale(0.1f, 0.1f, 0.1f, MTXMODE_APPLY);
        Matrix_Translate(sPiecesInit[i].unk_00.x * -10.0f, sPiecesInit[i].unk_00.y * -10.0f,
                         sPiecesInit[i].unk_00.z * -10.0f, MTXMODE_APPLY);

        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 778),
                  G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_OPA_DISP++, sDLists[i]);
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_bg_jya_megami.c", 783);
}

void BgJyaMegami_Draw(Actor* thisx, GlobalContext* globalCtx) {
    BgJyaMegami* pthis = (BgJyaMegami*)thisx;

    Collider_UpdateSpheres(0, &pthis->collider);
    if (pthis->actionFunc == BgJyaMegami_Explode) {
        BgJyaMegami_DrawExplode(pthis, globalCtx);
    } else {
        BgJyaMegami_DrawFace(pthis, globalCtx);
    }
}

void BgJyaMegami_Reset(Actor* pthisx, GlobalContext* globalCtx) {
    sVec_46 = { 0.0f, 0.0f, 0.0f };

    Bg_Jya_Megami_InitVars = {
        ACTOR_BG_JYA_MEGAMI,
        ACTORCAT_BG,
        FLAGS,
        OBJECT_JYA_OBJ,
        sizeof(BgJyaMegami),
        (ActorFunc)BgJyaMegami_Init,
        (ActorFunc)BgJyaMegami_Destroy,
        (ActorFunc)BgJyaMegami_Update,
        (ActorFunc)BgJyaMegami_Draw,
        (ActorFunc)BgJyaMegami_Reset,
    };

    sJntSphInit = {
        {
            COLTYPE_NONE,
            AT_NONE,
            AC_ON | AC_TYPE_PLAYER,
            OC1_NONE,
            OC2_NONE,
            COLSHAPE_JNTSPH,
        },
        1,
        sJntSphElementsInit,
    };

    sVelocity = { 0.0f, 0.0f, 0.8f };

}
