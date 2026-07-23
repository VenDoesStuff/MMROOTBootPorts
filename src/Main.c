#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z64player.h"

extern FloorType sPlayerFloorType;
extern s16 sFloorPitchShape;
extern FloorProperty sPrevFloorProperty;
u8 hoverBootsTimer = 0;
void func_8082E1F0(Player* this, u16 sfxId);

enum config_selected_boots {
    CONFIG_DEFAULT,
    CONFIG_HOVER,
    CONFIG_IRON,
};

// #define HOVER_BOOTS_ENABLED recomp_get_config_u32("selected_boots") == CONFIG_HOVER
// #define IRON_BOOTS_ENABLED recomp_get_config_u32("selected_boots") == CONFIG_IRON
u8 HOVER_BOOTS_ENABLED = false;
u8 IRON_BOOTS_ENABLED = false;

RECOMP_HOOK("Player_Update")
void ToggleBootsWithRZDpad(Actor* thisx, PlayState* play, Player* this) {
    Input* input = CONTROLLER1(&play->state);
    if (CHECK_BTN_ALL(input->cur.button, BTN_L) && CHECK_BTN_ALL(input->press.button, BTN_A)) {

        if (HOVER_BOOTS_ENABLED == true) {
            HOVER_BOOTS_ENABLED = false;
            // func_8082E1F0(this, NA_SE_PL_TAKE_OUT_SHIELD);
        } else 
        HOVER_BOOTS_ENABLED = true;
        IRON_BOOTS_ENABLED = false;
        func_8082E1F0(this, NA_SE_PL_CHANGE_ARMS);

    } else if (CHECK_BTN_ALL(input->cur.button, BTN_L) && CHECK_BTN_ALL(input->press.button, BTN_Z)) {

        if (IRON_BOOTS_ENABLED == true) {
            IRON_BOOTS_ENABLED = false;
            // func_8082E1F0(this, NA_SE_PL_TAKE_OUT_SHIELD);
        } else
        IRON_BOOTS_ENABLED = true;
        HOVER_BOOTS_ENABLED = false;
        func_8082E1F0(this, NA_SE_PL_CHANGE_ARMS);
    }
}

s32 func_808340AC(FloorType floorType);
bool func_808340D4(FloorType floorType);
s32 func_8083784C(Player* this);

s16 ootBootsData[][17] = {
    // PLAYER_BOOTS_HOVER
    {
        200,                         // REG(19)
        1000,                        // REG(30)
        300,                         // REG(32)
        800,                         // REG(34)
        500,                         // REG(35)
        400,                         // REG(36)
        800,                         // REG(37)
        400,                         // REG(38)
        800,                         // R_DECELERATE_RATE
        550,                         // R_RUN_SPEED_LIMIT
        -100,                        // REG(68)
        600,                         // REG(69)
        540,                         // IREG(66)
        750,                         // IREG(67)
        125,                         // IREG(68)
        400,                         // IREG(69)
        200,                         // MREG(95)
    },
    // PLAYER_BOOTS_IRON
    {
        200,                         // REG(19)
        1000,                        // REG(30)
        300,                         // REG(32)
        800,                         // REG(34)
        500,                         // REG(35)
        400,                         // REG(36)
        1000,                         // REG(37)
        0,                         // REG(38)
        800,                         // R_DECELERATE_RATE
        300,                         // R_RUN_SPEED_LIMIT
        -160,                        // REG(68)
        600,                         // REG(69)
        590,                         // IREG(66)
        750,                         // IREG(67)
        125,                         // IREG(68)
        200,                         // IREG(69)
        200,                         // MREG(95)
    },
    // PLAYER_BOOTS_IRON_UNDERWATER
    {
        200,                         // REG(19)
        1000,                        // REG(30)
        300,                         // REG(32)
        800,                         // REG(34)
        500,                         // REG(35)
        400,                         // REG(36)
        800,                         // REG(37)
        400,                         // REG(38)
        800,                         // R_DECELERATE_RATE
        550,                         // R_RUN_SPEED_LIMIT
        -160,                        // REG(68)
        600,                         // REG(69)
        540,                         // IREG(66)
        750,                         // IREG(67)
        125,                         // IREG(68)
        400,                         // IREG(69)
        200,                         // MREG(95)
    },
};

PlayState* sPlay;
Player* sPlayer;

RECOMP_HOOK("func_80123140") 
void setup_setBoots(PlayState* play, Player* player) {
    sPlay = play;
    sPlayer = player;
}

RECOMP_HOOK_RETURN("func_80123140") 
void setBoots() {
    PlayState* play = sPlay;
    Player* player = sPlayer;

    s8 selectedBoots = -1;

    if (HOVER_BOOTS_ENABLED) {
        selectedBoots = 0;
    } else if (IRON_BOOTS_ENABLED) {
        selectedBoots = 1;
        // iron boots have different boot values underwater
        if (player->stateFlags1 & PLAYER_STATE1_8000000) {
            selectedBoots = 2;
        }
    } else {
        return; // no boots
    }

    REG(19) = ootBootsData[selectedBoots][0];
    REG(30) = ootBootsData[selectedBoots][1];
    REG(32) = ootBootsData[selectedBoots][2];
    REG(34) = ootBootsData[selectedBoots][3];
    REG(35) = ootBootsData[selectedBoots][4];
    REG(36) = ootBootsData[selectedBoots][5];
    REG(37) = ootBootsData[selectedBoots][6];
    REG(38) = ootBootsData[selectedBoots][7];
    // REG(39) = ootBootsData[selectedBoots][8];
    R_DECELERATE_RATE = ootBootsData[selectedBoots][8];
    R_RUN_SPEED_LIMIT = ootBootsData[selectedBoots][9];
    REG(68) = ootBootsData[selectedBoots][10]; // gravity
    REG(69) = ootBootsData[selectedBoots][11];
    IREG(66) = ootBootsData[selectedBoots][12];
    IREG(67) = ootBootsData[selectedBoots][13];
    IREG(68) = ootBootsData[selectedBoots][14];
    IREG(69) = ootBootsData[selectedBoots][15];
    MREG(95) = ootBootsData[selectedBoots][16];
}

// Handles a special case where the Hover Boots are able to activate when standing on certain floor types even if the
// player is standing on the ground.  from OOT 

s32 Player_UpdateHoverBoots(Player* this) {
    s32 canHoverOnGround;
    
    if ((HOVER_BOOTS_ENABLED == true) && (hoverBootsTimer != 0)) {
        hoverBootsTimer--;
    } else {
        hoverBootsTimer = 0;
    }

    // never gets set to true?
    canHoverOnGround =
        (HOVER_BOOTS_ENABLED == true) &&
        ((this->actor.depthInWater >= 0.0f) || (func_808340AC(sPlayerFloorType) >= 0) || func_808340D4(sPlayerFloorType));

    if (canHoverOnGround && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && (hoverBootsTimer != 0)) {
        this->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        if (!canHoverOnGround) {
            hoverBootsTimer = 19;
        }

        return false;
    } else {
        sPlayerFloorType = FLOOR_TYPE_0;
        this->floorPitch = this->floorPitchAlt = sFloorPitchShape = 0;

        return true;
    }
}

RECOMP_HOOK("Player_UpdateUpperBody")
void onPlayer_UpdateUpperBody(Player* this, PlayState* play) {
    if (!(this->stateFlags1 & PLAYER_STATE1_800000) && (this->actor.parent != NULL) && Player_IsHoldingHookshot(this)) {
        hoverBootsTimer = 0;
    }
}

RECOMP_HOOK("func_808395F0")
void onfunc_808395F0(Player* this, PlayState* play) {
    hoverBootsTimer = 0;
}

RECOMP_HOOK("func_80834D50")
void onfunc_80834D50() {
    hoverBootsTimer = 0;
}

RECOMP_HOOK ("func_8083EA44") void HoverSFXHook(Player* this, f32 arg1) {
    if ((HOVER_BOOTS_ENABLED == true) && !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        (hoverBootsTimer != 0)) {
        Actor_PlaySfx_Flagged2(&this->actor, NA_SE_PL_HOBBERBOOTS_LV - SFX_FLAG);
    }
}

RECOMP_PATCH s32 func_808430E0(Player* this) {
    if (HOVER_BOOTS_ENABLED && this->actor.id == ACTOR_PLAYER) {
        return Player_UpdateHoverBoots(this);
    }

    if ((this->transformation == PLAYER_FORM_DEKU) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        func_8083784C(this)) {
        this->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
    }
    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        return false;
    }

    if (!(this->stateFlags1 & PLAYER_STATE1_8000000)) {
        sPlayerFloorType = FLOOR_TYPE_0;
    }
    this->floorPitch = 0;
    this->floorPitchAlt = 0;
    sFloorPitchShape = 0;
    return true;
}

// RECOMP_HOOK ("func_8083827C") void HoverVelocitySetup(Player* this, PlayState* play) {

// }

// RECOMP_HOOK_RETURN ("func_80083827C") void HoverVelocity() {
//         if (hoverBootsTimer != 0) {
//         this->actor.velocity.y = 1.0f;
//         sPrevFloorProperty = FLOOR_PROPERTY_9;
//         return;
//     }
// }

void func_80834140(PlayState* play, Player* this, PlayerAnimationHeader* anim);
void func_808345C8(void);
void func_8083B8D0(PlayState* play, Player* this);
s32 func_808373F8(PlayState* play, Player* this, u16 sfxId);
void func_80169EFC(PlayState* this);
void func_8083B930(PlayState* play, Player* this);
void func_8083B32C(PlayState* play, Player* this, f32 arg2);
void Player_SetupTurnInPlace(PlayState* play, Player* this, s16 yaw);
void Player_Action_33(Player* this, PlayState* play);
void Player_Action_49(Player* this, PlayState* play);
void Player_Action_28(Player* this, PlayState* play);
void Player_Action_WaitForPutAway(Player* this, PlayState* play);
s32 Player_SetAction(PlayState* play, Player* this, PlayerActionFunc actionFunc, s32 arg3);
void Player_Action_1(Player* this, PlayState* play);
void Player_Action_43(Player* this, PlayState* play);
void Player_Action_61(Player* this, PlayState* play);
void Player_Action_54(Player* this, PlayState* play);
void Player_Action_62(Player* this, PlayState* play);
void Player_Action_57(Player* this, PlayState* play);
void Player_Action_58(Player* this, PlayState* play);
void Player_Action_59(Player* this, PlayState* play);
void Player_Action_60(Player* this, PlayState* play);
void Player_Action_55(Player* this, PlayState* play);
void Player_Action_25(Player* this, PlayState* play);
void Player_Action_27(Player* this, PlayState* play);
void Player_Action_96(Player* this, PlayState* play);
void Player_Action_82(Player* this, PlayState* play);
void Player_Action_83(Player* this, PlayState* play);
s32 Player_SetAction(PlayState* play, Player* this, PlayerActionFunc actionFunc, s32 arg3);
void Player_Action_56(Player* this, PlayState* play);
s32 func_808381F8(PlayState* play, Player* this);
s32 func_80835428(PlayState* play, Player* this);
void func_8082DC64(PlayState* play, Player* this);
s32 func_80837730(PlayState* play, Player* this, f32 arg2, s32 scale);
void Player_Anim_PlayOnceMorph(PlayState* play, Player* this, PlayerAnimationHeader* anim);
extern f32 sControlStickMagnitude;
extern FloorProperty sPrevFloorProperty;
extern f32 sPlayerYDistToFloor; // missing
extern Input* sPlayerControlInput; // missing
void func_80834D50(PlayState* play, Player* this, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
void func_808373A4(PlayState* play, Player* this);
void Player_AnimSfx_PlayVoice(Player* this, u16 sfxId);
void Player_StopHorizontalMovement(Player* this);
void func_8082DD2C(PlayState* play, Player* this);
f32 func_80835CD8(PlayState* play, Player* this, Vec3f* arg2, Vec3f* pos, CollisionPoly** outPoly, s32* outBgId);
void func_80834DB8(Player* this, PlayerAnimationHeader* anim, f32 speed, PlayState* play);
s32 func_80837DEC(Player* this, PlayState* play);
void Player_Anim_PlayLoop(PlayState* play, Player* this, PlayerAnimationHeader* anim);
void Player_Action_CsAction(Player* this, PlayState* play);
void Player_Anim_PlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);
extern u32 sPlayerTouchedWallFlags; // missing
void func_8082DAD4(Player* this);
extern Actor* interactRangeActor; // missing
u16 Player_GetFloorSfxByAge(Player* this, u16 sfxId);

extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_down;
extern PlayerAnimationHeader gPlayerAnim_link_normal_jump;
extern PlayerAnimationHeader gPlayerAnim_link_normal_run_jump;
extern PlayerAnimationHeader gPlayerAnim_link_normal_run_jump_water_fall;
extern PlayerAnimationHeader gPlayerAnim_link_normal_landing_wait;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_deep_start;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_get;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_deep_end;
extern PlayerAnimationHeader gPlayerAnim_link_normal_250jump_start;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_15step_up;
extern PlayerAnimationHeader gPlayerAnim_link_normal_150step_up;
extern PlayerAnimationHeader gPlayerAnim_link_normal_100step_up;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_wait2swim_wait;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_land2swim_wait;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_get;
extern PlayerAnimationHeader gPlayerAnim_link_swimer_swim_deep_end;

Vec3f D_8085D154 = { 0.0f, 0.0f, 100.0f };

// probably temp patch
RECOMP_PATCH void func_8083827C(Player* this, PlayState* play) {
    s32 temp_t0; // sp64
    CollisionPoly* sp60;
    s32 sp5C;
    WaterBox* waterBox;
    Vec3f sp4C;
    f32 sp48;
    f32 sp44;

    this->fallDistance = this->fallStartHeight - (s32)this->actor.world.pos.y;
    if (!(this->stateFlags1 & (PLAYER_STATE1_8000000 | PLAYER_STATE1_20000000)) &&
        ((this->stateFlags1 & PLAYER_STATE1_80000000) ||
         !(this->stateFlags3 & (PLAYER_STATE3_200 | PLAYER_STATE3_2000))) &&
        !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        if (func_80835428(play, this)) {
            return;
        }

        if (sPrevFloorProperty == FLOOR_PROPERTY_8) {
            this->actor.world.pos.x = this->actor.prevPos.x;
            this->actor.world.pos.z = this->actor.prevPos.z;
            return;
        }

        if ((this->stateFlags3 & PLAYER_STATE3_2) || (this->skelAnime.movementFlags & ANIM_FLAG_80)) {
            return;
        }

        if ((Player_Action_25 == this->actionFunc) || (Player_Action_27 == this->actionFunc) ||
            (Player_Action_28 == this->actionFunc) || (Player_Action_96 == this->actionFunc) ||
            (Player_Action_82 == this->actionFunc) || (Player_Action_83 == this->actionFunc)) {
            return;
        }

        if ((sPrevFloorProperty == FLOOR_PROPERTY_7) || (this->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0) ||
            ((this->skelAnime.movementFlags & ANIM_FLAG_ENABLE_MOVEMENT) && func_808381F8(play, this))) {
            Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.prevPos);
            if (this->speedXZ > 0.0f) {
                Player_StopHorizontalMovement(this);
            }
            this->actor.bgCheckFlags |= BGCHECKFLAG_GROUND_TOUCH;
            return;
        }

        // handle hover boots behavior here
        if (HOVER_BOOTS_ENABLED && hoverBootsTimer != 0) {
            this->actor.velocity.y = 1.0f;
            sPrevFloorProperty = FLOOR_PROPERTY_9;
            return;
        }

        temp_t0 = BINANG_SUB(this->yaw, this->actor.shape.rot.y);
        Player_SetAction(play, this, Player_Action_25, 1);
        func_8082DD2C(play, this);

        this->floorSfxOffset = this->prevFloorSfxOffset;
        if ((this->transformation != PLAYER_FORM_GORON) &&
            ((this->transformation != PLAYER_FORM_DEKU) || (this->remainingHopsCounter != 0)) &&
            (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_LEAVE)) {
            if (!(this->stateFlags1 & PLAYER_STATE1_8000000)) {
                if ((sPrevFloorProperty != FLOOR_PROPERTY_6) && (sPrevFloorProperty != FLOOR_PROPERTY_9) &&
                    (sPlayerYDistToFloor > 20.0f) && (this->meleeWeaponState == PLAYER_MELEE_WEAPON_STATE_0)) {
                    if ((ABS_ALT(temp_t0) < 0x2000) && (this->speedXZ > 3.0f)) {
                        if (!(this->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR)) {
                            if (((this->transformation == PLAYER_FORM_ZORA) &&
                                 CHECK_BTN_ALL(sPlayerControlInput->cur.button, BTN_A)) ||
                                ((sPrevFloorProperty == FLOOR_PROPERTY_11) &&
                                 (this->transformation != PLAYER_FORM_GORON) &&
                                 (this->transformation != PLAYER_FORM_DEKU))) {

                                sp48 = func_80835CD8(play, this, &D_8085D154, &sp4C, &sp60, &sp5C);
                                sp44 = this->actor.world.pos.y;

                                if (WaterBox_GetSurface1(play, &play->colCtx, sp4C.x, sp4C.z, &sp44, &waterBox) &&
                                    ((sp44 - sp48) > 50.0f)) {
                                    func_80834DB8(this, &gPlayerAnim_link_normal_run_jump_water_fall, 6.0f, play);
                                    Player_SetAction(play, this, Player_Action_27, 0);
                                    return;
                                }
                            }
                        }
                        func_808373F8(play, this, NA_SE_VO_LI_AUTO_JUMP);
                        return;
                    }
                }
            }
        }

        // Checking if the ledge is tall enough for Player to hang from
        if ((sPrevFloorProperty == FLOOR_PROPERTY_9) || (sPlayerYDistToFloor <= this->ageProperties->unk_34) ||
            !func_80837DEC(this, play)) {
            Player_Anim_PlayLoop(play, this, &gPlayerAnim_link_normal_landing_wait);
        }
    } else {
        this->fallStartHeight = this->actor.world.pos.y;
        this->remainingHopsCounter = 5;
    }
}

RECOMP_HOOK("Player_UpdateCommon")
void Equip_IronBoots (Player* this, PlayState* play, Input* input) {

recomp_printf ("floor type %d\n", sPlayerFloorType);

    if (HOVER_BOOTS_ENABLED && (this->transformation == PLAYER_FORM_HUMAN)) {
        sPlayerFloorType = FLOOR_TYPE_5;
    }

    if (IRON_BOOTS_ENABLED && (this->transformation == PLAYER_FORM_HUMAN)) {
        this->currentBoots = PLAYER_BOOTS_ZORA_UNDERWATER;
        func_80123140(play, this);
    } else if ((this->transformation == PLAYER_FORM_HUMAN) && (this->currentBoots != PLAYER_BOOTS_HYLIAN) && (this->currentMask != PLAYER_MASK_GIANT)) {
        this->currentBoots = PLAYER_BOOTS_HYLIAN;
        func_80123140(play, this);
    }
}

// todo: IronBoots walking SFX

// RECOMP_HOOK ("Player_AnimSfx_PlayFloorWalk") 
// void IronSFX(Player* this, f32 freqVolumeLerp) {
//     s32 sfxId;

//     if ((this->currentMask == PLAYER_MASK_GIANT) || (IRON_BOOTS_ENABLED && this->transformation == PLAYER_FORM_HUMAN)) {
//         sfxId = NA_SE_PL_GIANT_WALK;
//     } else {
//         sfxId = Player_GetFloorSfxByAge(this, NA_SE_PL_WALK_GROUND);
//     }
// }

#define PLAYER_STATE1_23 PLAYER_STATE1_800000 // (1 << 23)

extern Vec3s gZeroVec3s;
extern Gfx gHoverBootsCircleDL[];
extern Gfx gLinkAdultLeftHoverBootDL[];
extern Gfx gLinkAdultRightHoverBootDL[];
extern Gfx gLinkAdultLeftIronBootDL[];
extern Gfx gLinkAdultRightIronBootDL[];

RECOMP_HOOK("Player_PostLimbDrawGameplay")
void Draw_OoTBoots(PlayState* play, s32 limbIndex, Gfx** dList1, Gfx** dList2, Vec3s* rot, Actor* actor) {
    Player* this = GET_PLAYER(play);
    
    OPEN_DISPS(play->state.gfxCtx);
    
    // temp index?
    // original circle draw was in `Player_DrawGameplay`, but that made him super huge
    if (limbIndex == PLAYER_LIMB_TORSO) {
        // if ((this->currentBoots == PLAYER_BOOTS_HOVER) && !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        //     !(this->stateFlags1 & PLAYER_STATE1_23) && ((u32)this->hoverBootsTimer != 0)) {
        if ((HOVER_BOOTS_ENABLED) && !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
            !(this->stateFlags1 & PLAYER_STATE1_23) && ((u32)hoverBootsTimer != 0)) {
            static s32 D_8085486C = 255;

            if (hoverBootsTimer < 19) {
                if (hoverBootsTimer >= 15) {
                    D_8085486C = (19 - hoverBootsTimer) * 51.0f;
                } else if (hoverBootsTimer < 19) {
                    s32 sp5C = hoverBootsTimer;

                    if (sp5C > 9) {
                        sp5C = 9;
                    }

                    D_8085486C = (-sp5C * 4) + 36;
                    D_8085486C = SQ(D_8085486C);
                    D_8085486C = (s32)((Math_CosS(D_8085486C) * 100.0f) + 100.0f) + 55.0f;
                    D_8085486C *= sp5C * (1.0f / 9.0f);
                }

                // Matrix_SetTranslateRotateYXZ(this->actor.world.pos.x, this->actor.world.pos.y + 2.0f,
                //                             this->actor.world.pos.z, &D_80854864);
                Matrix_SetTranslateRotateYXZ(this->actor.world.pos.x, this->actor.world.pos.y + 2.0f,
                                            this->actor.world.pos.z, &gZeroVec3s);
                Matrix_Scale(4.0f, 4.0f, 4.0f, MTXMODE_APPLY);

                // MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_player.c", 19317);
                MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
                gSPSegment(POLY_XLU_DISP++, 0x08,
                        Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, 0, 0, 16, 32, 1, 0,
                                            (play->gameplayFrames * -15) % 128, 16, 32));
                gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 255, D_8085486C);
                gDPSetEnvColor(POLY_XLU_DISP++, 120, 90, 30, 128);
                gSPDisplayList(POLY_XLU_DISP++, gHoverBootsCircleDL);
            }
        }
    } else if (limbIndex == PLAYER_LIMB_LEFT_FOOT) {
        if (HOVER_BOOTS_ENABLED) {
            gSPDisplayList(POLY_OPA_DISP++, gLinkAdultLeftHoverBootDL);
        } else if (IRON_BOOTS_ENABLED) {
            gSPDisplayList(POLY_OPA_DISP++, gLinkAdultLeftIronBootDL);
        }
    } else if (limbIndex == PLAYER_LIMB_RIGHT_FOOT) {
        if (HOVER_BOOTS_ENABLED) {
            gSPDisplayList(POLY_OPA_DISP++, gLinkAdultRightHoverBootDL);
        } else if (IRON_BOOTS_ENABLED) {
            gSPDisplayList(POLY_OPA_DISP++, gLinkAdultRightIronBootDL);
        }
    }
    
    CLOSE_DISPS(play->state.gfxCtx);
}