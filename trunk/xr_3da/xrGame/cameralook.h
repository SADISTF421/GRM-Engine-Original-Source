#pragma once

#include "../CameraBase.h"

class CCameraLook	: public CCameraBase
{
	typedef CCameraBase inherited;

	Fvector2		lim_zoom;
	float			dist, prev_d;
public:
					CCameraLook		( CObject* p, u32 flags=0);
	virtual			~CCameraLook	( );
	virtual void	Load			(LPCSTR section);
	virtual void	Move			( int cmd, float val=0, float factor=1.0f );

	virtual	void	OnActivate		( CCameraBase* old_cam );
	virtual void	Update			( Fvector& point, Fvector& noise_dangle );

	virtual float	GetWorldYaw		( )	{ return -yaw;	};
	virtual float	GetWorldPitch	( )	{ return pitch; };
};

// SHIZOMOD: third person over the shoulder, as shooters do it. The camera hangs on a boom from
// the shoulder, follows where you look, slides in front of walls instead of through them, and
// comes closer while aiming. The old PSP auto-aim on Shift is gone.
class CCameraLook2	: public CCameraLook
{
public:
	static Fvector	m_cam_offset;		// right, up, along the view (negative is behind)
protected:
	float			m_boom;				// current length of the boom, smoothed
	bool			m_boom_valid;
public:
					CCameraLook2	( CObject* p, u32 flags=0):CCameraLook(p, flags){m_boom=0.f; m_boom_valid=false;};
	virtual			~CCameraLook2	(){}
	virtual	void	OnActivate		( CCameraBase* old_cam );
	virtual void	Update			( Fvector& point, Fvector& noise_dangle );
	virtual void	Load			(LPCSTR section);
};
