#pragma once

#include "../BaseMonster/base_monster.h"
#include "../ai_monster_bones.h"
#include "../invisibility.h"
#include "../controlled_actor.h"
#include "../anim_triple.h"
#include "../../../script_export_space.h"
#include "bloodsucker_alien.h"

class CAI_Bloodsucker : public CBaseMonster, 
						public CInvisibility,
						public CControlledActor {

	typedef		CBaseMonster	inherited;
	
public:
							CAI_Bloodsucker	();
	virtual					~CAI_Bloodsucker();	

	virtual void			reinit					();
	virtual	void			reload					(LPCSTR section);

	virtual void			UpdateCL				();
	virtual void			shedule_Update			(u32 dt);
	virtual void			Die						(CObject* who);
	virtual BOOL			net_Spawn				(CSE_Abstract* DC);
	virtual void			net_Destroy				();			
	virtual	void			Load					(LPCSTR section);
	virtual	void			PitchCorrection			() {}
	
	virtual	void			CheckSpecParams			(u32 spec_params);
	virtual bool			ability_invisibility	() {return true;}

	// OGSR's force_visibility_state on SoC's energy-driven invisibility:
	// -1 the creature's own rules, 0 held invisible, 1/2 held visible (no shimmer,
	// no sound). First-person legs (swm_legs) are a bloodsucker and hold 2.
			void			force_visibility_state	(int state);
			int				m_force_visibility_state;
			// a script's "hidden" (first-person legs out of view): wins over every
			// visibility switch, and the object is not destroyed for it
			void			set_script_hidden		(bool hidden);
			bool			m_script_hidden;
	virtual bool			ability_pitch_correction() {return false;}
	virtual	void			post_fsm_update			();
	
	virtual bool			use_center_to_aim		() const {return true;}
	virtual bool			check_start_conditions	(ControlCom::EControlType);
	virtual void			HitEntity				(const CEntity *pEntity, float fDamage, float impulse, Fvector &dir);
	
	//--------------------------------------------------------------------
	// Bones
	//--------------------------------------------------------------------
private:
	static	void			BoneCallback			(CBoneInstance *B);
			void			vfAssignBones			();
			void			LookDirection			(Fvector to_dir, float bone_turn_speed);


	bonesManipulation		Bones;

	CBoneInstance			*bone_spine;
	CBoneInstance			*bone_head;

	//--------------------------------------------------------------------
	// Invisibility
	//--------------------------------------------------------------------
private:
	SMotionVel				invisible_vel;
	LPCSTR					invisible_particle_name;

	LPCSTR					invisible_run_particles_name;
	u32						m_run_particles_freq;
	u32						m_last_invisible_run_play;

public:
	virtual	void			on_activate				();
	virtual	void			on_deactivate			();
	virtual	void			on_change_visibility	(bool b_visibility);
	void					play_hidden_run_particles	();

	//--------------------------------------------------------------------
	// Vampire
	//--------------------------------------------------------------------
public:
	SAnimationTripleData	anim_triple_vampire;

	SPPInfo					pp_vampire_effector;
			
	void					ActivateVampireEffector	(float max_dist);
	void					SatisfyVampire			();
private:		
	float					m_vampire_wound;
	float					m_vampire_gain_health;

	void					LoadVampirePPEffector	(LPCSTR section);	

	//--------------------------------------------------------------------
	// Alien
	//--------------------------------------------------------------------
public:
	CBloodsuckerAlien		m_alien_control;
	u32						m_time_lunge;

			void			set_alien_control		(bool val);

	//--------------------------------------------------------------------
	// Sounds
	//--------------------------------------------------------------------
public:

	enum EBloodsuckerSounds {
		eAdditionalSounds		= MonsterSound::eMonsterSoundCustom,

		eVampireGrasp			= eAdditionalSounds | 0,
		eVampireSucking			= eAdditionalSounds | 1,
		eVampireHit				= eAdditionalSounds | 2,
		eVampireStartHunt		= eAdditionalSounds | 3,

		eGrowl					= eAdditionalSounds | 5,

		eChangeVisibility1		= eAdditionalSounds | 6,
		eChangeVisibility2		= eAdditionalSounds | 7,
		
		eAlien					= eAdditionalSounds | 8,
	};

	//--------------------------------------------------------------------

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CAI_Bloodsucker)
#undef script_type_list
#define script_type_list save_type_list(CAI_Bloodsucker)