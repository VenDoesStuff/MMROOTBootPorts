#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z64player.h"

extern FloorType sPlayerFloorType;
extern s16 sFloorPitchShape;
u8 hoverBootsTimer;

HoverBoots = false;

s32 func_808340AC(FloorType floorType);
bool func_808340D4(FloorType floorType);

s16 hoverBootsData[] = {
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
};

RECOMP_HOOK_RETURN("func_80123140") void setHoverBoots(PlayState* play, Player* player) {
    if (player->currentBoots == PLAYER_BOOTS_HOVER) {
        REG(19) = hoverBootsData[0];
        REG(30) = hoverBootsData[1];
        REG(32) = hoverBootsData[2];
        REG(34) = hoverBootsData[3];
        REG(35) = hoverBootsData[4];
        REG(36) = hoverBootsData[5];
        REG(37) = hoverBootsData[6];
        REG(38) = hoverBootsData[7];
        REG(39) = hoverBootsData[8];
        R_DECELERATE_RATE = hoverBootsData[9];
        R_RUN_SPEED_LIMIT = hoverBootsData[10];
        REG(68) = hoverBootsData[11]; // gravity
        REG(69) = hoverBootsData[12];
        IREG(66) = hoverBootsData[13];
        IREG(67) = hoverBootsData[14];
        IREG(68) = hoverBootsData[15];
        IREG(69) = hoverBootsData[16];
        MREG(95) = hoverBootsData[17];
    }
}

RECOMP_HOOK ("Player_UpdateCommon") void HoverCheck(Player* this, PlayState* play, Input* input) {
    if (CHECK_BTN_ANY(CONTROLLER1(&play->state)->cur.button, BTN_A)){
        HoverBoots = true;
    }
}

s32 Player_UpdateHoverBoots(Player* this) {
    s32 canHoverOnGround;

    if ((this->currentBoots == PLAYER_BOOTS_HOVER) && (hoverBootsTimer != 0)) {
        hoverBootsTimer--;
    } else {
        hoverBootsTimer = 0;
    }

    canHoverOnGround =
        (this->currentBoots == PLAYER_BOOTS_HOVER) &&
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

RECOMP_HOOK ("func_8083EA44") void HoverSFXHook(Player* this, f32 arg1) {
    if ((this->currentBoots == PLAYER_BOOTS_HOVER) && !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        (hoverBootsTimer != 0)) {
    Actor_PlaySfx_Flagged2(&this->actor, NA_SE_PL_HOBBERBOOTS_LV - SFX_FLAG);
}
}