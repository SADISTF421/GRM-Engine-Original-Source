#pragma once

#include "psy_dog.h"


#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CPseudodogGrowlingAbstract CPseudodogGrowling<_Object>

#define PSY_HIT_TIME_COOLDOWN	8000
#define PSY_HIT_MIN_DIST		3.0f
#define PSY_HIT_MAX_DIST		8.0f

TEMPLATE_SPECIALIZATION
CPseudodogGrowlingAbstract::CPseudodogGrowling(_Object *obj) : inherited(obj)
{
}

TEMPLATE_SPECIALIZATION
void CPseudodogGrowlingAbstract::reinit()
{
	inherited::reinit();

	m_time_last_psy_hit	= 0;
}

TEMPLATE_SPECIALIZATION
void CPseudodogGrowlingAbstract::execute()
{
	object->set_action				(ACT_STAND_IDLE);
	object->anim().SetSpecParams	(ASP_PSI_ATTACK);
	object->sound().play			(CAI_PseudoDog::ePsyAttack);
	object->play_effect_sound		();

	m_time_last_psy_hit				= Device.dwTimeGlobal;
}

TEMPLATE_SPECIALIZATION
bool CPseudodogGrowlingAbstract::check_start_conditions()
{
	const CEntityAlive	*enemy = object->EnemyMan.get_enemy();	
	if (enemy->CLS_ID != CLSID_OBJECT_ACTOR)	return false;
	
	if (!object->allow_psy_hit())				return false;

	CPsyDog *psy_dog = smart_cast<CPsyDog*>(object);
	CPsyDogPhantom *psy_dog_phantom = smart_cast<CPsyDogPhantom*>(object);
	if (psy_dog || psy_dog_phantom)				return false;
	
	if (!object->EnemyMan.see_enemy_now())		return false;

	if (object->com_man().jump_is_active())		return false;

	float dist = object->Position().distance_to(enemy->Position());
	if ((dist > PSY_HIT_MAX_DIST) || (dist < PSY_HIT_MIN_DIST))	return false;

	if (m_time_last_psy_hit + PSY_HIT_TIME_COOLDOWN > Device.dwTimeGlobal) return false;

	// проверить направленность на цель
	float h,p;
	Fvector().sub(object->EnemyMan.get_enemy_position(), object->Position()).getHP(h,p);
	float my_h,my_p;
	object->Direction().getHP(my_h,my_p);

	if (angle_difference(h,my_h) > deg(10))	return false;

	return true;
}

#undef TEMPLATE_SPECIALIZATION
#undef CPseudodogGrowlingAbstract
