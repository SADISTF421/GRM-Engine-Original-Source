//-----------------------------------------------------------------------------/
//  Коммуникационный поддиалог PDA
//-----------------------------------------------------------------------------/

#ifndef UI_PDA_COMMUNICATION_H_
#define UI_PDA_COMMUNICATION_H_

#include "UIPdaContactsWnd.h"
#include "UIPdaDialogWnd.h"

#include "UIDialogWnd.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "UIEditBox.h"
#include "UIListWnd.h"
#include "UIFrameWindow.h"

#include "../PhraseDialogDefs.h"



class CInventoryOwner;
class CActor;
class CPhraseDialogManager;
class CUIPdaContactsWnd;
class CPda;
///////////////////////////////////////
// Подложка и основные кнопки PDA
///////////////////////////////////////

class CUIPdaCommunication: public CUIWindow
{
private:
	typedef CUIWindow inherited;
public:
	CUIPdaCommunication();
	virtual ~CUIPdaCommunication();

			void Init();
			void InitPDACommunication();

	virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void Draw();
	virtual void Update();

	virtual void Show(bool status);

protected:
	//контакты
			void InitPdaContacts();
			void UpdatePdaContacts();

	//диалог
	virtual void InitPdaDialog();
public:
	void Reload();
protected:

//	CUIFrameWindow		UIMainPdaFrame;
	CUIPdaContactsWnd*	UIPdaContactsWnd;
	CUIPdaDialogWnd*	UIPdaDialogWnd;

	//указатель на владельца инвентаря вызвавшего менюшку
	CInventoryOwner*	m_pInvOwner;
	CPda*				m_pPda;

	//указатели на текущий контакт
	u32					m_idContact;
	CObject*			m_pContactObject;
	CInventoryOwner*	m_pContactInvOwner;
	CPda*				m_pContactPda;

	//потеря контакта во время диалога
//	bool m_bContactLoss;


	//указатель на владельца инвентаря вызвавшего менюшку
	//и его собеседника
	CActor*			 m_pActor;
	CInventoryOwner* m_pOurInvOwner;
	CInventoryOwner* m_pOthersInvOwner;

	//диалог
	void InitTalkDialog();
};

#endif