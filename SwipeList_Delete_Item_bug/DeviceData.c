#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "GUI.h"
#include "DIALOG.h"

#define USER_ID_TEXT (GUI_ID_USER + 0)
//
//���º궨�����separator����ͨItem��type��Ҫ�����ڴ���node��ʱ��
//
#define NODE_TYPE_SEP	0
#define NODE_TYPE_ITE	1
#define ITEM_SIZE		40
#define SEP_SIZE		1
//
//ö�ٳ�Item������
//
typedef enum {
	SEPA_TYPE =1,
	ITEM_TYPE
}ItemType_e;
//
//���� Item�����ݽṹ
//
struct ItemInfo_t {
	ItemType_e	ItemType;
	U8	Focus;
	U8	Temp;
	char ItemName[16];
	U16 ItemIndex;
	U32 hAllocSpace;
	GUI_HWIN hAttachWin; //ÿ��Item(��separator֮���Item) �����ŵĴ��ڡ�
						 //�Ƿ���Ҫָ��Item������SepartorItem �Ի����Index? �Ա� ֪����Item���ĸ�֮��
	struct SepaInfo_t * pParentSepa;
	struct ItemInfo_t * pItemNext;
};
typedef struct ItemInfo_t Item_t;

//typedef struct {
//	Item_t * pItemHeader;
//}ItemHeader_t;
//
//���� Separator Item�����ݽṹ
//
struct SepaInfo_t {
	ItemType_e	ItemType;
	char SepaName[16];
	U16 SepaIndex;		//separator������
	//U16 ItemCnt;		//�����ж��ٸ�Item
	U32	hAllocSpace;
	Item_t * pItemHead; //Item������ͷ
	struct SepaInfo_t * pSepNext;
};
typedef  struct SepaInfo_t Separator_t;

typedef struct {
	Separator_t * pSepaHeader;
}Header_t;

char SepName[][16] = {
	"Living room",
	"Bed room",
	"Dining room",
	"Child room"
};

static BUTTON_Handle _ahText[11];
static int Visable;
static Header_t Header;
static unsigned int LastFocus;
static int Traversal;

/*********************************************************************
*
*       AddNode
*   ���һ��� ���ڵ����ռ�
* @parm
*	NodeType :�ڵ�����ͣ�NODE_TYPE_SEP => separator������ͨ�� NODE_TYPE_ITE => Item
* @retv
*	����ɹ�������ָ�룬���ʧ�ܷ���0
*/
static void * AddNode(U8 NodeType) {
	GUI_HMEM hAlloc;
	void * pNext;
	if (NodeType == NODE_TYPE_SEP) {
		hAlloc = GUI_ALLOC_AllocZero(sizeof(Separator_t));
		pNext = (Separator_t *)GUI_ALLOC_h2p(hAlloc);
		((Separator_t *)pNext)->hAllocSpace = hAlloc;
	}else if(NodeType == NODE_TYPE_ITE){
		hAlloc = GUI_ALLOC_AllocZero(sizeof(Item_t));
		pNext = (Item_t*)GUI_ALLOC_h2p(hAlloc);
		((Item_t *)pNext)->hAllocSpace = hAlloc;
	}
	
	if (pNext == NULL) {
		GUI_ALLOC_Free(hAlloc);
	}
	//*hAllocSpace = hAlloc; //��������ڴ����������б��б���
		return pNext;

}
/*********************************************************************
*
*       AddSepLst
*   ���һ��SeparatorItem List ����ֻ�Ƿ������Ӧ���ڴ�ռ�
* @parm
*   n	: ��Ҫ��Ӷ��ٸ�
* @retv
*	����ɹ�����0�����ʧ�ܷ���ʧ�ܵ�separatorItem��Index 
*	-1 ͷ����ʧ��
*/
int AddSepLst(U16 n) {
	int i;
	int indexCnt;
	Separator_t * pSepLast;
	
	indexCnt = 0;
	i = 0;
	//
	//���ͷΪ�մ���һ��ͷ
	//
	if (Header.pSepaHeader == NULL) {
		Header.pSepaHeader = (Separator_t *)AddNode(NODE_TYPE_SEP);
		if (Header.pSepaHeader == NULL) {
			return -1;
		}
		++i;
		Header.pSepaHeader->SepaIndex = i;
		Header.pSepaHeader->ItemType = SEPA_TYPE;
		Header.pSepaHeader->pSepNext = NULL;
	}
	pSepLast = Header.pSepaHeader;
	//
	//�����һ���Ϊ�ռ�ָ����һ��ֱ��ָ�����һ����
	//
	while (pSepLast->pSepNext != NULL) {
		pSepLast = pSepLast->pSepNext;
		indexCnt++;
	} 
	//
	//����n����ͨItem  ����ʱ
	//
	for (; i < n; i++) {
		pSepLast->pSepNext = (Separator_t *)AddNode(NODE_TYPE_SEP);	//����while��pSepLastӦ��ΪNull,���Ҵ����￪ʼ�½��ڵ�
		if (pSepLast->pSepNext == NULL) {
			return i + indexCnt + 1;
		}
		
		pSepLast = pSepLast->pSepNext;
		pSepLast->SepaIndex = indexCnt + i + 1;
		pSepLast->ItemType = SEPA_TYPE;
		pSepLast->pItemHead = NULL; //�½���Separator���ڲ���ItemͷӦ��ΪNULL
		
	}
	pSepLast = NULL; //������ָ��ָ��
	return 0;
}
/*********************************************************************
*
*       AddItemLst
*   ���һ��Item�� ����ֻ�Ƿ������Ӧ���ڴ�ռ�
* @parm
*	pSepLst		: ����Separator Item ����ָ��
*   n			: ��Ҫ��Ӷ��ٸ�
* @retv Item_t * Item������
*	NULL : ͷ����ʧ��
*/
int AddItemLst(Separator_t *pSepLst,U16 n) {

	Item_t * pLast;
	//Item_t * pHeader;
	int i;
	int indexCnt;
	indexCnt = 0;
	i = 0;
	//pLast = pHeader = pSepLst->pItemHead;
	//
	//���Item��ͷ�����ڱ����½�һ����ͷ
	//
	if (pSepLst->pItemHead == NULL) {
		pSepLst->pItemHead = (Item_t *)AddNode(NODE_TYPE_ITE);
		if (pSepLst->pItemHead == NULL) {
			return -1;
		}
		i++;
		pLast  = pSepLst->pItemHead;
		pLast->ItemIndex = i;
		pLast->ItemType = ITEM_TYPE;
		pLast->pParentSepa = pSepLst; //Ӧ�������е���ͨItem�ĸ�SeparatorItemȷ��
		//if (pSepLst->SepaIndex == 1) {
		//	pLast->Focus = 1;
		//}
	}

	//���ͷ����ָ��ͷ
	pLast  = pSepLst->pItemHead;
	//
	//�ҵ���ĩβ����������β��ΪNULL��
	//
	while (pLast->pItemNext != NULL) {
		indexCnt++;
		pLast = pLast->pItemNext;
	}
	//
	//ѭ����ӽڵ�
	//
	for (; i < n; i++) {
		pLast->pItemNext = (Item_t *)AddNode(NODE_TYPE_ITE);
		if (pLast->pItemNext == NULL) {
			//return NULL;
			return i;
		}
		pLast = pLast->pItemNext;
		pLast->pParentSepa = pSepLst;
		pLast->ItemIndex = i + indexCnt + 1;
		pLast->ItemType = ITEM_TYPE;
		
	}
	pLast = NULL;
	return 0;
}

/*********************************************************************
*
*       DelItem
*   ɾ��һ��Item�� ɾ��SwpLst ��Itemʱͬʱ��Ҫɾ����������
* @parm
*	ppItemLst	: ������Ҫע������䴫�����һ������ָ�룬��Ŀ����ʲô���Լ�Ʒ
* @retv
*	0 : ɾ���ɹ�
* ��1 : δ�ҵ���Ҫɾ���Ķ��� 
*/
int DelItem(Item_t * pItem) {
	Separator_t * pSepa;
	Item_t * pBufItem;
	//
	//Item����Ի��ݵ�Separator�� ��������һ��Item�����������������ҵ���Ҫɾ������ɾ��
	//
	pSepa = pItem->pParentSepa; //����Item���Ի���丸separator
	pBufItem = pSepa->pItemHead; //�����丸separator���԰�Item list��ͷ�ҳ����ٱ����������Item
	//
	//���ж�ͷ�Ƿ�Ӧ����ɾ��
	//
	if (pBufItem == pItem) {
		pSepa->pItemHead = pBufItem->pItemNext;
		GUI_ALLOC_Free(pBufItem->hAllocSpace);
		return 0;
	}
	//
	//���ɾ���Ĳ�Ӧ����ͷ�ʹ�ͷ���濪ʼ����
	//
	pBufItem = pSepa->pItemHead;
	while (pBufItem->pItemNext != NULL) {
		if (pBufItem->pItemNext == pItem) {
			pBufItem->pItemNext = pItem->pItemNext;
			//
			// ɾ��������������а�Item index���ġ�
			//
			//pBufItem->pItemNext->ItemIndex = pBufItem->ItemIndex + 1;
			//while (pBufItem->pItemNext != NULL) {
			//	pBufItem->pItemNext->ItemIndex = pBufItem->ItemIndex + 1;
			//	pBufItem = pBufItem->pItemNext;
			//}
			GUI_ALLOC_Free(pItem->hAllocSpace);

			return 0;
		}
		else {
			pBufItem = pBufItem->pItemNext; //ָ����һ����
		}
	}
	//
	//��ִ�е�����ʱpBufItem ˵���ѱ�����Item��û���ҵ���Ҫɾ����Item
	//

	return -1;
}
/*********************************************************************
*
*       DelSpeItem
*   ɾ��һ�� Separator Item�� ɾ��Separator ��Itemʱ��Ҫ��������ڿ����б��� �����б��ɱ�ɾ��
* @parm
*	pHeader	: ��ͷָ�� ����ָ����Ϊ�������һ����ɾ����ͷ��ָ����Ӧ������
* @retv
*	0 : ɾ���ɹ�
* ��1 : δ�ҵ���Ҫɾ���Ķ���
*/
int DelSpeItem(Header_t *pHeader) {

}
/*********************************************************************
*
*       _OwnerDraw
*	�˺�����SWIPELIST���Զ�����ƺ����ص���
*	��Ҫ���ã�����Item�Ĳ����У�user data���� FOCUSE_BIT ��ȷ����������ɫ
*  note:�������Ƶ�ʱ��������Ļ�ڿɼ���Item ������һ������µ�LCD��ȥ���������ɿ�Item��ʱ����WM_InvalidateWindow�����Item
*/
static int _OwnerDraw(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {

	int  width;//height,
	GUI_RECT rInside;
	Item_t *pItemData;
	Separator_t * pSepaData;
	//U32 pItemData;
	GUI_COLOR color;
	static GUI_COLOR colorBk = -1;
	//int releaseItem;

	if (colorBk == -1) {
		colorBk = SWIPELIST_GetBkColor(pDrawItemInfo->hWin, SWIPELIST_CI_BK_SEP_ITEM);
	}
	switch (pDrawItemInfo->Cmd) {
	case WIDGET_ITEM_GET_XSIZE:
		WM_GetInsideRectEx(pDrawItemInfo->hWin, &rInside);
		width = rInside.x1;
		return width;
	case WIDGET_ITEM_DRAW_TEXT:
		GUI_SetTextMode(GUI_TM_TRANS);
		SWIPELIST_OwnerDraw(pDrawItemInfo);
		break;
	case WIDGET_ITEM_DRAW_BACKGROUND:

		//
		//��������Ĵ����Ǹ�����Item�����Ƿ���focuse���ı��䱳����ɫ
		//
		pItemData = (Item_t *)SWIPELIST_GetItemUserData(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex); //��Item��User Data���ȡ����ص�����
		if (pItemData->ItemType == SEPA_TYPE) {
			//SWIPELIST_OwnerDraw(pDrawItemInfo);
			//color = SWIPELIST_GetBkColor(pDrawItemInfo->hWin, SWIPELIST_CI_BK_SEP_ITEM);
			//color = GUI_DARKGRAY;
			if (colorBk) {
				color = colorBk;
			}
		}
		else {
			if (pItemData->Focus) {
				//���ݽ���״̬���Ʋ�ͬ��background
				//color = GUI_BLUE;
				color = GUI_MAKE_COLOR(0xB06245);
			}
			else {
				//color = GUI_DARKGRAY;
				color = GUI_MAKE_COLOR(0x6B563E);
				//GUI_ALLOC_AllocZero()
			}
		}

		GUI_SetColor(color);
		//GUI_ClearRect(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1);
		GUI_FillRect(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1);
		break;
	default:
		SWIPELIST_OwnerDraw(pDrawItemInfo);
		break;
	}
	return 0;
}
/*********************************************************************
*
*       _cbParent
*   SWIPELIST �ؼ��ĸ����ڻص�����
* @parm
*	pMsg	: ���ڻص�����
*/
static void _cbParent(WM_MESSAGE * pMsg) {
	int ID;
	int NCode;
	static int i,cnt;	
	static int itemIndex, numItems, cpltDisItm;
	static int tpOutPixel, btmInPixel; //���ڶ���¶������ͺ͵ײ�¶�����������
	int scrollPos;
	SWIPELIST_Handle hSwp;
	Item_t * pItemData;

	itemIndex = 0;
	cnt = 0;
	switch (pMsg->MsgId) {
	case WM_PAINT:
		GUI_SetBkColor(GUI_MAKE_COLOR(0xFAFBEA));
		GUI_Clear();
		break;
	case WM_NOTIFY_PARENT:
		ID = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch (NCode) {
		case WM_NOTIFICATION_CLICKED:

			break;
		case WM_NOTIFICATION_RELEASED:
			
			
			switch (ID) {
			case GUI_ID_BUTTON0 :
				if (LastFocus >= 0) { //��Ҫ����0��ԭ������Ϊ0һ�㶼��ΪSeparatorItem
					//
					//ȡ��SWPLST�ľ�� ��ֻ��Item���ܱ�ɾ��
					//
					hSwp = WM_GetDialogItem(pMsg->hWin, GUI_ID_SWIPELIST0);
					pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus);
					if (pItemData->ItemType == SEPA_TYPE) {
						LastFocus = 0;
					}
					else {
						//
						//ɾ��Item,������֮ǰ�Ȼ�ȡ������������ɾ��������ɾ��Item
						//
						pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus);
						DelItem(pItemData);
						SWIPELIST_DeleteItem(hSwp, LastFocus);
						//
						//������Ҫע��������ɾ�����һ��itemʱLastFocus���Ϊ�Ƿ�������Ҫ����һ�¡�
						//
						numItems = SWIPELIST_GetNumItems(hSwp);
						LastFocus = (LastFocus >= numItems) ? numItems - 1 : LastFocus;
						//�����Ҫ�������������һ���������ӡ����					
						Traversal = 1;
						//
						//���ɾ���� ���Ľ�������ͨItem ��ǽ��㣬���򽹵�ס���ƶ�������
						//
						pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus);
						if (pItemData->ItemType == ITEM_TYPE) {
							pItemData->Focus = 1;
						}
						else {
							pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus-1);
							if (pItemData->ItemType == ITEM_TYPE) {
								pItemData->Focus = 1;
								LastFocus -= 1;
							}else{
								LastFocus = 0;
							}
						}
/*ɾ������ĺ��Ĵ�������￪ʼ*/
						//
						//ɾ��������scrollPos �������õ�ǰ�����Itemindex
						//
						scrollPos = -SWIPELIST_GetScrollPos(hSwp);
						itemIndex = scrollPos / (ITEM_SIZE + SEP_SIZE);
						//
						//����Item�Ƴ���pixel�͵ײ���Item���ڴ��������pixel
						//
						tpOutPixel = scrollPos % (ITEM_SIZE + SEP_SIZE);
						btmInPixel = (WM_GetWindowSizeY(hSwp) % (ITEM_SIZE + SEP_SIZE));
						cpltDisItm = WM_GetWindowSizeY(hSwp) / (ITEM_SIZE + SEP_SIZE);
						//
						//�����cnt��Ϊ����
						//
						numItems  = SWIPELIST_GetNumItems(hSwp);
						cnt = (numItems <= cpltDisItm) ? numItems - 1 : itemIndex + cpltDisItm;
						//cnt = itemIndex + 6;
						if (tpOutPixel + btmInPixel >= ITEM_SIZE + SEP_SIZE) {
							cnt += 1;
						}
						//
						//�Ӵ����ڵĵ�һ��Item��ʼѭ��
						//
						for (i = itemIndex; i <= cnt; i++) {
							if (i == numItems) break; //��Ҫ��֤���ҵ�ItemIndex����Խ��
							pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, i);
							if (pItemData->ItemType == ITEM_TYPE) {
								SWIPELIST_ItemDetachWindow(hSwp, pItemData->hAttachWin);
								SWIPELIST_ItemAttachWindow(hSwp, i, pItemData->hAttachWin, 138, 1);
								if (!WM_IsVisible(pItemData->hAttachWin)) {
									WM_ShowWin(pItemData->hAttachWin);
								}
							}
						}
						//SWIPELIST_SetScrollPosItem(hSwp, 6);
					}
				}
				break;
			case GUI_ID_SWIPELIST0:
				
				Visable = SWIPELIST_GetScrollPos(pMsg->hWinSrc);
				//Visable = SWIPELIST_GetNumItems(pMsg->hWinSrc);
				itemIndex = SWIPELIST_GetReleasedItem(pMsg->hWinSrc);
				//
				//�����һ����ý����Item���λ
				//
				if (LastFocus > 0) {
					pItemData = (Item_t *)SWIPELIST_GetItemUserData(pMsg->hWinSrc, LastFocus);
					//if (pItemData->ItemType == ITEM_TYPE) {
						pItemData->Focus = 0;
					//}
				}
				//
				//��ȡ�ɿ���Item��UserData ���ﲻ���ж���Sepa Item ������ͨItem ��Ϊֻ��Item�����е���ɿ�֪ͨ
				//
				pItemData = (Item_t *)SWIPELIST_GetItemUserData(pMsg->hWinSrc, itemIndex);//����ֵǿ��ת����һ��ָ��																				   //*pItemData &= 0xFFFFFF00; //��������¶ȵĵ�8λ
				pItemData->Focus = 1;

				LastFocus = itemIndex;
				//���ڲ��� ��� 1 Item��ʱ������6ȥ
				//if (itemIndex == 1) {
				//	SWIPELIST_SetScrollPosItem(pMsg->hWinSrc, 5);
				//}
				//
				//��Ч��Swpelist������ʹ֮�ػ�
				//
				WM_InvalidateWindow(pMsg->hWin);
				break;
			} //end for switch(ID)
			break;
		case WM_NOTIFICATION_SEL_CHANGED:
			//scrollPos = -SWIPELIST_GetScrollPos(hSwp);
			//itemIndex = scrollPos / ITEM_SIZE;

			//itemIndex +=6

			break;
		}//end for  switch(NCode)
		break;
	default:
		WM_DefaultProc(pMsg);
	}
}

/*********************************************************************
*
*       MainTask
*   ������  ���򽫴����￪ʼ����
*	
*/
void MainTask(void) {
	int xSize, ySize;
	int textYPos;
	SWIPELIST_Handle hSwpLst;
	GUI_HWIN hSwpParent;
	//GUI_HWIN hAttachText;
	BUTTON_Handle hButton;
	Separator_t * pSepNode;
	Item_t		* pItemNode;
	int i, devCnt, itemIndex;
	char text[10];
	char sepInfoText[50];
	Header.pSepaHeader = NULL;
	//LastFocus = -1; //
	WM_SetCreateFlags(WM_CF_MEMDEV);

	GUI_Init();

	GUI_SelectLayer(0);

	WM_MULTIBUF_Enable(1);
	WM_MOTION_Enable(1);
	
	xSize = LCD_GetXSize();
	ySize = LCD_GetYSize();

	hSwpParent = WM_CreateWindowAsChild(0, 0, xSize/2, ySize, WM_HBKWIN, WM_CF_SHOW | WM_CF_MEMDEV | WM_CF_HASTRANS, _cbParent, 0);
	//
	//����SWIPELISTС����
	//
	hSwpLst = SWIPELIST_CreateEx(1, 1, xSize / 2-40, ySize - 1, hSwpParent, WM_CF_SHOW | WM_CF_MOTION_Y | WM_CF_HASTRANS, 0, GUI_ID_SWIPELIST0);
	SWIPELIST_SetOwnerDraw(hSwpLst, _OwnerDraw);
	SWIPELIST_SetBkColor(hSwpLst, SWIPELIST_CI_BK_SEP_ITEM, GUI_MAKE_COLOR(0xD74580));
	SWIPELIST_SetDefaultSepSize(SEP_SIZE);
	//
	//����һ����������ɾ������
	//
	hButton = BUTTON_CreateEx(203, 5, 35, 20, hSwpParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, GUI_ID_BUTTON0);
	BUTTON_SetText(hButton, "Del");
	//
	//ģ�����Item
	//
	AddSepLst(4);
	AddItemLst(Header.pSepaHeader, 5);//��ͷSeparatorItem���
	AddItemLst(Header.pSepaHeader->pSepNext, 4);
	AddItemLst(Header.pSepaHeader->pSepNext->pSepNext,4);
	AddItemLst(Header.pSepaHeader->pSepNext->pSepNext->pSepNext, 4);
	//����һ��SeparatorLst��ÿ����Item��ӽ�ȥ
	pSepNode = Header.pSepaHeader;

	GUI_SetColor(GUI_WHITE);
	textYPos = 5;
	i = 0;
	itemIndex = 0;
	//
	//��������SeparatorItem��Item���ݲ���������ӵ�SwipeLst��ȥ��������Ĵ��������˶����ݵ���䣬��ʵ��������������豸���
	//
	while (pSepNode != NULL) {
		//pSepNode->pItemHead = AddItemLst(pSepNode->pItemHead, 4);
		//
		//��ӡ��һЩSeparatorItem��Ϣ�ڴ���
		//
		strcpy((char*)(pSepNode->SepaName), SepName[i++]);
		sprintf(sepInfoText, "Index:%d\tmHand:%d\tName:%s",pSepNode->SepaIndex,pSepNode->hAllocSpace,pSepNode->SepaName);
		GUI_SetFont(GUI_FONT_16B_ASCII);
		GUI_SetColor(GUI_RED);
		GUI_DispStringAt(sepInfoText, 240, textYPos);
		textYPos += 17;
		//
		//���һ��SeparatorItemʱ����������ItemҲ��ȫ��������ӽ�SwipLst �����Item UserData
		//
		SWIPELIST_AddSepItem(hSwpLst, pSepNode->SepaName, ITEM_SIZE);
		SWIPELIST_SetItemUserData(hSwpLst, itemIndex++, (U32)(pSepNode));
		//
		//��¼itemIndex, �豸������0���ڵ��ƶ�����һ���ڵ�
		//
		devCnt = 0;
		pItemNode = pSepNode->pItemHead;
		while (pItemNode != NULL) {
			//
			//��ӡ��һЩItem��Ϣ�ڴ���
			//
			sprintf(pItemNode->ItemName, "Device%d",devCnt++);
			sprintf(sepInfoText, "--Item:%d\tmHand:%d\tName:%s", pItemNode->ItemIndex,pItemNode->hAllocSpace,pItemNode->ItemName);
			GUI_SetFont(GUI_FONT_10_ASCII);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt(sepInfoText, 240, textYPos);
			textYPos += 17;
			//
			//���Item��SiwpeList��
			//
			SWIPELIST_AddItem(hSwpLst, pItemNode->ItemName, ITEM_SIZE);
			//
			//��һ��Win������Attach��Item��
			//
			pItemNode->hAttachWin = TEXT_CreateEx(0, 0, 50, ITEM_SIZE - 2, hSwpParent, WM_CF_SHOW, 0, USER_ID_TEXT + i, "3�C");//"3�C"
			TEXT_SetTextAlign(pItemNode->hAttachWin, TEXT_CF_VCENTER | TEXT_CF_HCENTER);
			TEXT_SetBkColor(pItemNode->hAttachWin, GUI_GRAY);
			TEXT_SetFont(pItemNode->hAttachWin, GUI_FONT_16B_1);
			SWIPELIST_ItemAttachWindow(hSwpLst, itemIndex, pItemNode->hAttachWin, 138, 1);
			//��Item��Data��ָ�봫�ݸ����Item User Data
			SWIPELIST_SetItemUserData(hSwpLst, itemIndex, (U32)(pItemNode));
			//ָ����һ���ڵ� 
			pItemNode = pItemNode->pItemNext;
			
			itemIndex++;
		}
		//ָ��SeparatorItem�����һ���ڵ�
		pSepNode = pSepNode->pSepNext;
	}
	//�ƶ�������λ�ò����Ƿ�ɹ�
	SWIPELIST_SetScrollPos(hSwpLst, 15);
	//
	//�������е����ݲ���ÿ��SeparatorItem����һ��Item��SepSize��������һ���ƹ����BUG
	//
	itemIndex = 0;
	pSepNode = Header.pSepaHeader;
	while (pSepNode != NULL) {
		devCnt = 0;
		pItemNode = pSepNode->pItemHead;
		itemIndex++;
		while (pItemNode != NULL) {
			pItemNode = pItemNode->pItemNext;
			if (pItemNode == NULL && pSepNode->pSepNext != NULL) {
				//SWIPELIST_SetItemSize(hSwpLst, itemIndex, ITEM_SIZE + SEP_SIZE);
				SWIPELIST_SetSepSize(hSwpLst, itemIndex, SEP_SIZE);
			}
			itemIndex++;
		}		
		pSepNode = pSepNode->pSepNext;
	}
	//
	//������ѭ����������ִ��GUI_Delay()��GUI���в�������ӡ��ص���Ϣ����Ļ�ϡ�
	//
	while (1) {
		if (Traversal) {
			//
			//������λ����ʼ����Ӧ����
			//
			Traversal = 0;
			textYPos = 5;
			i = 0;
			itemIndex = 0;

			//
			//���������ӡ�����е����� �ڴ�֮ǰ��Ҫ�����ʾ����
			//
			GUI_ClearRect(240, 3, LCD_GetXSize(), LCD_GetYSize());
			pSepNode = Header.pSepaHeader;
			while (pSepNode != NULL) {
				//
				//ȡ��Separator��������ݲ���ӡ����Ļ�ϡ�
				//
				strcpy((char*)(pSepNode->SepaName), SepName[i++]);
				sprintf(sepInfoText, "Index:%d\tmHand:%d\tName:%s", pSepNode->SepaIndex, pSepNode->hAllocSpace, pSepNode->SepaName);
				GUI_SetFont(GUI_FONT_16B_ASCII);
				GUI_SetColor(GUI_RED);
				GUI_DispStringAt(sepInfoText, 240, textYPos);
				textYPos += 17;

				devCnt = 0;
				pItemNode = pSepNode->pItemHead;
				while (pItemNode != NULL) {
					//
					//ȡ��Item��������ݲ���ӡ����Ļ��
					//
					sprintf(pItemNode->ItemName, "Device%d", devCnt++);
					sprintf(sepInfoText, "--Item:%d\tmHand:%d\tName:%s", pItemNode->ItemIndex, pItemNode->hAllocSpace, pItemNode->ItemName);
					GUI_SetFont(GUI_FONT_10_ASCII);
					GUI_SetColor(GUI_WHITE);
					GUI_DispStringAt(sepInfoText, 240, textYPos);
					textYPos += 17;
					//ָ����һ���ڵ� 
					pItemNode = pItemNode->pItemNext;
				}
				//ָ��SeparatorItem�����һ���ڵ�
				pSepNode = pSepNode->pSepNext;
			}
		}
		GUI_Delay(50);
	}
}

