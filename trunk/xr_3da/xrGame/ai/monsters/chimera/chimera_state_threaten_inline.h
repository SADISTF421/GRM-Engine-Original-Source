#pragma once

#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CStateChimeraThreatenAbstract CStateChimeraThreaten<_Object>

#define MIN_DIST_TO_ENEMY	3.0f
#define MAX_DIST_TO_ENEMY	10.0f
#define THREATEN_DELAY		12000

TEMPLATE_SPECIALIZATION
CStateChimeraThreatenAbstract::CStateChimeraThreaten(_Object *obj) : inherited(obj)
{
}

TEMPLATE_SPECIALIZATION
void CStateChimeraThreatenAbstract::reinit()
{
	inherited::reinit();

	m_last_time_threaten	= 0;
}

TEMPLATE_SPECIALIZATION
void CStateChimeraThreatenAbstract::execute()
{
	object->set_action				(ACT_STAND_IDLE);
	object->anim().SetSpecParams	(ASP_THREATEN);
	object->set_state_sound			(MonsterSound::eMonsterSoundThreaten);
	object->dir().face_target		(object->EnemyMan.get_enemy(), 1200);

	m_last_time_threaten			= Device.dwTimeGlobal;
}

TEMPLATE_SPECIALIZATION
bool CStateChimeraThreatenAbstract::check_start_conditions()
{
	const CEntityAlive	*enemy = object->EnemyMan.get_enemy();	
	if (enemy->CLS_ID != CLSID_OBJECT_ACTOR)	return false;

	if (!object->EnemyMan.see_enemy_now())		return false;

	if (object->com_man().run_attack_is_active())	return false;

	if (object->com_man().jump_is_active())		return false;
	
	float dist = object->Position().distance_to(enemy->Position());
	if ((dist > MAX_DIST_TO_ENEMY) || (dist < MIN_DIST_TO_ENEMY))	return false;

	if (m_last_time_threaten + THREATEN_DELAY > Device.dwTimeGlobal) return false;

	// проверить направленность на цель
	float h,p;
	Fvector().sub(object->EnemyMan.get_enemy_position(), object->Position()).getHP(h,p);
	float my_h,my_p;
	object->Direction().getHP(my_h,my_p);

	if (angle_difference(h,my_h) > deg(10))	return false;

	return true;
}


#undef TEMPLATE_SPECIALIZATION
#undef CStateChimeraThreatenAbstract
