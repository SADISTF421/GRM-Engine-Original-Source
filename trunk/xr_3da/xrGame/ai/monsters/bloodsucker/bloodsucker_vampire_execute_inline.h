#pragma once

#include "../../../../skeletoncustom.h"
#include "../../../actor.h"
#include "../../../../CameraBase.h"

#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CStateBloodsuckerVampireExecuteAbstract CStateBloodsuckerVampireExecute<_Object>

#define VAMPIRE_TIME_HOLD		4000
#define VAMPIRE_HIT_IMPULSE		400.f
#define VAMPIRE_MIN_DIST		0.5f
#define VAMPIRE_MAX_DIST		1.5f

extern bool g_bDisableAllInput;

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::initialize()
{
	inherited::initialize					();

	object->CControlledActor::install		();

	look_head				();

	m_action				= eActionPrepare;
	time_vampire_started	= 0;

	object->CInvisibility::manual_deactivate();

	g_bDisableAllInput = true;
	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);

	m_effector_activated			= false;
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute()
{
	if (!object->CControlledActor::is_turning() && !m_effector_activated) {
		float dist = object->EnemyMan.get_enemy()->Position().distance_to(object->Position());

		object->ActivateVampireEffector	(_abs(dist - 1.0f));
		m_effector_activated			= true;
	}
	
	look_head							();

	switch (m_action) {
		case eActionPrepare:
			execute_vampire_prepare();
			m_action = eActionContinue;
			break;

		case eActionContinue:
			execute_vampire_continue();
			break;

		case eActionFire:
			execute_vampire_hit();
			m_action = eActionWaitTripleEnd;
			break;

		case eActionWaitTripleEnd:
			if (!object->com_man().ta_is_active()) {
				m_action = eActionCompleted; 
			}

		case eActionCompleted:
			break;
	}

	object->set_action			(ACT_STAND_IDLE);
	object->dir().face_target	(object->EnemyMan.get_enemy(), 1200);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::finalize()
{
	inherited::finalize();

	object->CInvisibility::manual_activate		();

	if (object->CControlledActor::is_controlling())
		object->CControlledActor::release		();

	g_bDisableAllInput = false;
	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::critical_finalize()
{
	inherited::critical_finalize();

	if (object->CControlledActor::is_controlling())
		object->CControlledActor::release		();

	g_bDisableAllInput = false;
	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
	
	// TODO: Find if need this line
	//object->anim().TA_Deactivate				();
	object->CInvisibility::manual_activate		();
}

TEMPLATE_SPECIALIZATION
bool CStateBloodsuckerVampireExecuteAbstract::check_start_conditions()
{
	const CEntityAlive	*enemy = object->EnemyMan.get_enemy();
	
	if (enemy->CLS_ID != CLSID_OBJECT_ACTOR)								return false;
	
	// проверить дистанцию
	float dist = object->MeleeChecker.distance_to_enemy	(enemy);
	if ((dist > VAMPIRE_MAX_DIST) || (dist < VAMPIRE_MIN_DIST))				return false;

	if (object->CControlledActor::is_controlling())							return false;
	if (object->com_man().run_attack_is_active())							return false;

	const CActor *m_actor = smart_cast<const CActor*>(enemy);
	VERIFY(m_actor);
	if (m_actor->input_external_handler_installed())						return false;

	// проверить направление на врага
	if (!object->control().direction().is_face_target(enemy, PI_DIV_6))		return false;

	return true;
}

TEMPLATE_SPECIALIZATION
bool CStateBloodsuckerVampireExecuteAbstract::check_completion()
{
	return (m_action == eActionCompleted);
}

//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_prepare()
{
	object->com_man().ta_activate		(object->anim_triple_vampire);
	time_vampire_started				= Device.dwTimeGlobal;
	
	object->sound().play(CAI_Bloodsucker::eVampireGrasp);
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_continue()
{
	object->sound().play(CAI_Bloodsucker::eVampireSucking);

	// проверить на грави удар
	if (time_vampire_started + VAMPIRE_TIME_HOLD < Device.dwTimeGlobal) {
		m_action = eActionFire;
	}
}

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::execute_vampire_hit()
{
	object->com_man().ta_pointbreak				();
	object->sound().play						(CAI_Bloodsucker::eVampireHit);
	object->SatisfyVampire						();
}

//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateBloodsuckerVampireExecuteAbstract::look_head()
{
	CKinematics *pK = smart_cast<CKinematics*>(object->Visual());
	Fmatrix bone_transform;
	bone_transform = pK->LL_GetTransform(pK->LL_BoneID("bip01_head"));	

	Fmatrix global_transform;
	global_transform.mul_43(object->XFORM(),bone_transform);

	object->CControlledActor::look_point	(global_transform.c);
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateBloodsuckerVampireExecuteAbstract

