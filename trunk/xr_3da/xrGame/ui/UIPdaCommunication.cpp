//-----------------------------------------------------------------------------/
//  Коммуникационный поддиалог PDA
//-----------------------------------------------------------------------------/

#include "stdafx.h"
#include "UIPdaCommunication.h"
#include "UIPdaContactsWnd.h"

#include "../Pda.h"
#include "UIPdaWnd.h"

#include "UIXmlInit.h"

#include "../HUDManager.h"
#include "../actor.h"
#include "../level.h"
#include "../character_info.h"

#include "../PhraseDialog.h"

#define		PDA_XML					"pda.xml"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIPdaCommunication::CUIPdaCommunication()
{
	Show(false);

	m_pInvOwner = NULL;
	m_pPda = NULL;

	SetFont(HUD().Font().pFontMedium);
	UIPdaContactsWnd	= NULL;
	UIPdaDialogWnd		= NULL;

//	m_pOurDialogManager = m_pOthersDialogManager = NULL;

//	ToTopicMode();

//	m_bNeedToUpdateQuestions = false;
}

CUIPdaCommunication::~CUIPdaCommunication()
{
}

void CUIPdaCommunication::Init()
{
	CUIXml uiXml;
	uiXml.Init(CONFIG_PATH, UI_PATH,PDA_XML);
	CUIXmlInit xml_init;

	inherited::Init(0,0, float(Device.dwWidth), float(Device.dwHeight));

	//окно разговора по PDA
	UIPdaDialogWnd = xr_new<CUIPdaDialogWnd>(); UIPdaDialogWnd->SetAutoDelete(true);
	AttachChild(UIPdaDialogWnd);
	UIPdaDialogWnd->Init(0,0, GetWidth(), GetHeight());
	UIPdaDialogWnd->Show(false);

	//список контактов
	UIPdaContactsWnd = xr_new<CUIPdaContactsWnd>(); UIPdaContactsWnd->SetAutoDelete(true);
	AttachChild(UIPdaContactsWnd);
	UIPdaContactsWnd->Init();
	UIPdaContactsWnd->Show(false);

	//Элементы автоматического добавления
	xml_init.InitAutoStatic(uiXml, "auto_static", this);
}

void CUIPdaCommunication::InitPDACommunication()
{
	m_pInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	if(!m_pInvOwner) return;

	m_pPda = m_pInvOwner->GetPDA();

	if(!m_pPda)	return;

	InitPdaContacts();

	UIPdaDialogWnd->Show(false);
	UIPdaContactsWnd->Show(true);
}


void CUIPdaCommunication::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	if(pWnd == UIPdaContactsWnd)
	{
		if(msg == PDA_CONTACTS_WND_CONTACT_SELECTED)
		{
			UIPdaContactsWnd->Show(false);
			UIPdaDialogWnd->ContactRestore();

			InitPdaDialog();
			UIPdaDialogWnd->Show(true);
		}
	}
	else if(pWnd == UIPdaDialogWnd)
	{
		if(msg == CUIPdaDialogWnd::BACK_BUTTON_CLICKED)
		{
			UIPdaContactsWnd->Show(true);
			UIPdaDialogWnd->Show(false);
		}
	}
	inherited::SendMessage(pWnd, msg, pData);
}


void CUIPdaCommunication::Update()
{
	if(!m_pPda) return;

	//обновлять список контактов вне зависимости
	//от того показано окно контактов или нет
	UpdatePdaContacts();


	inherited::Update();
}

void CUIPdaCommunication::Draw()
{
	inherited::Draw();
}

void CUIPdaCommunication::Show(bool status)
{
	inherited::Show(status);
	if (status)
	{
		InitPDACommunication();
	}
}


/////////////////////////////////////////
// Функции работы с котактами
////////////////////////////////////////
void CUIPdaCommunication::InitPdaContacts()
{
	UIPdaContactsWnd->RemoveAll();

	xr_vector<CPda*> pda_list;
	m_pPda->ActivePDAContacts(pda_list);

	xr_vector<CPda*>::iterator it = pda_list.begin();

	for(; it!=pda_list.end();++it){
		UIPdaContactsWnd->AddContact(*it);
	}
}

//обновление списка активных контактов PDA
void CUIPdaCommunication::UpdatePdaContacts()
{
	xr_vector<CPda*> pda_list;
	m_pPda->ActivePDAContacts(pda_list);

	xr_vector<CPda*>::iterator it = pda_list.begin();

	for(; it!=pda_list.end();++it){
		CPda* pPda = (*it);
		if(!UIPdaContactsWnd->IsInList(pPda))
			UIPdaContactsWnd->RemoveContact(pPda);
	}
}

/////////////////////////////////////////
// Функции работы для поддержания диалога
////////////////////////////////////////

void CUIPdaCommunication::InitPdaDialog()
{
	m_idContact = UIPdaContactsWnd->GetContactID();

	m_pContactObject =  Level().Objects.net_Find(m_idContact);

	R_ASSERT2(m_pContactObject, "wrong ID");
	m_pOthersInvOwner = m_pContactInvOwner = smart_cast<CInventoryOwner*>(m_pContactObject);
	R_ASSERT2(m_pContactInvOwner, "can't cast to inventory owner");
	m_pContactPda = m_pContactInvOwner->GetPDA();
	VERIFY(m_pContactPda);

	m_pOurInvOwner = m_pInvOwner  = smart_cast<CInventoryOwner*>(Level().CurrentEntity());;
	R_ASSERT2(m_pInvOwner, "wrong inventory owner");

	//инициализировать окошко с информацией о собеседнике
	UIPdaDialogWnd->UICharacterInfo.InitCharacter(m_pContactInvOwner);

	xr_string		buf;
	string128		buf2;
	buf = UIPdaDialogWnd->UICharacterInfo.UIName().GetText();
	UIPdaDialogWnd->UICharIconHeader.UITitleText.SetText(buf.c_str());
	strconcat(sizeof(buf2),buf2, ALL_PDA_HEADER_PREFIX, PDA_CONTACTS_HEADER_SUFFIX, "/", buf.c_str());
	UIPdaDialogWnd->UIMsglogHeader.UITitleText.SetText(buf2);
}

void CUIPdaCommunication::Reload()
{
	UIPdaContactsWnd->Reload();
}
