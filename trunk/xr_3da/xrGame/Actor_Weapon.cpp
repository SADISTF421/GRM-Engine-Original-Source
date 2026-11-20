// Actor_Weapon.cpp:	 для работы с оружием
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "actor.h"
#include "actoreffector.h"
#include "Missile.h"
#include "inventory.h"
#include "weapon.h"
#include "map_manager.h"
#include "level.h"
#include "CharacterPhysicsSupport.h"
#include "EffectorShot.h"
#include "WeaponMagazined.h"
#include "Grenade.h"
#include "game_base_space.h"
#include "Artifact.h"
#include "WeaponKnife.h"
#include "WeaponBinoculars.h"
#include "../CustomHUD.h"
#include "../CameraBase.h"
#include "../xr_input.h"
#include "xr_level_controller.h"
#include "ActorCondition.h"

static const float VEL_MAX		= 10.f;
static const float VEL_A_MAX	= 10.f;

//возвращает текуший разброс стрельбы (в радианах)с учетом движения
// SHIZOMOD: held breath over the shoulder. The sprint key (Shift) while aiming in third
// person: a small extra zoom, and the shots spread and kick a bit less.
// Holding the breath costs stamina; out of it, the hands shake again until the hero gets
// his wind back (a third of the bar).
static bool			g_tps_breath_spent	= false;
static const float	tps_breath_cost		= .08f;		// stamina per second while held
static const float	tps_breath_out		= .1f;		// below this the breath gives out
static const float	tps_breath_back		= .35f;		// and comes back above this

static bool tps_breath_held(CActor* actor)
{
	if ((actor->active_cam() != eacLookAt) || actor->Holder() || !actor->g_Alive())
		return			(false);

	CWeapon*			W = smart_cast<CWeapon*>(actor->inventory().ActiveItem());
	if (!W || !W->IsZoomed())
		return			(false);

	for (int dik = 0; dik < 256; ++dik)
		if (is_binded(kSPRINT_TOGGLE, dik) && pInput->iGetAsyncKeyState(dik))
			return		(true);

	return				(false);
}

bool CActor::tps_steady_aim() const
{
	return				(!g_tps_breath_spent && tps_breath_held(const_cast<CActor*>(this)));
}

void tps_steady_aim_update(CActor* actor, float dt)
{
	CActorCondition&	condition = actor->conditions();
	if (g_tps_breath_spent) {
		if (condition.GetPower() > tps_breath_back)
			g_tps_breath_spent	= false;
		return;
	}

	if (!tps_breath_held(actor))
		return;

	condition.ChangePower	(-tps_breath_cost*dt);
	if (condition.GetPower() < tps_breath_out)
		g_tps_breath_spent	= true;
}

float CActor::GetWeaponAccuracy() const
{
	CWeapon* W	= smart_cast<CWeapon*>(inventory().ActiveItem());

	if(m_bZoomAimingMode&&W&&!W->IsRotatingToZoom())
		return 0.f;

	float dispersion = m_fDispBase;

	CEntity::SEntityState state;
	if (g_State(state))
	{
		// angular factor
		dispersion *= (1.f + (state.fAVelocity/VEL_A_MAX)*m_fDispVelFactor);
//		Msg("--- base=[%f] angular disp=[%f]",m_fDispBase, dispersion);
		// linear movement factor
		bool bAccelerated = isActorAccelerated(mstate_real, IsZoomAimingMode());
		if( bAccelerated )
			dispersion *= (1.f + (state.fVelocity/VEL_MAX)*
			m_fDispVelFactor*(1.f + m_fDispAccelFactor));
		else
			dispersion *= (1.f + (state.fVelocity/VEL_MAX)*m_fDispVelFactor);

		if (state.bCrouch){	
			dispersion *= (1.f + m_fDispCrouchFactor);

			if(!bAccelerated )
				dispersion *= (1.f + m_fDispCrouchNoAccelFactor);
		}
	}

	return dispersion;
}


void CActor::g_fireParams	(const CHudItem* pHudItem, Fvector &fire_pos, Fvector &fire_dir)
{
//	VERIFY			(inventory().ActiveItem());

	fire_pos		= Cameras().Pos();
	fire_dir		= Cameras().Dir();

	// SHIZOMOD: in third person the camera hangs behind the shoulder. Shots and throws leave
	// from the eyes, towards whatever the crosshair rests on - not from the camera, which would
	// shoot through the hero's back and over any cover he hides behind.
	if (eacLookAt == cam_active && !m_holder && this == Level().CurrentControlEntity())
	{
		const Fvector	cam_pos	= Cameras().Pos();
		const Fvector	cam_dir	= Cameras().Dir();
		const Fvector	eye		= cameras[eacFirstEye]->vPosition;

		// look along the crosshair from where it passes the hero, so nothing behind him counts
		float			skip	= _max(0.f, Fvector().sub(eye, cam_pos).dotproduct(cam_dir));
		Fvector			start;	start.mad(cam_pos, cam_dir, skip);
		const float		range	= 300.f;
		collide::rq_result	R;
		float			hit		= range;
		if (Level().ObjectSpace.RayPick(start, cam_dir, range, collide::rqtBoth, R, this))
			hit			= R.range;
		Fvector			target;	target.mad(start, cam_dir, _max(hit, 1.f));

		fire_pos		= eye;
		fire_dir.sub	(target, eye);
		if (fire_dir.square_magnitude() > EPS_L)
			fire_dir.normalize();
		else
			fire_dir	= cam_dir;
	}

	const CMissile	*pMissile = smart_cast <const CMissile*> (pHudItem);
	if (pMissile)
	{
		Fvector offset;
		XFORM().transform_dir(offset, m_vMissileOffset);
		fire_pos.add(offset);
		return;
	}

	// SHIZOMOD: with hud_crosshair_hard the bullet leaves the muzzle along the barrel,
	// where the crosshair shows - from the eye if a wall stands between them
	CWeapon* W = smart_cast<CWeapon*>(const_cast<CHudItem*>(pHudItem));
	if (W && psHUD_Flags.test(HUD_CROSSHAIR_HARD) && active_cam() == eacFirstEye && !W->IsZoomed() && W->GetHUDmode()
		&& !smart_cast<CWeaponKnife*>(W) && !smart_cast<CWeaponBinoculars*>(W))
	{
		const Fvector eye	= fire_pos;
		fire_dir			= W->get_LastBD();
		fire_pos			= W->get_LastFP();
		Fvector to;			to.sub(fire_pos, eye);
		const float len		= to.magnitude();
		if (len > EPS_L)
		{
			to.div			(len);
			collide::rq_result R;
			if (Level().ObjectSpace.RayPick(eye, to, len, collide::rqtStatic, R, this))
				fire_pos	= eye;
		}
	}
}

void CActor::g_WeaponBones	(int &L, int &R1, int &R2)
{
	R1				= m_r_hand;
	R2				= m_r_finger2;
	L				= m_l_finger1;
}

BOOL CActor::g_State (SEntityState& state) const
{
	state.bJump			= !!(mstate_real&mcJump);
	state.bCrouch		= !!(mstate_real&mcCrouch);
	state.bFall			= !!(mstate_real&mcFall);
	state.bSprint		= !!(mstate_real&mcSprint);
	state.fVelocity		= character_physics_support()->movement()->GetVelocityActual();
	state.fAVelocity	= fCurAVelocity;
	return TRUE;
}

void CActor::SetWeaponHideState (u32 State, bool bSet)
{
	if (g_Alive() && this == Level().CurrentControlEntity())
	{
		NET_Packet	P;
		u_EventGen	(P, GEG_PLAYER_WEAPON_HIDE_STATE, ID());
		P.w_u32		(State);
		P.w_u8		(u8(bSet));
		u_EventSend	(P);
	};
}
static	u16 BestWeaponSlots [] = {
	RIFLE_SLOT		,		// 2
	PISTOL_SLOT		,		// 1
	GRENADE_SLOT	,		// 3
	KNIFE_SLOT		,		// 0
};
void CActor::SelectBestWeapon	(CObject* O)
{
	if (!O) return;
	if ( IsGameTypeSingle() ) return;
	if (OnClient()) return;
	//-------------------------------------------------
	CWeapon* pWeapon = smart_cast<CWeapon*>(O);
	CGrenade* pGrenade = smart_cast<CGrenade*>(O);
	CArtefact* pArtefact = smart_cast<CArtefact*>(O);
	CInventoryItem*	pIItem	= smart_cast<CInventoryItem*> (O);
	bool NeedToSelectBestWeapon = false;
	if ((pWeapon || pGrenade || pArtefact) && pIItem)
	{
		NeedToSelectBestWeapon = true;
		if (GameID() == GAME_ARTEFACTHUNT)
		{
			if (pIItem->GetSlot() == PISTOL_SLOT || pIItem->GetSlot() == RIFLE_SLOT)
			{
				CInventoryItem* pIItemInSlot = inventory().ItemFromSlot(pIItem->GetSlot());
				if (pIItemInSlot != NULL && pIItemInSlot != pIItem)				
					NeedToSelectBestWeapon = false;
			}
		}
	}
	if (!NeedToSelectBestWeapon) return;
	//-------------------------------------------------
	for (int i=0; i<4; i++)
	{
		if (inventory().m_slots[BestWeaponSlots[i]].m_pIItem)
		{
			if (inventory().GetActiveSlot() != BestWeaponSlots[i])
			{
				inventory().Activate(BestWeaponSlots[i]);
			}
			return;
		};
	};
}

#define ENEMY_HIT_SPOT	"mp_hit_sector_location"
BOOL	g_bShowHitSectors	= TRUE;

void	CActor::HitSector(CObject* who, CObject* weapon)
{
	if (!g_bShowHitSectors) return;
	if (!g_Alive()) return;

	bool bShowHitSector = true;
	
	CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(who);

	if (!pEntityAlive || this == who) bShowHitSector = false;

	if (weapon)
	{
		CWeapon* pWeapon = smart_cast<CWeapon*> (weapon);
		if (pWeapon)
		{
			if (pWeapon->IsSilencerAttached())
			{
				bShowHitSector = false;
				if (pWeapon->IsGrenadeLauncherAttached())
				{
				}
			}
		}
	}

	if (!bShowHitSector) return;	
		Level().MapManager().AddMapLocation(ENEMY_HIT_SPOT, who->ID());
}

//////////////////////////////////////////////////////////////////////////

void	CActor::SpawnAmmoForWeapon	(CInventoryItem *pIItem)
{
	if (OnClient()) return;
	if (!pIItem) return;

	CWeaponMagazined* pWM = smart_cast<CWeaponMagazined*> (pIItem);
	if (!pWM || !pWM->AutoSpawnAmmo()) return;

	pWM->SpawnAmmo(0xffffffff, NULL, ID());
};

void	CActor::RemoveAmmoForWeapon	(CInventoryItem *pIItem)
{
	if (OnClient()) return;
	if (!pIItem) return;

	CWeaponMagazined* pWM = smart_cast<CWeaponMagazined*> (pIItem);
	if (!pWM || !pWM->AutoSpawnAmmo()) return;

	CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(inventory().GetAny(*(pWM->m_ammoTypes[0]) ));
	if (!pAmmo) return;
	pAmmo->DestroyObject();
};
