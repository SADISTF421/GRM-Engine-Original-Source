//////////////////////////////////////////////////////////////////////
// HudItem.h: класс предок для всех предметов имеющих
//			  собственный HUD (CWeapon, CMissile etc)
//////////////////////////////////////////////////////////////////////

#pragma once

class CSE_Abstract;
class CPhysicItem;
class NET_Packet;
struct HUD_SOUND;
class CInventoryItem;

#include "actor_defs.h"
#include "weaponHUD.h"

class CHudItem {
protected: //чтоб нельзя было вызвать на прямую
	CHudItem(void);
	virtual ~CHudItem(void);
	virtual DLL_Pure*_construct			();
private:
	u32				m_state;
	u32				m_nextState;
public:
	virtual void	Load				(LPCSTR section);
	virtual CHudItem*cast_hud_item		()	 { return this; }


	virtual void	PlaySound			(HUD_SOUND& snd, const Fvector& position);
										
	///////////////////////////////////////////////
	// общие функции HUD
	///////////////////////////////////////////////

	IC void			SetHUDmode			(BOOL H)		{	hud_mode = H;								}
	IC BOOL			GetHUDmode			()				{	return hud_mode;							}
	
	virtual bool	IsPending			()		const	{   return m_bPending;}
	virtual void	StopHUDSounds		()				{};
	
	//для предачи команд владельцем
	virtual bool	Action				(s32 cmd, u32 flags);
	virtual void	onMovementChanged	(ACTOR_DEFS::EMoveCommand cmd)				{};

	virtual void	OnDrawUI			()				{};
	
	IC		u32		GetNextState		() const			{return		m_nextState;}
	IC		u32		GetState			() const			{return		m_state;}

	IC		void	SetState			(u32 v)				{m_state = v; ResetSubStateTime();}
	IC		void	SetNextState		(u32 v)				{m_nextState = v;}
	//посылка сообщения на сервер о смене состояния оружия 
	virtual void	SwitchState			(u32 S);
	//прием сообщения с сервера и его обработка
	virtual void	OnStateSwitch		(u32 S);
	virtual void	OnEvent				(NET_Packet& P, u16 type);

	virtual void	OnH_A_Chield		();
	virtual void	OnH_B_Chield		();
	virtual void	OnH_B_Independent	(bool just_before_destroy);
	virtual void	OnH_A_Independent	();
	
	virtual	BOOL	net_Spawn			(CSE_Abstract* DC);
	virtual void	net_Destroy			();

	virtual void	StartIdleAnim		() {};

	
	virtual bool	Activate			();
	virtual void	Deactivate			();
	
	virtual void	OnActiveItem		() {};
	virtual void	OnHiddenItem		() {};

	virtual void	OnAnimationEnd		(u32 state)				{};

	virtual void	UpdateCL			();
	virtual void	renderable_Render	();

	virtual void	Hide() = 0;
	virtual void	Show() = 0;

	virtual void	UpdateHudPosition	();
	
	//просчет инерции для HUD 
	virtual void	UpdateHudInertion		(Fmatrix& hud_trans, float actor_yaw, float actor_pitch);
	//просчет дополнительных вычислений (переопределяется в потомках)
	virtual void	UpdateHudAdditonal		(Fmatrix&);


	virtual	void	UpdateXForm			() = 0;
	void					animGet		(MotionSVec& lst, LPCSTR prefix);

	CWeaponHUD*		GetHUD				() {return m_pHUD;}

protected:
	//TRUE - оружие занято, выполнением некоторого действия
	bool					m_bPending;

	CWeaponHUD*				m_pHUD;
	BOOL					hud_mode;
	shared_str				hud_sect;
	bool					m_bRenderHud;

	//время нахождения в текущем состоянии
	u32						m_dwStateTime;

	//кадры момента пересчета XFORM и FirePos
	u32						dwFP_Frame;
	u32						dwXF_Frame;

	//вкл/выкл инерции (временное, с плавным возвращением оружия в состояние без инерции)
	void					StartHudInertion();
	void					StopHudInertion();
private:
	bool					m_bInertionEnable;
	bool					m_bInertionAllow;

	// Плавный худ, как в OGSR: оружие догоняет камеру, кренится на стрейфе и при
	// выглядывании, подпрыгивает в прыжке и проседает при приземлении. Всё
	// подстраивается в секции оружия и его худа теми же ключами, что в OGSR.
protected:
	// в прицеле ли сейчас (свои значения смещений и инерции)
	virtual bool			HudIsAiming				() const	{ return false; }
private:
	void					LoadHudMotion			(LPCSTR section);

	struct SHudInertion
	{
		float	pitch_offset_r, pitch_offset_n, pitch_offset_d, pitch_low_limit;
		float	origin_offset, origin_offset_aim;
		float	tendto_speed, tendto_speed_aim;
	}						m_inertion;
	Fvector					m_inert_last_dir;
	bool					m_inert_started;

	// [0] смещение, [1] поворот в градусах, [2] x - включено, y - время перехода; второй индекс: 0 от бедра, 1 в прицеле
	Fvector					m_strafe_offset[3][2];
	Fvector					m_lookout_offset[3][2];
	Fvector					m_jump_offset[3][2];
	Fvector					m_fall_offset[2][2];
	Fvector					m_landing_offset[2][2];
	Fvector					m_move_offset[3];
	Fvector					m_walk_offset[3];

	// текущие значения, плавно догоняющие цель
	Fvector					m_cur_strafe[2];
	Fvector					m_cur_jump[2];
	Fvector					m_cur_lookout[2];
	Fvector					m_cur_move[2];
	Fvector					m_cur_walk[2];
protected:
	u32						m_animation_slot;
public:
	IC		u32				animation_slot			()	{	return m_animation_slot;}

private:
	CPhysicItem				*m_object;
	CInventoryItem			*m_item;

public:
	IC CPhysicItem&			object					() const {	VERIFY(m_object); return(*m_object);}
	IC CInventoryItem&		item					() const {	VERIFY(m_item);	return(*m_item);}

	virtual void			on_renderable_Render	() = 0;

protected:
	u32 m_dw_curr_substate_time;

public:
	IC void	ResetSubStateTime() { m_dw_curr_substate_time=Device.dwTimeGlobal; }
	
protected:
	u32 m_dw_BoreTimeout;
	bool m_bPendingInBore;
};

