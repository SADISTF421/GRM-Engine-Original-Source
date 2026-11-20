#include "stdafx.h"
#include "UIOutfitSlot.h"
#include "UIStatic.h"
#include "UICellItem.h"
#include "../CustomOutfit.h"
#include "UIInventoryUtilities.h"
#include "../UICursor.h"

CUIOutfitDragDropList::CUIOutfitDragDropList()
{
	m_background				= xr_new<CUIStatic>();
	m_background->SetAutoDelete	(true);
	AttachChild					(m_background);
}

CUIOutfitDragDropList::~CUIOutfitDragDropList()
{
}

void CUIOutfitDragDropList::SetOutfit(CUICellItem* itm)
{
	static float fNoOutfitX				= pSettings->r_float("without_outfit", "full_scale_icon_x");
	static float fNoOutfitY				= pSettings->r_float("without_outfit", "full_scale_icon_y");
	
	Frect			r;
	r.x1			= fNoOutfitX*ICON_GRID_WIDTH;
	r.y1			= fNoOutfitY*ICON_GRID_HEIGHT;
	if(itm)
	{
		PIItem _iitem	= (PIItem)itm->m_pData;
		CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(_iitem); VERIFY(pOutfit);

		r.x1			= float(pOutfit->GetIconX())*ICON_GRID_WIDTH;
		r.y1			= float(pOutfit->GetIconY())*ICON_GRID_HEIGHT;
	}
	r.x2				= r.x1+CHAR_ICON_FULL_WIDTH*ICON_GRID_WIDTH;
	r.y2				= r.y1+CHAR_ICON_FULL_HEIGHT*ICON_GRID_HEIGHT;
	
	m_background->Init(0,0, GetWidth(), GetHeight());
	m_background->SetShader				(InventoryUtilities::GetCharIconsShader());

	m_background->SetOriginalRect		(r);

	m_background->TextureAvailable		(true);
	m_background->TextureOn				();
	m_background->RescaleRelative2Rect	(r);
	if(UI()->is_16_9_mode())
	{
		m_background->SetWndPos(0.0f,0.0f);
		m_background->SetWidth(GetWidth());
		m_background->SetHeight(GetHeight());
	}
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm)
{
	if(itm)	inherited::SetItem			(itm);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Fvector2 abs_pos)
{
	if(itm)	inherited::SetItem			(itm, abs_pos);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Ivector2 cell_pos)
{
	if(itm)	inherited::SetItem			(itm, cell_pos);
	SetOutfit							(itm);
}

CUICellItem* CUIOutfitDragDropList::RemoveItem(CUICellItem* itm, bool force_root)
{
	VERIFY								(!force_root);
	CUICellItem* ci						= inherited::RemoveItem(itm, force_root);
	SetOutfit							(NULL);
	return								ci;
}


void CUIOutfitDragDropList::Draw()
{
	m_background->Draw					();
//.	inherited::Draw						();
}

bool CUIOutfitDragDropList::OnMouse(float x, float y, EUIMessages mouse_action)
{
	bool b = inherited::OnMouse		(x,y,mouse_action);

	switch(mouse_action){
	case WINDOW_LBUTTON_DOWN:
			GetUICursor()->HoldMode(true);
			return true;
			break;
	case WINDOW_LBUTTON_UP:
			GetUICursor()->HoldMode(false);
			return true;
			break;
	}

	return b;
}