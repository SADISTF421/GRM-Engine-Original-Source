#pragma once
#include "UIDragDropListEx.h"

class CUIOutfitDragDropList :public CUIDragDropListEx
{
	typedef CUIDragDropListEx						inherited;
	CUIStatic*										m_background;
	void					SetOutfit				(CUICellItem* itm);
public:
							CUIOutfitDragDropList	();
	virtual					~CUIOutfitDragDropList	();

	virtual void			SetItem					(CUICellItem* itm); //auto
	virtual void			SetItem					(CUICellItem* itm, Fvector2 abs_pos);  // start at cursor pos
	virtual void			SetItem					(CUICellItem* itm, Ivector2 cell_pos); // start at cell
	virtual CUICellItem*	RemoveItem				(CUICellItem* itm, bool force_root);
	virtual	void			Draw					();
	virtual	bool			OnMouse					(float x, float y, EUIMessages mouse_action);
};

