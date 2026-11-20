//////////////////////////////////////////////////////////////////////
// HudItem.cpp: класс родитель для всех предметов имеющих
//				собственный HUD (CWeapon, CMissile etc)
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HudItem.h"
#include "physic_item.h"
#include "WeaponHUD.h"
#include "actor.h"
#include "actoreffector.h"
#include "Missile.h"
#include "xrmessages.h"
#include "level.h"
#include "inventory.h"
#include "../CameraBase.h"


CHudItem::CHudItem(void)
{
	m_pHUD				= NULL;
	SetHUDmode			(FALSE);
	m_dwStateTime		= 0;
	m_bRenderHud		= true;

	m_bInertionEnable	= true;
	m_bInertionAllow	= true;

	m_inert_last_dir.set	(0.f, 0.f, 0.f);
	m_inert_started			= false;
	for (int i = 0; i < 2; ++i)
	{
		m_cur_strafe[i].set	(0.f, 0.f, 0.f);
		m_cur_jump[i].set	(0.f, 0.f, 0.f);
		m_cur_lookout[i].set(0.f, 0.f, 0.f);
		m_cur_move[i].set	(0.f, 0.f, 0.f);
		m_cur_walk[i].set	(0.f, 0.f, 0.f);
	}
	
	m_dw_BoreTimeout = 20000;
	m_bPendingInBore = false;
	
	ResetSubStateTime();
}

CHudItem::~CHudItem(void)
{
	xr_delete			(m_pHUD);
}

DLL_Pure *CHudItem::_construct	()
{
	m_object			= smart_cast<CPhysicItem*>(this);
	VERIFY				(m_object);

	m_item				= smart_cast<CInventoryItem*>(this);
	VERIFY				(m_item);

	return				(m_object);
}

void CHudItem::Load(LPCSTR section)
{
	//загрузить hud, если он нужен
	if(pSettings->line_exist(section,"hud"))
		hud_sect		= pSettings->r_string		(section,"hud");

	if(*hud_sect){
		m_pHUD			= xr_new<CWeaponHUD> (this);
		m_pHUD->Load	(*hud_sect);
		if(pSettings->line_exist(*hud_sect, "allow_inertion")) 
			m_bInertionAllow = !!pSettings->r_bool(*hud_sect, "allow_inertion");
	}else{
		m_pHUD = NULL;
		//если hud не задан, но задан слот, то ошибка
		R_ASSERT2(item().GetSlot() == NO_ACTIVE_SLOT, "active slot is set, but hud for food item is not available");
	}

	m_animation_slot	= pSettings->r_u32(section,"animation_slot");
	
	m_dw_BoreTimeout = READ_IF_EXISTS(pSettings, r_u32, section, "bore_timeout", 20000);
	m_bPendingInBore = READ_IF_EXISTS(pSettings, r_bool, section, "pending_in_bore", FALSE);

	LoadHudMotion		(section);
}

static Fvector hud_v(float x, float y, float z)	{ Fvector v; v.set(x, y, z); return v; }
#define HUD_V3(name, x, y, z)	READ_IF_EXISTS(pSettings, r_fvector3, section, name, hud_v(x, y, z))
#define HUD_SW(on, time, t_def)	hud_v(READ_IF_EXISTS(pSettings, r_bool, section, on, TRUE) ? 1.f : 0.f, READ_IF_EXISTS(pSettings, r_float, section, time, t_def), 0.f)

// Значения по умолчанию - те же, что в OGSR.
void CHudItem::LoadHudMotion(LPCSTR section)
{
	m_strafe_offset[0][0]	= HUD_V3("strafe_hud_offset_pos",		0.025f, 0.f, 0.f);
	m_strafe_offset[1][0]	= HUD_V3("strafe_hud_offset_rot",		0.f, 0.f, 5.5f);
	m_strafe_offset[0][1]	= HUD_V3("strafe_aim_hud_offset_pos",	0.f, 0.f, 0.f);
	m_strafe_offset[1][1]	= HUD_V3("strafe_aim_hud_offset_rot",	0.f, 0.f, 3.5f);
	m_strafe_offset[2][0]	= HUD_SW("strafe_enabled",		"strafe_transition_time",		0.25f);
	m_strafe_offset[2][1]	= HUD_SW("strafe_aim_enabled",	"strafe_aim_transition_time",	0.15f);

	m_lookout_offset[0][0]	= HUD_V3("lookout_hud_offset_pos",		0.045f, 0.f, 0.f);
	m_lookout_offset[1][0]	= HUD_V3("lookout_hud_offset_rot",		0.f, 0.f, 10.f);
	m_lookout_offset[0][1]	= HUD_V3("lookout_aim_hud_offset_pos",	0.f, 0.f, 0.f);
	m_lookout_offset[1][1]	= HUD_V3("lookout_aim_hud_offset_rot",	0.f, 0.f, 15.f);
	m_lookout_offset[2][0]	= HUD_SW("lookout_enabled",		"lookout_transition_time",		0.25f);
	m_lookout_offset[2][1]	= HUD_SW("lookout_aim_enabled",	"lookout_aim_transition_time",	0.15f);

	m_jump_offset[0][0]		= HUD_V3("jump_hud_offset_pos",			0.f, 0.05f, 0.03f);
	m_jump_offset[1][0]		= HUD_V3("jump_hud_offset_rot",			0.f, -10.f, -10.f);
	m_jump_offset[0][1]		= HUD_V3("jump_aim_hud_offset_pos",		0.f, 0.03f, 0.01f);
	m_jump_offset[1][1]		= HUD_V3("jump_aim_hud_offset_rot",		0.f, 2.5f, -3.f);
	m_jump_offset[2][0]		= HUD_SW("jump_enabled",		"jump_transition_time",		0.35f);
	m_jump_offset[2][1]		= HUD_SW("jump_aim_enabled",	"jump_aim_transition_time",	0.4f);

	m_fall_offset[0][0]		= HUD_V3("fall_hud_offset_pos",			0.f, -0.05f, 0.06f);
	m_fall_offset[1][0]		= HUD_V3("fall_hud_offset_rot",			0.f, 5.f, 0.f);
	m_fall_offset[0][1]		= HUD_V3("fall_aim_hud_offset_pos",		0.f, 0.03f, -0.01f);
	m_fall_offset[1][1]		= HUD_V3("fall_aim_hud_offset_rot",		0.f, -2.5f, 3.f);

	m_landing_offset[0][0]	= HUD_V3("landing_hud_offset_pos",		0.f, -0.2f, 0.03f);
	m_landing_offset[1][0]	= HUD_V3("landing_hud_offset_rot",		0.f, -5.f, 10.f);
	m_landing_offset[0][1]	= HUD_V3("landing_aim_hud_offset_pos",	0.f, -0.1f, 0.02f);
	m_landing_offset[1][1]	= HUD_V3("landing_aim_hud_offset_rot",	0.f, -2.5f, 5.f);

	m_move_offset[0]		= HUD_V3("stay_hud_offset_pos",			0.f, -0.03f, 0.f);
	m_move_offset[1]		= HUD_V3("stay_hud_offset_rot",			0.f, 0.5f, -3.f);
	m_move_offset[2]		= HUD_SW("move_enabled",		"move_transition_time",		0.25f);

	m_walk_offset[0]		= HUD_V3("walk_hud_offset_pos",			-0.02f, -0.02f, -0.03f);
	m_walk_offset[1]		= HUD_V3("walk_hud_offset_rot",			0.f, 0.05f, -1.f);
	m_walk_offset[2]		= HUD_SW("walk_enabled",		"walk_transition_time",		0.25f);

	// Knives and pistols are short models: the movement offsets above, made for
	// rifles, pulled them down far enough to take the hands of a knife slash out
	// of view. They move by a third as much (hud_motion_factor overrides it).
	{
		const int slot = READ_IF_EXISTS(pSettings, r_s32, section, "slot", 2);
		const float k = READ_IF_EXISTS(pSettings, r_float, section, "hud_motion_factor", slot <= 1 ? 0.35f : 1.f);
		if (!fsimilar(k, 1.f))
		{
			for (int i = 0; i < 2; ++i)
			{
				m_strafe_offset[i][0].mul(k);
				m_lookout_offset[i][0].mul(k);
				m_jump_offset[i][0].mul(k);
				m_fall_offset[i][0].mul(k);
				m_landing_offset[i][0].mul(k);
				m_move_offset[i].mul(k);
				m_walk_offset[i].mul(k);
			}
		}
	}

	// инерция - из секции худа, как в OGSR
	LPCSTR hs = *hud_sect ? *hud_sect : section;
	m_inertion.pitch_offset_r	= READ_IF_EXISTS(pSettings, r_float, hs, "pitch_offset_right",			0.0f);
	m_inertion.pitch_offset_n	= READ_IF_EXISTS(pSettings, r_float, hs, "pitch_offset_up",				0.0f);
	m_inertion.pitch_offset_d	= READ_IF_EXISTS(pSettings, r_float, hs, "pitch_offset_forward",		0.02f);
	m_inertion.pitch_low_limit	= READ_IF_EXISTS(pSettings, r_float, hs, "pitch_offset_up_low_limit",	-PI);
	m_inertion.origin_offset	= READ_IF_EXISTS(pSettings, r_float, hs, "inertion_origin_offset",		-0.03f);
	m_inertion.origin_offset_aim= READ_IF_EXISTS(pSettings, r_float, hs, "inertion_zoom_origin_offset",	-0.02f);
	m_inertion.tendto_speed		= READ_IF_EXISTS(pSettings, r_float, hs, "inertion_tendto_speed",		5.f);
	m_inertion.tendto_speed_aim	= READ_IF_EXISTS(pSettings, r_float, hs, "inertion_zoom_tendto_speed",	10.f);
}
#undef HUD_V3
#undef HUD_SW

void CHudItem::net_Destroy()
{
	if(m_pHUD)
		m_pHUD->net_DestroyHud	();

	SetHUDmode			(FALSE);
	m_dwStateTime		= 0;
}

void CHudItem::PlaySound	(HUD_SOUND& hud_snd, const Fvector& position)
{
	HUD_SOUND::PlaySound	(hud_snd, position, object().H_Root(), !!GetHUDmode());
}

BOOL  CHudItem::net_Spawn	(CSE_Abstract* DC) 
{
	return TRUE;
}

void CHudItem::renderable_Render()
{
	UpdateXForm	();
	BOOL _hud_render			= ::Render->get_HUD() && GetHUDmode();
	if(_hud_render && !m_pHUD->IsHidden() && !item().IsHidden()){ 
		// HUD render
		if(m_bRenderHud){
			::Render->set_Transform		(&m_pHUD->Transform());
			::Render->add_Visual		(m_pHUD->Visual());
		}
	}
	else {
		if (!object().H_Parent() || (!_hud_render && m_pHUD && !m_pHUD->IsHidden() && !item().IsHidden()))
			on_renderable_Render		();
		else
			if (object().H_Parent()) {
				CInventoryOwner	*owner = smart_cast<CInventoryOwner*>(object().H_Parent());
				VERIFY			(owner);
				CInventoryItem	*self = smart_cast<CInventoryItem*>(this);
				if (owner->attached(self))
					on_renderable_Render();
			}
	}
}

bool CHudItem::Action(s32 cmd, u32 flags) 
{
	return false;
}

void CHudItem::SwitchState(u32 S)
{
	if (OnClient()) return;
	SetNextState( S );	// Very-very important line of code!!! :)

	if (object().Local() && !object().getDestroy())	
	{
		// !!! Just single entry for given state !!!
		NET_Packet		P;
		object().u_EventGen		(P,GE_WPN_STATE_CHANGE,object().ID());
		P.w_u8			(u8(S));
		object().u_EventSend		(P);
	}
}

void CHudItem::OnEvent		(NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_WPN_STATE_CHANGE:
		{
			u8				S;
			P.r_u8			(S);
			OnStateSwitch	(u32(S));
		}
		break;
	}
}

void CHudItem::OnStateSwitch	(u32 S)
{
	m_dwStateTime = 0;
	SetState( S );
	if(object().Remote()) SetNextState( S );
}


bool CHudItem::Activate() 
{
	if(m_pHUD) 
		m_pHUD->Init();

	Show();
	OnActiveItem ();
	return true;
}

void CHudItem::Deactivate() 
{
	Hide();
	OnHiddenItem ();
}



void CHudItem::UpdateHudPosition	()
{
	if (m_pHUD && GetHUDmode()){
		if(item().IsHidden()) 
			SetHUDmode(FALSE);

		Fmatrix							trans;

		CActor* pActor = smart_cast<CActor*>(object().H_Parent());
		if(pActor){
			pActor->Cameras().affected_Matrix			(trans);
			UpdateHudInertion							(trans, pActor->cam_FirstEye()->yaw, pActor->cam_FirstEye()->pitch);
			UpdateHudAdditonal							(trans);
			m_pHUD->UpdatePosition						(trans);
		}
	}
}

// Одна величина худа плавно догоняет свою цель за time секунд.
static void hud_tend(Fvector cur[2], const Fvector& offs, const Fvector& rot_deg, float time)
{
	if (time <= EPS)
		time = 0.01f;
	float step = Device.fTimeDelta / time;
	clamp(step, 0.f, 1.f);

	Fvector rot;
	rot.mul(rot_deg, -PI / 180.f);	// в радианы

	if (!offs.similar(cur[0], EPS))
		cur[0].lerp(cur[0], offs, step);
	if (!rot.similar(cur[1], EPS))
		cur[1].lerp(cur[1], rot, step);
}

// Движение худа от того, что делает герой - как CHudItem::UpdateHudAdditional в OGSR.
void CHudItem::UpdateHudAdditonal		(Fmatrix& hud_trans)
{
	CActor* pActor = smart_cast<CActor*>(object().H_Parent());
	if (!pActor)
		return;

	const u32	state	= pActor->MovingState();
	const bool	aim		= HudIsAiming();
	const int	idx		= aim ? 1 : 0;
	const bool	pose_free = !aim && !dynamic_cast<CMissile*>(this);	// позы ходьбы и стойки - не в прицеле и не с гранатой

	Fvector summary_offset, summary_rotate, zero;
	summary_offset.set	(0.f, 0.f, 0.f);
	summary_rotate.set	(0.f, 0.f, 0.f);
	zero.set			(0.f, 0.f, 0.f);

	//============= Боковой стрейф =============//
	if (m_strafe_offset[2][idx].x > 0.f)
	{
		Fvector offs = zero, rot = zero;
		if (state & mcLStrafe)		{ offs.invert(m_strafe_offset[0][idx]); rot.invert(m_strafe_offset[1][idx]); }
		else if (state & mcRStrafe)	{ offs.set(m_strafe_offset[0][idx]);	rot.set(m_strafe_offset[1][idx]); }
		hud_tend(m_cur_strafe, offs, rot, m_strafe_offset[2][idx].y);
		summary_offset.add(m_cur_strafe[0]);
		summary_rotate.add(m_cur_strafe[1]);
	}

	//============= Прыжок, полёт, приземление =============//
	if (m_jump_offset[2][idx].x > 0.f)
	{
		Fvector offs = zero, rot = zero;
		if (state & mcJump)								{ offs.set(m_jump_offset[0][idx]);		rot.set(m_jump_offset[1][idx]); }
		else if (state & mcFall)						{ offs.set(m_fall_offset[0][idx]);		rot.set(m_fall_offset[1][idx]); }
		else if ((state & mcLanding) || (state & mcLanding2)) { offs.set(m_landing_offset[0][idx]);	rot.set(m_landing_offset[1][idx]); }
		const float k = (state & mcLanding2) ? 1.3f : 1.f;
		offs.mul(k); rot.mul(k);
		hud_tend(m_cur_jump, offs, rot, m_jump_offset[2][idx].y);
		summary_offset.add(m_cur_jump[0]);
		summary_rotate.add(m_cur_jump[1]);
	}

	//============= Выглядывание =============//
	if (m_lookout_offset[2][idx].x > 0.f)
	{
		float k = 1.f;
		if ((state & mcCrouch) && (state & mcAccel))	k = 0.5f;
		else if (state & mcCrouch)						k = 0.75f;

		Fvector offs = zero, rot = zero;
		if ((state & mcLLookout) && !(state & mcRLookout))		{ offs.invert(m_lookout_offset[0][idx]); rot.invert(m_lookout_offset[1][idx]); }
		else if ((state & mcRLookout) && !(state & mcLLookout))	{ offs.set(m_lookout_offset[0][idx]);	rot.set(m_lookout_offset[1][idx]); }
		offs.mul(k); rot.mul(k);
		hud_tend(m_cur_lookout, offs, rot, m_lookout_offset[2][idx].y);
		summary_offset.add(m_cur_lookout[0]);
		summary_rotate.add(m_cur_lookout[1]);
	}

	//============= Стойка (присед) =============//
	if (m_move_offset[2].x > 0.f)
	{
		float k = 0.f;
		if ((state & mcCrouch) && (state & mcAccel))	k = 1.f;
		else if (state & mcCrouch)						k = 0.5f;
		if (!pose_free)									k = 0.f;

		Fvector offs, rot;
		offs.mul(m_move_offset[0], k);
		rot.mul	(m_move_offset[1], k);
		hud_tend(m_cur_move, offs, rot, m_move_offset[2].y);
		summary_offset.add(m_cur_move[0]);
		summary_rotate.add(m_cur_move[1]);
	}

	//============= Ходьба =============//
	if (m_walk_offset[2].x > 0.f)
	{
		float k = 1.f;
		if ((state & mcCrouch) && (state & mcAccel))	k = 0.5f;
		else if (state & mcCrouch)						k = 0.7f;
		if (!pose_free)									k = 0.f;

		Fvector offs = zero, rot = zero;
		if (state & mcFwd)			{ offs.set(m_walk_offset[0]); rot.set(m_walk_offset[1]); }
		else if (state & mcBack)	{ offs.set(m_walk_offset[0].x, m_walk_offset[0].y, -m_walk_offset[0].z); rot.set(m_walk_offset[1]); }
		offs.mul(k); rot.mul(k);
		hud_tend(m_cur_walk, offs, rot, m_walk_offset[2].y);
		summary_offset.add(m_cur_walk[0]);
		summary_rotate.add(m_cur_walk[1]);
	}

	//============= Применение =============//
	// поворот с сохранением смещения (by Zander, OGSR)
	Fvector pos = hud_trans.c;
	float h, p, b;
	hud_trans.getHPB	(h, p, b);
	hud_trans.setHPB	(h - summary_rotate.x, p - summary_rotate.y, b - summary_rotate.z);
	hud_trans.c			= pos;

	Fmatrix offset;
	offset.identity		();
	offset.translate_over(summary_offset);
	hud_trans.mulB_43	(offset);
}

void CHudItem::StartHudInertion()
{
	m_bInertionEnable = true;
}
void CHudItem::StopHudInertion()
{
	m_bInertionEnable = false;
}


// Инерция как в OGSR: у каждого ствола свой "запаздывающий" взгляд, который
// плавно догоняет камеру; разница сдвигает худ. Раньше направление было одно
// static на все предметы, и при смене оружия худ дёргался.
void CHudItem::UpdateHudInertion		(Fmatrix& hud_trans, float actor_yaw, float actor_pitch)
{
	if (m_pHUD && m_bInertionAllow && m_bInertionEnable){
		Fmatrix			xform				= hud_trans;
		Fvector&		origin				= hud_trans.c;

		if (!m_inert_started)
		{
			m_inert_last_dir.set	(xform.k);
			m_inert_started			= true;
		}

		// calc difference
		Fvector								diff_dir;
		diff_dir.sub						(xform.k, m_inert_last_dir);

		// clamp by PI_DIV_2
		Fvector last;						last.normalize_safe(m_inert_last_dir);
		float dot							= last.dotproduct(xform.k);
		if (dot<EPS){
			Fvector v0;
			v0.crossproduct			(m_inert_last_dir,xform.k);
			m_inert_last_dir.crossproduct	(xform.k,v0);
			diff_dir.sub			(xform.k, m_inert_last_dir);
		}

		// tend to forward
		const bool	aim				= HudIsAiming();
		float		tendto_speed	= aim ? m_inertion.tendto_speed_aim : m_inertion.tendto_speed;
		float		origin_offset	= aim ? m_inertion.origin_offset_aim : m_inertion.origin_offset;

		// фактор силы инерции (как в OGSR)
		const float	power_factor	= 0.5f;
		tendto_speed				*= power_factor;
		origin_offset				*= power_factor;

		m_inert_last_dir.mad	(diff_dir, tendto_speed * Device.fTimeDelta);
		origin.mad				(diff_dir, -origin_offset);

		// pitch compensation
		float pitch		= angle_normalize_signed(xform.k.getP());
		origin.mad		(xform.k,	-pitch * m_inertion.pitch_offset_d);
		origin.mad		(xform.i,	-pitch * m_inertion.pitch_offset_r);
		clamp			(pitch, m_inertion.pitch_low_limit, PI);
		origin.mad		(xform.j,	-pitch * m_inertion.pitch_offset_n);
	}
	else
		m_inert_started	= false;
}

void CHudItem::UpdateCL()
{
	m_dwStateTime += Device.dwTimeDelta;

	if(m_pHUD) m_pHUD->Update();
	UpdateHudPosition	();

	if (object().H_Parent() == Level().CurrentEntity())
    {
        CActor* pActor = smart_cast<CActor*>(object().H_Parent());
		if (pActor && pActor->AnyMove())
			ResetSubStateTime();
	}
}

void CHudItem::OnH_A_Chield		()
{
	SetHUDmode		(FALSE);

	if (m_pHUD) {
		if(Level().CurrentEntity() == object().H_Parent() && smart_cast<CActor*>(object().H_Parent()))
			m_pHUD->Visible(true);
		else
			m_pHUD->Visible(false);
	}
}

void CHudItem::OnH_B_Chield		()
{
	OnHiddenItem ();
}

void CHudItem::OnH_B_Independent	(bool just_before_destroy)
{
	SetHUDmode				(FALSE);

	if (m_pHUD)
		m_pHUD->Visible		(false);
	
	StopHUDSounds			();

	UpdateXForm				();
}

void CHudItem::OnH_A_Independent	()
{
}
void CHudItem::animGet	(MotionSVec& lst, LPCSTR prefix)
{
	const MotionID		&M = m_pHUD->animGet(prefix);
	if (M)				lst.push_back(M);
	for (int i=0; i<MAX_ANIM_COUNT; ++i)
	{
		string128		sh_anim;
		sprintf_s			(sh_anim,"%s%d",prefix,i);
		const MotionID	&M = m_pHUD->animGet(sh_anim);
		if (M)			lst.push_back(M);
	}
	R_ASSERT2			(!lst.empty(),prefix);
}
