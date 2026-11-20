#ifndef _EFFECTOR_BOBBING_H
#define _EFFECTOR_BOBBING_H
#pragma once

#include "CameraEffector.h"
#include "../cameramanager.h"

class CEffectorBobbing : public CEffectorCam  
{
	float	fTime;
	Fvector	vAngleAmplitude;
	float	fYAmplitude;
	float	fSpeed;

	u32		dwMState;
	float	fReminderFactor;
	bool	is_limping;
	bool	m_bZoomMode;

	float	m_fAmplitudeRun;
	float	m_fAmplitudeWalk;
	float	m_fAmplitudeLimp;

	float	m_fSpeedRun;
	float	m_fSpeedWalk;
	float	m_fSpeedLimp;

	// The head sways with the steps and leans into a strafe, as a body does.
	// [bobbing_effector]: strafe_roll (deg), strafe_roll_speed, side_sway (m).
	float	m_fStrafeRoll;
	float	m_fStrafeRollSpeed;
	float	m_fSideSway;
	float	fRoll;

public:
			CEffectorBobbing	();
	virtual ~CEffectorBobbing	();
	virtual	BOOL	Process		(Fvector &delta_p, Fvector &delta_d, Fvector &delta_n, float& fFov, float& fFar, float& fAspect);
	void	SetState			(u32 st, bool limping, bool ZoomMode);
};

#endif //_EFFECTOR_BOBBING_H
