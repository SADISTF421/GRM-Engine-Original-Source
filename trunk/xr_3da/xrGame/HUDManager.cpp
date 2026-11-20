#include "stdafx.h"
#include "HUDManager.h"
#include "hudtarget.h"

#include "actor.h"
#include "../igame_level.h"
#include "clsid_game.h"
#include "GamePersistent.h"
#include "../SkeletonAnimated.h"

// First-person legs drawn by the engine (ported from the co-op OGSR fork's
// player_hud local legs). A model, a cycle, a place and a yaw, set by script
// every frame: no object in the world, no AI, no spawn, no physics, no sounds.
// It stops being drawn as soon as the script stops setting it.
static IRender_Visual*	g_local_legs			= NULL;
static shared_str		g_local_legs_name;
static shared_str		g_local_legs_anim;
static Fmatrix			g_local_legs_xform;
static u32				g_local_legs_frame		= 0;
static bool				g_local_legs_visible	= false;

void set_local_legs(LPCSTR visual, LPCSTR animation, const Fvector& position, float yaw)
{
	g_local_legs_visible = false;
	if (!visual || !visual[0] || !animation || !animation[0])
		return;
	if (!g_local_legs || g_local_legs_name != visual)
	{
		if (g_local_legs)
			::Render->model_Delete(g_local_legs);
		g_local_legs		= ::Render->model_Create(visual);
		g_local_legs_name	= visual;
		g_local_legs_anim	= NULL;
	}
	CKinematicsAnimated* ka = smart_cast<CKinematicsAnimated*>(g_local_legs);
	if (!ka)
		return;
	if (g_local_legs_anim != animation)
	{
		MotionID m = ka->ID_Cycle_Safe(animation);
		if (!m.valid())
			return;
		ka->PlayCycle(m);
		g_local_legs_anim = animation;
	}
	g_local_legs_xform.setHPB(yaw, 0.f, 0.f);
	g_local_legs_xform.c.set(position);
	ka->UpdateTracks();
	ka->CalculateBones_Invalidate();
	ka->CalculateBones(TRUE);
	g_local_legs_frame		= Device.dwFrame;
	g_local_legs_visible	= true;
}

static void destroy_local_legs()
{
	if (g_local_legs)
		::Render->model_Delete(g_local_legs);
	g_local_legs			= NULL;
	g_local_legs_name		= NULL;
	g_local_legs_anim		= NULL;
	g_local_legs_visible	= false;
}


CFontManager::CFontManager()
{
	Device.seqDeviceReset.Add(this,REG_PRIORITY_HIGH);

	m_all_fonts.push_back(&pFontSmall				);// used cpp
	m_all_fonts.push_back(&pFontMedium				);// used cpp
	m_all_fonts.push_back(&pFontDI					);// used cpp
	m_all_fonts.push_back(&pFontHeaderEurope		);
	m_all_fonts.push_back(&pFontHeaderRussian		); // ???
	m_all_fonts.push_back(&pFontArial14				);// used xml
	m_all_fonts.push_back(&pArialN21Russian			);
	m_all_fonts.push_back(&pFontGraffiti19Russian	);
	m_all_fonts.push_back(&pFontGraffiti22Russian	);
	m_all_fonts.push_back(&pFontLetterica16Russian	);
	m_all_fonts.push_back(&pFontLetterica18Russian	);
	m_all_fonts.push_back(&pFontGraffiti32Russian	);
	m_all_fonts.push_back(&pFontGraffiti50Russian	);
	m_all_fonts.push_back(&pFontLetterica25			);
	m_all_fonts.push_back(&pFontStat				);

	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		(**it) = NULL;

	InitializeFonts();

}

void CFontManager::InitializeFonts()
{

	InitializeFont(pFontSmall				,"hud_font_small"				);
	InitializeFont(pFontMedium				,"hud_font_medium"				);
	InitializeFont(pFontDI					,"hud_font_di",					CGameFont::fsGradient|CGameFont::fsDeviceIndependent);
	InitializeFont(pFontHeaderEurope		,"ui_font_header_europe"		);
	InitializeFont(pFontHeaderRussian		,"ui_font_header_russian"		);
	InitializeFont(pFontArial14				,"ui_font_arial_14"				);
	InitializeFont(pArialN21Russian			,"ui_font_arial_21"	);
	InitializeFont(pFontGraffiti19Russian	,"ui_font_graffiti19_russian"	);
	InitializeFont(pFontGraffiti22Russian	,"ui_font_graffiti22_russian"	);
	InitializeFont(pFontLetterica16Russian	,"ui_font_letterica16_russian"	);
	InitializeFont(pFontLetterica18Russian	,"ui_font_letterica18_russian"	);
	InitializeFont(pFontGraffiti32Russian	,"ui_font_graff_32"				);
	InitializeFont(pFontGraffiti50Russian	,"ui_font_graff_50"				);
	InitializeFont(pFontLetterica25			,"ui_font_letter_25"			);
	InitializeFont(pFontStat				,"stat_font",					CGameFont::fsDeviceIndependent);

}

LPCSTR CFontManager::GetFontTexName (LPCSTR section)
{
	static char* tex_names[]={"texture800","texture","texture1600"};
	int def_idx		= 1;//default 1024x768
	int idx			= def_idx;

#if 0
	u32 w = Device.dwWidth;

	if(w<=800)		idx = 0;
	else if(w<=1280)idx = 1;
	else 			idx = 2;
#else
	u32 h = Device.dwHeight;

	if(h<=600)		idx = 0;
	else if(h<=900)	idx = 1;
	else 			idx = 2;
#endif

	while(idx>=0){
		if( pSettings->line_exist(section,tex_names[idx]) )
			return pSettings->r_string(section,tex_names[idx]);
		--idx;
	}
	return pSettings->r_string(section,tex_names[def_idx]);
}

void CFontManager::InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
	LPCSTR font_tex_name = GetFontTexName(section);
	R_ASSERT(font_tex_name);

	if(!F)
		F = xr_new<CGameFont> ("font", font_tex_name, flags);
	else
		F->Initialize("font",font_tex_name);

#ifdef DEBUG
	F->m_font_name = section;
#endif
	if (pSettings->line_exist(section,"size")){
		float sz = pSettings->r_float(section,"size");
		if (flags&CGameFont::fsDeviceIndependent)	F->SetHeightI(sz);
		else										F->SetHeight(sz);
	}
	if (pSettings->line_exist(section,"interval"))
	F->SetInterval(pSettings->r_fvector2(section,"interval"));

}

CFontManager::~CFontManager()
{
	Device.seqDeviceReset.Remove(this);
	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		xr_delete(**it);
}

void CFontManager::Render()
{
	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		(**it)->OnRender			();
}
void CFontManager::OnDeviceReset()
{
	InitializeFonts();
}

//--------------------------------------------------------------------
CHUDManager::CHUDManager()
{ 
	pUI						= 0;
	m_pHUDTarget			= xr_new<CHUDTarget>();
	OnDisconnected			();
}
//--------------------------------------------------------------------
CHUDManager::~CHUDManager()
{
	destroy_local_legs	();
	xr_delete			(pUI);
	xr_delete			(m_pHUDTarget);
	b_online			= false;
}

//--------------------------------------------------------------------

void CHUDManager::Load()
{
	if(pUI){
		pUI->Load			( pUI->UIGame() );
		return;
	}
	pUI					= xr_new<CUI> (this);
	pUI->Load			(NULL);
	OnDisconnected		();
}
//--------------------------------------------------------------------
void CHUDManager::OnFrame()
{
	if(!b_online)					return;
	if (pUI) pUI->UIOnFrame();
	m_pHUDTarget->CursorOnFrame();
}
//--------------------------------------------------------------------

ENGINE_API extern float psHUD_FOV;

void CHUDManager::Render_First()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT))return;
	if (0==pUI)						return;
	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (!A)							return;
	if (A && !A->HUDview())			return;

	// only shadow 
	::Render->set_Invisible			(TRUE);
	::Render->set_Object			(O->H_Root());
	O->renderable_Render			();
	::Render->set_Invisible			(FALSE);

	// first-person legs: world geometry, lit like the hero (see set_local_legs)
	if (g_local_legs_visible && g_local_legs && Device.dwFrame - g_local_legs_frame <= 2)
	{
		::Render->set_Object		(O->H_Root());
		::Render->set_Transform		(&g_local_legs_xform);
		::Render->add_Visual		(g_local_legs);
	}
}

void CHUDManager::Render_Last()
{
	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT))return;
	if (0==pUI)						return;
	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (A && !A->HUDview())			return;
	if(O->CLS_ID == CLSID_CAR)
		return;

	if(O->CLS_ID == CLSID_SPECTATOR)
		return;

	// hud itself
	::Render->set_HUD				(TRUE);
	::Render->set_Object			(O->H_Root());
	O->OnHUDDraw					(this);
	::Render->set_HUD				(FALSE);
}
void CHUDManager::Render_Actor_Shadow()	// added by KD
{
	if (0==pUI)						return;
	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (!A)							return;
	if (A->active_cam() != eacFirstEye) return;		// KD: we need to render actor shadow only in first eye cam mode because 
													// in other modes actor model already in scene graph and renders well
	::Render->set_Object			(O->H_Root());
	O->renderable_Render			();
}
extern void draw_wnds_rects();
extern ENGINE_API BOOL bShowPauseString;
//отрисовка элементов интерфейса
#include "string_table.h"
void  CHUDManager::RenderUI()
{
	if(!b_online)					return;

	BOOL bAlready					= FALSE;
	if (true || psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT))
	{
		HitMarker.Render			();
		bAlready					= ! (pUI && !pUI->Render());
		Font().Render();
	}

	if (psHUD_Flags.is(HUD_CROSSHAIR|HUD_CROSSHAIR_RT|HUD_CROSSHAIR_RT2) && !bAlready)	
		m_pHUDTarget->Render();

	draw_wnds_rects		();

	if( Device.Paused() && bShowPauseString){
		CGameFont* pFont	= Font().pFontGraffiti50Russian;
		pFont->SetColor		(0x80FF0000	);
		LPCSTR _str			= CStringTable().translate("st_game_paused").c_str();
		
		Fvector2			_pos;
		_pos.set			(UI_BASE_WIDTH/2.0f, UI_BASE_HEIGHT/2.0f);
		UI()->ClientToScreenScaled(_pos);
		pFont->SetAligment	(CGameFont::alCenter);
		pFont->Out			(_pos.x, _pos.y, _str);
		pFont->OnRender		();
	}

}

void CHUDManager::OnEvent(EVENT E, u64 P1, u64 P2)
{
}

collide::rq_result&	CHUDManager::GetCurrentRayQuery	() 
{
	return m_pHUDTarget->RQ;
}

void CHUDManager::SetCrosshairDisp	(float disp)
{	
	m_pHUDTarget->HUDCrosshair.SetDispersion(psHUD_Flags.test(HUD_CROSSHAIR_DYNAMIC) ? disp : 0.f);
}

void  CHUDManager::ShowCrosshair	(bool show)
{
	m_pHUDTarget->m_bShowCrosshair = show;
}


#include "ui\UIMainInGameWnd.h"
void CHUDManager::OnScreenRatioChanged()
{
	xr_delete							(pUI->UIMainIngameWnd);

	pUI->UIMainIngameWnd				= xr_new<CUIMainIngameWnd>	();
	pUI->UIMainIngameWnd->Init			();
	pUI->UnLoad							();
	pUI->Load							(pUI->UIGame());
}

void CHUDManager::OnDisconnected()
{
//.	if(!b_online)			return;
	b_online				= false;
	if(pUI)
		Device.seqFrame.Remove	(pUI);
}

void CHUDManager::OnConnected()
{
	if(b_online)			return;
	b_online				= true;
	if(pUI){
		Device.seqFrame.Add	(pUI,REG_PRIORITY_LOW-1000);
	}
}

void CHUDManager::net_Relcase	(CObject *object)
{
	VERIFY						(m_pHUDTarget);
	m_pHUDTarget->net_Relcase	(object);
}
