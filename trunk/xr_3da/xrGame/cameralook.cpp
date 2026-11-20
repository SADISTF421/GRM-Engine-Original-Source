#include "stdafx.h"
#pragma hdrstop

#include "CameraLook.h"
#include "../Cameramanager.h"
#include "xr_level_controller.h"
#include "actor.h"

CCameraLook::CCameraLook(CObject* p, u32 flags ) 
:CCameraBase(p, flags)
{
}

void CCameraLook::Load(LPCSTR section)
{
	inherited::Load		(section);
	style				= csLookAt;
	lim_zoom			= pSettings->r_fvector2	(section,"lim_zoom");
	dist				= (lim_zoom[0]+lim_zoom[1])*0.5f;
	prev_d				= 0;
}

CCameraLook::~CCameraLook()
{
}

void CCameraLook::Update(Fvector& point, Fvector& /**noise_dangle/**/)
{
	vPosition.set		(point);
	Fmatrix mR;
	mR.setHPB			(-yaw,-pitch,-roll);

	vDirection.set		(mR.k);
	vNormal.set			(mR.j);

	if (m_Flags.is(flRelativeLink)){
		parent->XFORM().transform_dir(vDirection);
		parent->XFORM().transform_dir(vNormal);
	}
	Fvector				vDir;
	collide::rq_result	R;

	float				covariance = VIEWPORT_NEAR*6.f;
	vDir.invert			(vDirection);
	g_pGameLevel->ObjectSpace.RayPick( point, vDir, dist+covariance, collide::rqtBoth, R, parent);

	float d				= psCamSlideInert*prev_d+(1.f-psCamSlideInert)*(R.range-covariance);
	prev_d = d;
	
	vPosition.mul		(vDirection,-d-VIEWPORT_NEAR);
	vPosition.add		(point);

}

void CCameraLook::Move( int cmd, float val, float factor)
{
	switch (cmd){
	case kCAM_ZOOM_IN:	dist	-= val?val:(rot_speed.z*Device.fTimeDelta);	break;
	case kCAM_ZOOM_OUT:	dist	+= val?val:(rot_speed.z*Device.fTimeDelta);	break;
	case kDOWN:			pitch	-= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	case kUP:			pitch	+= val?val:(rot_speed.x*Device.fTimeDelta/factor);	break;
	case kLEFT:			yaw		-= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	case kRIGHT:		yaw		+= val?val:(rot_speed.y*Device.fTimeDelta/factor);	break;
	}
	if (bClampYaw)		clamp(yaw,lim_yaw[0],lim_yaw[1]);
	if (bClampPitch)	clamp(pitch,lim_pitch[0],lim_pitch[1]);
	clamp			(dist,lim_zoom[0],lim_zoom[1]);	
}

void CCameraLook::OnActivate( CCameraBase* old_cam )
{
	if (old_cam&&(m_Flags.is(flRelativeLink)==old_cam->m_Flags.is(flRelativeLink)))
	{
		yaw				= old_cam->yaw;
		vPosition.set	(old_cam->vPosition);
	}
	if (yaw>PI_MUL_2) yaw-=PI_MUL_2;
	if (yaw<-PI_MUL_2)yaw+=PI_MUL_2;
}

#include "weapon.h"
#include "inventory.h"

Fvector CCameraLook2::m_cam_offset;

void CCameraLook2::OnActivate( CCameraBase* old_cam )
{
	CCameraLook::OnActivate	( old_cam );
	// keep looking where the eyes looked, up and down as well
	if (old_cam && (m_Flags.is(flRelativeLink) == old_cam->m_Flags.is(flRelativeLink))) {
		pitch				= old_cam->pitch;
		if (bClampPitch)
			clamp			(pitch, lim_pitch[0], lim_pitch[1]);
	}
	// start the boom fully out and let the wall check pull it in on the first frame
	m_boom_valid			= false;
}

void CCameraLook2::Update(Fvector& point, Fvector&)
{
	Fmatrix mR;
	mR.setHPB						(-yaw,-pitch,-roll);

	vDirection.set					(mR.k);
	vNormal.set						(mR.j);

	// where the boom is mounted: the shoulder, beside the eyes
	Fmatrix							yaw_xform;
	yaw_xform.setXYZ				(0, -yaw, 0);
	Fvector							right = yaw_xform.i;
	right.normalize_safe			();

	Fvector							pivot;
	pivot.set						(point.x, point.y + m_cam_offset.y, point.z);

	Fvector							shoulder;
	shoulder.mad					(pivot, right, m_cam_offset.x);

	// the shoulder may itself be in a wall when standing sideways to one
	collide::rq_result				R;
	{
		Fvector						dir;
		dir.sub						(shoulder, pivot);
		float						len = dir.magnitude();
		if (len > EPS_L) {
			dir.div					(len);
			if (g_pGameLevel->ObjectSpace.RayPick(pivot, dir, len + VIEWPORT_NEAR, collide::rqtStatic, R, parent))
				shoulder.mad		(pivot, dir, _max(R.range - VIEWPORT_NEAR, 0.f));
		}
	}

	// the boom runs back along the view; aiming brings the camera closer
	float							boom_length = _abs(m_cam_offset.z);
	CActor*							actor = smart_cast<CActor*>(parent);
	if (actor) {
		CWeapon*					weapon = smart_cast<CWeapon*>(actor->inventory().ActiveItem());
		if (weapon && weapon->IsZoomed())
			boom_length				*= .55f;
	}

	Fvector							back;
	back.invert						(vDirection);
	if (m_cam_offset.z > 0.f)
		back.set					(vDirection);

	float							target = boom_length;
	if (g_pGameLevel->ObjectSpace.RayPick(shoulder, back, boom_length + VIEWPORT_NEAR*2.f, collide::rqtStatic, R, parent))
		target						= _max(R.range - VIEWPORT_NEAR*2.f, 0.f);

	// in at once (never look through the wall), out smoothly (no jerk when the wall is passed)
	if (!m_boom_valid || (target < m_boom))
		m_boom						= target;
	else {
		m_boom						+= (target - m_boom)*_min(1.f, Device.fTimeDelta*6.f);
	}
	m_boom_valid					= true;

	vPosition.mad					(shoulder, back, m_boom);
}

void CCameraLook2::Load(LPCSTR section)
{
	CCameraLook::Load		(section);
	// "tps_offset" if the section has one; the old PSP "offset" sat the camera half a metre
	// behind the head, so it is not read
	m_cam_offset			= READ_IF_EXISTS(pSettings, r_fvector3, section, "tps_offset", Fvector().set(.5f, .1f, -1.7f));
}
