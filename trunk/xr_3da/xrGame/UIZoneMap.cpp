#include "stdafx.h"
#include "uizonemap.h"

#include "hudmanager.h"


#include "InfoPortion.h"
#include "Pda.h"

#include "Grenade.h"
#include "level.h"
#include "game_cl_base.h"

#include "actor.h"
#include "ai_space.h"
#include "game_graph.h"

#include "ui/UIMapWnd.h"
#include "ui/UIXmlInit.h"
//////////////////////////////////////////////////////////////////////////

#define				MAX_SCALE					4.0f
#define				MIN_SCALE					-4.0f

CUIZoneMap::CUIZoneMap()
{
	m_fScale  = 1.0f;
}

CUIZoneMap::~CUIZoneMap()
{
	
}

void CUIZoneMap::Init()
{

	CUIXml uiXml;
	bool xml_result			= uiXml.Init(CONFIG_PATH, UI_PATH, "zone_map.xml");
	R_ASSERT3(xml_result, "xml file not found", "zone_map.xml");

	// load map background
	CUIXmlInit xml_init;
	xml_init.InitStatic			(uiXml, "minimap:background", 0, &m_background);

	xml_init.InitStatic(uiXml, "minimap:background:level_frame", 0, &m_clipFrame);
	m_background.AttachChild(&m_clipFrame);

	xml_init.InitStatic(uiXml, "minimap:center", 0, &m_center);
	

	
	m_activeMap = xr_new<CUIMiniMap>();
	m_activeMap->SetAutoDelete(true);
	m_clipFrame.AttachChild(m_activeMap);
	m_activeMap->EnableHeading(true);  
	xml_init.InitStatic(uiXml, "minimap:compass", 0, &m_compass);
	m_compass.EnableHeading(true);
//	m_background.AttachChild(&m_compass);

	m_clipFrame.AttachChild(&m_center);
	m_center.SetWndPos(m_clipFrame.GetWidth()/2,m_clipFrame.GetHeight()/2);
}

void CUIZoneMap::Render			()
{
	m_background.Draw	();
	m_compass.Draw		();
}

void CUIZoneMap::SetHeading		(float angle)
{
	m_activeMap->SetHeading(angle);
	m_compass.SetHeading(angle);
};

void CUIZoneMap::UpdateRadar		(Fvector pos)
{
	m_background.Update();
	m_activeMap->SetActivePoint( pos );

	if(m_fScale>0)
		m_activeMap->SetZoomFactor( (float(m_clipFrame.GetWndRect().width())/100.0f) * m_fScale );
	else
		m_activeMap->SetZoomFactor( (float(m_clipFrame.GetWndRect().width())/100.0f) / _abs(m_fScale) );

	//Zoom factor static
	string256 buf;
	sprintf_s(buf, "%dx", int(m_fScale));
	m_clipFrame.SetText(buf);
	m_clipFrame.SetVTextAlignment(valBotton);
	m_clipFrame.SetFont(UI()->Font()->pFontMedium);
}

bool CUIZoneMap::ZoomIn()
{
	if(m_fScale<MAX_SCALE)
	{
		if(m_fScale == -2)
			m_fScale += 3;
		else
			m_fScale += 1;
		return true;
	}

	return false;
}
bool CUIZoneMap::ZoomOut()
{
	if(m_fScale>MIN_SCALE)
	{
		if(m_fScale == 1)
			m_fScale -= 3;
		else
			m_fScale -= 1;
		return true;
	}
	return false;
}

void CUIZoneMap::SetupCurrentMap()
{
	CInifile* pLtx				= pGameIni;

	if(!pLtx->section_exist(Level().name()))
		pLtx							= Level().pLevel;

	m_activeMap->Init(Level().name(),*pLtx,"hud\\default");

	Frect r;
	m_clipFrame.GetAbsoluteRect		(r);
	m_activeMap->SetClipRect		(r);
}
