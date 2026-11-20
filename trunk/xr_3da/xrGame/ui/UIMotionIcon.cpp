#include "stdafx.h"
#include "UIMainIngameWnd.h"
#include "UIMotionIcon.h"
#include "UIXmlInit.h"
const LPCSTR MOTION_ICON_XML = "motion_icon.xml";

CUIMotionIcon::CUIMotionIcon()
{
	m_curren_state	= stLast;
}

CUIMotionIcon::~CUIMotionIcon()
{

}
void CUIMotionIcon::Init()
{
	CUIXml uiXml;
	bool result = uiXml.Init(CONFIG_PATH, UI_PATH, MOTION_ICON_XML);
	R_ASSERT3(result, "xml file not found", MOTION_ICON_XML);

	CUIXmlInit	xml_init;

	xml_init.InitStatic			(uiXml, "background", 0, this);

	AttachChild					(&m_luminosity_progress);
	xml_init.InitProgressBar	(uiXml, "luminosity_progress", 0, &m_luminosity_progress);	

	AttachChild					(&m_noise_progress);
	xml_init.InitProgressBar	(uiXml, "noise_progress", 0, &m_noise_progress);	
	
	////////////////////////////////////////////////////////////////////
//	AttachChild(&m_states[stNormal]);
//	xml_init.InitStatic(uiXml, "normal", 0, &m_states[stNormal]);

	AttachChild(&m_PowerBar[stNormal]);
	xml_init.InitProgressBar(uiXml, "power_bar_normal", 0, &m_PowerBar[stNormal]);	
	////////////////////////////////////////////////////////////////////
//	AttachChild(&m_states[stCrouch]);
//	xml_init.InitStatic(uiXml, "crouch", 0, &m_states[stCrouch]);

	AttachChild(&m_PowerBar[stCrouch]);
	xml_init.InitProgressBar(uiXml, "power_bar_crouch", 0, &m_PowerBar[stCrouch]);
	////////////////////////////////////////////////////////////////////
//	AttachChild(&m_states[stCreep]);
//	xml_init.InitStatic(uiXml, "creep", 0, &m_states[stCreep]);

	AttachChild(&m_PowerBar[stCreep]);
	xml_init.InitProgressBar(uiXml, "power_bar_creep", 0, &m_PowerBar[stCreep]);
	////////////////////////////////////////////////////////////////////
//	AttachChild(&m_states[stClimb]);
//	xml_init.InitStatic(uiXml, "climb", 0, &m_states[stClimb]);

	AttachChild(&m_PowerBar[stClimb]);
	xml_init.InitProgressBar(uiXml, "power_bar_normal", 0, &m_PowerBar[stClimb]);
	////////////////////////////////////////////////////////////////////

	Show(true);
	Enable(true);

	for(int i=stNormal;i!=stLast;++i)
	{
//		m_states[i].Show(false);
//		m_states[i].Enable(false);
		m_PowerBar[i].Show(false);
		m_PowerBar[i].Enable(false);
	}
	ShowState					(stNormal);
}

void CUIMotionIcon::ShowState(EState state)
{
	if(m_curren_state==state)			return;
	if(m_curren_state!=stLast)
	{
	
		m_PowerBar[m_curren_state].Show(false);
		m_PowerBar[m_curren_state].Enable(false);
	}
	m_PowerBar[state].Show(true);
	m_PowerBar[state].Enable(true);

	m_curren_state=state;
}

void CUIMotionIcon::SetPower(float Pos)
{
    m_PowerBar[m_curren_state].SetProgressPos(Pos);
}

void CUIMotionIcon::SetNoise(float Pos)
{
	Pos	= clampr(Pos, m_noise_progress.GetRange_min(), m_noise_progress.GetRange_max());
	m_noise_progress.SetProgressPos(Pos);
}

void CUIMotionIcon::SetLuminosity(float Pos)
{
	Pos						= clampr(Pos, m_luminosity_progress.GetRange_min(), m_luminosity_progress.GetRange_max());
	m_luminosity_progress.SetProgressPos(Pos);
}