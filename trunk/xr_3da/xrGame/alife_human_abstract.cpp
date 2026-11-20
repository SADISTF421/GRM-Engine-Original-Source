////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_human_abstract.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife human abstract class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_objects_ALife_Monsters.h"
#include "alife_human_brain.h"
#include "alife_human_object_handler.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_group_registry.h"
#include "alife_object_registry.h"
#include "actor.h"

void CSE_ALifeHumanAbstract::update									()
{
	if (!bfActive())
		return;

	brain().update							();
	
	if (Actor()->HasInfo("offline_alife_active"))
		alife().check_for_interaction(this);
}

CSE_ALifeItemWeapon	*CSE_ALifeHumanAbstract::tpfGetBestWeapon(ALife::EHitType &tHitType, float &fHitPower)
{
	fHitPower					= 0.f;
	m_tpCurrentBestWeapon		= 0;
	u32							l_dwBestWeapon = 0;
	ALife::OBJECT_IT			I = children.begin();
	ALife::OBJECT_IT			E = children.end();
	for ( ; I != E; ++I) {
		CSE_ALifeItemWeapon		*l_tpALifeItemWeapon = smart_cast<CSE_ALifeItemWeapon*>(ai().alife().objects().object(*I));
		if (!l_tpALifeItemWeapon)
			continue;

		l_tpALifeItemWeapon->m_dwAmmoAvailable = get_available_ammo_count(l_tpALifeItemWeapon,children);
		if (l_tpALifeItemWeapon->m_dwAmmoAvailable || (!l_tpALifeItemWeapon->m_dwSlot) || (3 == l_tpALifeItemWeapon->m_dwSlot)) {
			u32					l_dwCurrentBestWeapon = l_tpALifeItemWeapon->ef_weapon_type(); 
			if (l_dwCurrentBestWeapon > l_dwBestWeapon) {
				l_dwBestWeapon = l_dwCurrentBestWeapon;
				m_tpCurrentBestWeapon = l_tpALifeItemWeapon;
			}
		}
	}
	if (m_tpCurrentBestWeapon) {
		fHitPower				= m_tpCurrentBestWeapon->m_fHitPower;
		tHitType				= m_tpCurrentBestWeapon->m_tHitType;
		return					(m_tpCurrentBestWeapon);
	}
	else
		return(inherited2::tpfGetBestWeapon(tHitType,fHitPower));
}

bool CSE_ALifeHumanAbstract::bfPerformAttack						()
{
	return									(brain().perform_attack());
}

u16	CSE_ALifeHumanAbstract::get_available_ammo_count(const CSE_ALifeItemWeapon *tpALifeItemWeapon, ALife::OBJECT_VECTOR &tpObjectVector)
{
	if (!tpALifeItemWeapon->m_caAmmoSections)
		return(u16(-1));
	u32							l_dwResult = 0;
	ALife::OBJECT_IT					I = tpObjectVector.begin();
	ALife::OBJECT_IT					E = tpObjectVector.end();
	for ( ; I != E; ++I) {
		CSE_ALifeItemAmmo		*l_tpALifeItemAmmo = smart_cast<CSE_ALifeItemAmmo*>(ai().alife().objects().object(*I));
		if (l_tpALifeItemAmmo && strstr(tpALifeItemWeapon->m_caAmmoSections,*l_tpALifeItemAmmo->s_name))
			l_dwResult			+= l_tpALifeItemAmmo->a_elapsed;
	}
	return						(u16(l_dwResult));
}

u16	CSE_ALifeHumanAbstract::get_available_ammo_count(const CSE_ALifeItemWeapon *tpALifeItemWeapon, ALife::ITEM_P_VECTOR &tpItemVector, ALife::OBJECT_VECTOR *tpObjectVector)
{
	if (!tpALifeItemWeapon->m_caAmmoSections)
		return(u16(-1));
	u32							l_dwResult = 0;
	ALife::ITEM_P_IT					I = tpItemVector.begin();
	ALife::ITEM_P_IT					E = tpItemVector.end();
	for ( ; I != E; ++I) {
		CSE_ALifeItemAmmo		*l_tpALifeItemAmmo = smart_cast<CSE_ALifeItemAmmo*>(*I);
		if (l_tpALifeItemAmmo && strstr(tpALifeItemWeapon->m_caAmmoSections,*l_tpALifeItemAmmo->s_name) && (l_tpALifeItemAmmo->m_dwCost <= brain().m_dwTotalMoney) && (!tpObjectVector || (std::find(tpObjectVector->begin(),tpObjectVector->end(),l_tpALifeItemAmmo->ID) == tpObjectVector->end()))) {
			l_dwResult			+= l_tpALifeItemAmmo->a_elapsed;
			brain().m_dwTotalMoney		-= l_tpALifeItemAmmo->m_dwCost;
		}
	}
	return						(u16(l_dwResult));
}

ALife::EMeetActionType	CSE_ALifeHumanAbstract::tfGetActionType(CSE_ALifeSchedulable *tpALifeSchedulable, int iGroupIndex, bool bMutualDetection)
{
	if (ALife::eCombatTypeMonsterMonster == ai().alife().combat_type()) {
		CSE_ALifeMonsterAbstract	*l_tpALifeMonsterAbstract = smart_cast<CSE_ALifeMonsterAbstract*>(tpALifeSchedulable);
		R_ASSERT2					(l_tpALifeMonsterAbstract,"Inconsistent meet action type");
		return						(ALife::eRelationTypeFriend == ai().alife().relation_type(this,smart_cast<CSE_ALifeMonsterAbstract*>(tpALifeSchedulable)) ? ALife::eMeetActionTypeIgnore : ((bMutualDetection || (ALife::eCombatActionAttack == alife().choose_combat_action(iGroupIndex))) ? ALife::eMeetActionTypeAttack : ALife::eMeetActionTypeIgnore));
	}
	else
		return(ALife::eMeetActionTypeAttack);
}

void CSE_ALifeHumanAbstract::vfDetachAll							(bool bFictitious)
{
	brain().objects().detach_all			(bFictitious);
}

void CSE_ALifeHumanAbstract::vfUpdateWeaponAmmo						()
{
	brain().objects().update_weapon_ammo	();
}

void CSE_ALifeHumanAbstract::vfProcessItems							()
{
	brain().objects().process_items			();
}

void CSE_ALifeHumanAbstract::vfAttachItems							(ALife::ETakeType tTakeType)
{
	brain().objects().attach_items			();
}

CSE_ALifeDynamicObject *CSE_ALifeHumanAbstract::tpfGetBestDetector	()
{
	return									(brain().objects().best_detector());
}

void CSE_ALifeHumanAbstract::on_register							()
{
	inherited2::on_register					();
	brain().on_register						();
	// because we need to load profile to setup graph vertex masks
	specific_character						();
}

void CSE_ALifeHumanAbstract::on_unregister							()
{
	brain().on_unregister					();
	if (m_group_id != 0xffff)
		ai().alife().groups().object(m_group_id).unregister_member	(ID);
}

void CSE_ALifeHumanAbstract::spawn_supplies							()
{
	specific_character			();
	inherited1::spawn_supplies	();
	inherited2::spawn_supplies	();
}

void CSE_ALifeHumanAbstract::add_online								(const bool &update_registries)
{
	CSE_ALifeTraderAbstract::add_online		(update_registries);
	brain().on_switch_online				();
}

void CSE_ALifeHumanAbstract::add_offline							(const xr_vector<ALife::_OBJECT_ID> &saved_children, const bool &update_registries)
{
	CSE_ALifeTraderAbstract::add_offline	(saved_children,update_registries);
	brain().on_switch_offline				();
}
