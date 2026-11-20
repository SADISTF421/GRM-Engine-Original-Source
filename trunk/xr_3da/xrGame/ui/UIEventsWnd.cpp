#include "pch_script.h"
#include "UIEventsWnd.h"
#include "UIFrameWindow.h"
#include "UIFrameLineWnd.h"
#include "UIAnimatedStatic.h"
#include "UIScrollView.h"
#include "UITabControl.h"
#include "UITaskDescrWnd.h"
#include "UI3tButton.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../actor.h"
#include "../gametaskmanager.h"
#include "../gametask.h"
#include "../map_manager.h"
#include "../map_location.h"
#include "../string_table.h"
#include "UITaskItem.h"
#include "../alife_registry_wrappers.h"
#include "../encyclopedia_article.h"
#include "UIInventoryUtilities.h"

CUIEventsWnd::CUIEventsWnd			()
{
	m_flags.zero			();
}

CUIEventsWnd::~CUIEventsWnd			()
{
	delete_data			(m_UITaskInfoWnd);
}

void CUIEventsWnd::Init				()
{
	CUIXml uiXml;
	bool xml_result					= uiXml.Init(CONFIG_PATH, UI_PATH, "pda_events.xml");
	R_ASSERT3						(xml_result, "xml file not found", "pda_events.xml");

	CUIXmlInit xml_init;

	inherited::Init(0,0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	AttachChild(&UITreeViewBg);
	xml_init.InitFrameWindow(uiXml, "right_frame_window", 0, &UITreeViewBg);

	UITreeViewBg.AttachChild(&UITreeViewHeader);
	xml_init.InitFrameLine(uiXml, "right_frame_line", 0, &UITreeViewHeader);

	UITreeViewHeader.AttachChild(&UIAnimation);
	xml_init.InitAnimatedStatic(uiXml, "a_static", 0, &UIAnimation);

	AttachChild(&UIFrameWnd);
	xml_init.InitFrameWindow(uiXml, "left_frame_window", 0, &UIFrameWnd);

	UIFrameWnd.AttachChild(&UIFrameWndHeader);
	xml_init.InitFrameLine(uiXml, "left_frame_line", 0, &UIFrameWndHeader);
	UIFrameWndHeader.UITitleText.SetElipsis(CUIStatic::eepBegin, 20);

	UIFrameWnd.AttachChild(&UIArticleCaption);
	xml_init.InitStatic(uiXml, "article_header_static", 0, &UIArticleCaption);

	m_UITaskInfoWnd					= xr_new<CUITaskDescrWnd>(); m_UITaskInfoWnd->SetAutoDelete(false);
	m_UITaskInfoWnd->Init			(&uiXml,"left_frame_window:task_descr_view");

	m_ListWnd						= xr_new<CUIScrollView>(); m_ListWnd->SetAutoDelete(true);
	UIFrameWnd.AttachChild			(m_ListWnd);
	xml_init.InitScrollView			(uiXml, "left_frame_window:list", 0, m_ListWnd);

	m_TaskFilter					= xr_new<CUITabControl>(); m_TaskFilter->SetAutoDelete(true);
	UITreeViewBg.AttachChild		(m_TaskFilter);
	xml_init.InitTabControl			(uiXml, "right_frame_window:filter_tab", 0, m_TaskFilter);
	m_TaskFilter->SetWindowName		("filter_tab");
	Register						(m_TaskFilter);
	AddCallback						("filter_tab",TAB_CHANGED,CUIWndCallback::void_function(this,&CUIEventsWnd::OnFilterChanged));

	// Кнопка возврата
	AttachChild(&UIBack);
	xml_init.InitButton(uiXml, "back_button", 0, &UIBack);

	// Autostatics
	xml_init.InitAutoStatic(uiXml, "left_auto_static", &UIFrameWnd)[2];
	xml_init.InitAutoStatic(uiXml, "right_auto_static", &UITreeViewBg);

	m_currFilter						= eActiveTask;
	SetDescriptionMode				(true);

	m_ui_task_item_xml.Init			(CONFIG_PATH, UI_PATH, "job_item.xml");
}

void CUIEventsWnd::Update			()
{
	if(m_flags.test(flNeedReload) ){
		ReloadList(false);
		m_flags.set(flNeedReload,FALSE );
	}

	if (m_currFilter==eActiveTask) {
		UIArticleCaption.SetText("Active Jobs");
		UIFrameWndHeader.UITitleText.SetText("#root 15/FD-665#68/Jobs/Current");
	} else if (m_currFilter==eAccomplishedTask) {
		UIArticleCaption.SetText("Completed Jobs");
		UIFrameWndHeader.UITitleText.SetText("#root 15/FD-665#68/Jobs/Accomplished");
	} else if (m_currFilter==eFailedTask) {
		UIArticleCaption.SetText("Failed Jobs");
		UIFrameWndHeader.UITitleText.SetText("#root 15/FD-665#68/Jobs/Failed");
	}

	inherited::Update		();
}

void CUIEventsWnd::Draw				()
{
	inherited::Draw			();
}

void	CUIEventsWnd::SendMessage			(CUIWindow* pWnd, s16 msg, void* pData)
{
	if (&UIBack == pWnd && BUTTON_CLICKED == msg)
	{
		SetDescriptionMode		(true);
	}

	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIEventsWnd::OnFilterChanged			(CUIWindow* w, void*)
{
	m_currFilter			=(ETaskFilters)m_TaskFilter->GetActiveIndex();
	ReloadList				(false);
	if(!GetDescriptionMode())
		SetDescriptionMode		(true);
}

void CUIEventsWnd::Reload					()
{
		m_flags.set(flNeedReload,TRUE );
}

void CUIEventsWnd::ReloadList(bool bClearOnly)
{
	m_ListWnd->Clear			();
	if(bClearOnly)				return;

	if(!g_actor)				return;
	GameTasks& tasks			= Actor()->GameTaskManager().GameTasks();
	GameTasks::iterator it		= tasks.begin();
	CGameTask* task				= NULL;
	
	for(;it!=tasks.end();++it)
	{
		task					= (*it).game_task;
		R_ASSERT				(task);
		R_ASSERT				(task->m_Objectives.size() > 0);

		if( !Filter(task) )		continue;
		CUITaskItem* pTaskItem	= NULL;
/*
		if(task->m_Objectives[0].TaskState()==eTaskUserDefined)
		{
			VERIFY				(task->m_Objectives.size()==1);
			pTaskItem			= xr_new<CUIUserTaskItem>(this);
			pTaskItem->SetGameTask			(task, 0);
			m_ListWnd->AddWindow			(pTaskItem,true);
		}else
*/
		for (u16 i = 0; i < task->m_Objectives.size(); ++i)
		{
			if(i==0){
				pTaskItem					= xr_new<CUITaskRootItem>(this);
			}else{
				pTaskItem					= xr_new<CUITaskSubItem>(this);
			}
			pTaskItem->SetGameTask			(task, i);
			m_ListWnd->AddWindow			(pTaskItem,true);
		}

	}

}

void CUIEventsWnd::Show(bool status)
{
	inherited::Show			(status);
	m_UITaskInfoWnd->Show	(status);

	if (status)
		InventoryUtilities::SendInfoToActor("ui_pda_jobs");
	else
		InventoryUtilities::SendInfoToActor("ui_pda_jobs_hide");

	ReloadList				(status == false);
}

bool CUIEventsWnd::Filter(CGameTask* t)
{
	ETaskState task_state		= t->m_Objectives[0].TaskState();
//	bool bprimary_only			= m_primary_or_all_filter_btn->GetCheck();

	return (false/*m_currFilter==eOwnTask && task_state==eTaskUserDefined*/ )		||
			( 
			  ( true/*!bprimary_only || (bprimary_only && t->m_is_task_general)*/ )	&&
				(
					(m_currFilter==eAccomplishedTask	&& task_state==eTaskStateCompleted )||
					(m_currFilter==eFailedTask			&& task_state==eTaskStateFail )||
					(m_currFilter==eActiveTask			&& task_state==eTaskStateInProgress )
				)
			);
}


void CUIEventsWnd::SetDescriptionMode		(bool bList)
{
	if(bList){
		UIFrameWnd.DetachChild		(m_UITaskInfoWnd);
		m_ListWnd->Show(true);
		UIBack.Show(false);
	}else{
		m_ListWnd->Show(false);
		UIFrameWnd.AttachChild		(m_UITaskInfoWnd);
		UIBack.Show(true);
	}
	m_flags.set(flListMode, bList);
}

bool CUIEventsWnd::GetDescriptionMode		()
{
	return !!m_flags.test(flListMode);
}

void CUIEventsWnd::ShowDescription			(CGameTask* t, int idx)
{
	if(!GetDescriptionMode()){//articles
		SGameTaskObjective& o		= t->Objective(0);
		idx							= 0;

		m_UITaskInfoWnd->ClearAll	();

		if(Actor()->encyclopedia_registry->registry().objects_ptr())
		{
			string512	need_group;
			if(0==idx){
				strcpy(need_group,*t->m_ID);
			}else
			if(o.article_key.size())
			{
				sprintf_s(need_group, "%s/%s", *t->m_ID, *o.article_key);
			}else
			{
				sprintf_s(need_group, "%s/%d", *t->m_ID, idx);
			}

			ARTICLE_VECTOR::const_iterator it		= Actor()->encyclopedia_registry->registry().objects_ptr()->begin();

			for(; it != Actor()->encyclopedia_registry->registry().objects_ptr()->end(); ++it)
			{
				if (ARTICLE_DATA::eTaskArticle == it->article_type)
				{
					CEncyclopediaArticle	A;
					A.Load					(it->article_id);

					const shared_str& group = A.data()->group;

					if( strstr(group.c_str(), need_group)== group.c_str() )
					{
						u32 sz			= xr_strlen(need_group);
						if ( group.size()== sz || group.c_str()[sz]=='/' )
							m_UITaskInfoWnd->AddArticle(&A);
					}else
					if(o.article_id.size() && it->article_id ==o.article_id)
					{
						CEncyclopediaArticle			A;
						A.Load							(it->article_id);
						m_UITaskInfoWnd->AddArticle		(&A);
					}
				}
			}
		}
	}

	int sz			= m_ListWnd->GetSize		();

	for(int i=0; i<sz;++i)
	{
		CUITaskItem* itm			= (CUITaskItem*)m_ListWnd->GetItem(i);

		if((itm->GameTask()==t) && (itm->ObjectiveIdx()==idx) )	
			itm->MarkSelected		(true);
		else
			itm->MarkSelected		(false);
	}
}

bool CUIEventsWnd::ItemHasDescription(CUITaskItem* itm)
{
	if(itm->ObjectiveIdx()==0)// root
	{
		bool bHasLocation	= itm->GameTask()->HasLinkedMapLocations();
		return bHasLocation;
	}else
	{
		SGameTaskObjective	*obj				= itm->Objective();
		CMapLocation* ml						= obj->LinkedMapLocation();
		bool bHasLocation						= (NULL != ml);
		bool bIsMapMode							= GetDescriptionMode(); 
		bool b									= (bIsMapMode&&bHasLocation&&ml->SpotEnabled());
		return b;
	}
}
void CUIEventsWnd::Reset()
{
	inherited::Reset	();
	Reload				();
}
