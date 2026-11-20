#pragma once
#include "UIWindow.h"
#include "UIWndCallback.h"
#include "UIXmlInit.h"
#include "UIFrameWindow.h"
#include "UIFrameLineWnd.h"
#include "UIAnimatedStatic.h"
#include "UIButton.h"

class CUITabControl;
class CGameTask;
class CUITaskDescrWnd;
class CUIScrollView;
class CUITaskItem;

class CUIEventsWnd	:public CUIWindow, public CUIWndCallback{
	typedef CUIWindow			inherited;
	enum ETaskFilters{	eActiveTask			=	0,
						eAccomplishedTask,
						eFailedTask,
//.						eOwnTask,
						eMaxTask};
	enum EEventWndFlags{
						flNeedReload	=(1<<0),
						flListMode		=(1<<1),
	};
	Flags16						m_flags;
	ETaskFilters				m_currFilter;
	CUITaskDescrWnd*			m_UITaskInfoWnd;
	CUIScrollView*				m_ListWnd;
	CUITabControl*				m_TaskFilter;

	// Подложка под TreeView
	CUIFrameWindow		UITreeViewBg;
	CUIFrameLineWnd		UITreeViewHeader;
	// Анимационная иконка
	CUIAnimatedStatic	UIAnimation;

	// Подложка под основное поле
	CUIFrameWindow		UIFrameWnd;
	// Хидер основного поля
	CUIFrameLineWnd		UIFrameWndHeader;
	// Заголовок отображаемого в основном поле текущего раздела
	CUIStatic			UIArticleCaption;
	// Кнопка возврата в меню заданий
	CUIButton			UIBack;
	bool						Filter					(CGameTask* t);
	void __stdcall				OnFilterChanged			(CUIWindow*,void*);
	void						ReloadList				(bool bClearOnly);

public:
	void						SetDescriptionMode		(bool bList);
	bool						GetDescriptionMode		();
	void						ShowDescription			(CGameTask* t, int idx);
	bool						ItemHasDescription		(CUITaskItem*);
public:

								CUIEventsWnd			();
	virtual						~CUIEventsWnd			();
	virtual void				SendMessage				(CUIWindow* pWnd, s16 msg, void* pData);
			void				Init					();
	virtual void				Update					();
	virtual void				Draw					();
	virtual void				Show					(bool status);
			void				Reload					();
	virtual void				Reset					();

	CUIXml						m_ui_task_item_xml;

};
