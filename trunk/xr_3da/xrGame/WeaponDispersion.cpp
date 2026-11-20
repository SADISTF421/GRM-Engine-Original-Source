// WeaponDispersion.cpp: разбос при стрельбе
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "weapon.h"
#include "inventoryowner.h"
#include "actor.h"
#include "inventory_item_impl.h"

#include "actoreffector.h"
#include "effectorshot.h"


//возвращает 1, если оружие в отличном состоянии и >1 если повреждено
float CWeapon::GetConditionDispersionFactor() const
{
	return (1.f + fireDispersionConditionFactor*(1.f-GetCondition()));
}

float CWeapon::GetFireDispersion	(bool with_cartridge) 
{
	return GetFireDispersion	(with_cartridge?m_fCurrentCartirdgeDisp:1.f);
}

//текущая дисперсия (в радианах) оружия с учетом используемого патрона
float CWeapon::GetFireDispersion	(float cartridge_k) 
{
	//учет базовой дисперсии, состояние оружия и влияение патрона
	float fire_disp = fireDispersionBase*cartridge_k*GetConditionDispersionFactor();

	// SHIZOMOD: held breath over the shoulder spreads the shots a bit less
	const CActor* steady_actor		= smart_cast<const CActor*>(H_Parent());
	if (steady_actor && steady_actor->tps_steady_aim())
		fire_disp					*= .8f;
	
	//вычислить дисперсию, вносимую самим стрелком
	const CInventoryOwner* pOwner	=	smart_cast<const CInventoryOwner*>(H_Parent());
	VERIFY (pOwner);

	float parent_disp = pOwner->GetWeaponAccuracy();
	fire_disp += parent_disp;

	return fire_disp;
}


//////////////////////////////////////////////////////////////////////////
// Для эффекта отдачи оружия
void CWeapon::AddShotEffector		()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
	{
		CEffectorShot* S	= smart_cast<CEffectorShot*>(pActor->Cameras().GetCamEffector(eCEShot)); 
		if (!S)	S				= (CEffectorShot*)pActor->Cameras().AddCamEffector(xr_new<CEffectorShot> (camMaxAngle,camRelaxSpeed, camMaxAngleHorz, camStepAngleHorz));
		R_ASSERT				(S);
		S->SetRndSeed(pActor->GetShotRndSeed());
		// SHIZOMOD: and kick a bit less
		S->Shot					(pActor->tps_steady_aim() ? camDispersion*.8f : camDispersion);
	}
}

void  CWeapon::RemoveShotEffector	()
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
		pActor->Cameras().RemoveCamEffector(eCEShot);
}

const Fvector& CWeapon::GetRecoilDeltaAngle()
{
	CActor* pActor		= smart_cast<CActor*>(H_Parent());

	CEffectorShot* S = NULL;
	if(pActor)
		S = smart_cast<CEffectorShot*>(pActor->Cameras().GetCamEffector(eCEShot)); 

	if(S)
	{
		S->GetDeltaAngle(m_vRecoilDeltaAngle);
		return m_vRecoilDeltaAngle;
	}
	else
	{
		m_vRecoilDeltaAngle.set(0.f,0.f,0.f);
		return m_vRecoilDeltaAngle;
	}
}