#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "GUI.h"
#include "DIALOG.h"

#define USER_ID_TEXT (GUI_ID_USER + 0)
//
//以下宏定义的是separator和普通Item的type主要用于在创建node的时候
//
#define NODE_TYPE_SEP	0
#define NODE_TYPE_ITE	1
#define ITEM_SIZE		40
#define SEP_SIZE		1
//
//枚举出Item的类型
//
typedef enum {
	SEPA_TYPE =1,
	ITEM_TYPE
}ItemType_e;
//
//定义 Item的数据结构
//
struct ItemInfo_t {
	ItemType_e	ItemType;
	U8	Focus;
	U8	Temp;
	char ItemName[16];
	U16 ItemIndex;
	U32 hAllocSpace;
	GUI_HWIN hAttachWin; //每个Item(除separator之外的Item) 所附着的窗口。
						 //是否需要指向Item所属的SepartorItem 以获得其Index? 以便 知道本Item在哪个之下
	struct SepaInfo_t * pParentSepa;
	struct ItemInfo_t * pItemNext;
};
typedef struct ItemInfo_t Item_t;

//typedef struct {
//	Item_t * pItemHeader;
//}ItemHeader_t;
//
//定义 Separator Item的数据结构
//
struct SepaInfo_t {
	ItemType_e	ItemType;
	char SepaName[16];
	U16 SepaIndex;		//separator的索引
	//U16 ItemCnt;		//其内有多少个Item
	U32	hAllocSpace;
	Item_t * pItemHead; //Item的数据头
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
*   添加一结点 给节点分配空间
* @parm
*	NodeType :节点的类型，NODE_TYPE_SEP => separator还是普通的 NODE_TYPE_ITE => Item
* @retv
*	如果成功返回其指针，如果失败返回0
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
	//*hAllocSpace = hAlloc; //把申请的内存句柄保存在列表中便于
		return pNext;

}
/*********************************************************************
*
*       AddSepLst
*   添加一个SeparatorItem List 这里只是分配出相应的内存空间
* @parm
*   n	: 需要添加多少个
* @retv
*	如果成功返回0，如果失败返回失败的separatorItem的Index 
*	-1 头创建失败
*/
int AddSepLst(U16 n) {
	int i;
	int indexCnt;
	Separator_t * pSepLast;
	
	indexCnt = 0;
	i = 0;
	//
	//如果头为空创建一个头
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
	//如果下一个项不为空既指向下一个直到指向最后一个。
	//
	while (pSepLast->pSepNext != NULL) {
		pSepLast = pSepLast->pSepNext;
		indexCnt++;
	} 
	//
	//创建n个普通Item  这里时
	//
	for (; i < n; i++) {
		pSepLast->pSepNext = (Separator_t *)AddNode(NODE_TYPE_SEP);	//跳出while后pSepLast应当为Null,并且从这里开始新建节点
		if (pSepLast->pSepNext == NULL) {
			return i + indexCnt + 1;
		}
		
		pSepLast = pSepLast->pSepNext;
		pSepLast->SepaIndex = indexCnt + i + 1;
		pSepLast->ItemType = SEPA_TYPE;
		pSepLast->pItemHead = NULL; //新建的Separator其内部的Item头应当为NULL
		
	}
	pSepLast = NULL; //把最后的指针指空
	return 0;
}
/*********************************************************************
*
*       AddItemLst
*   添加一个Item表 这里只是分配出相应的内存空间
* @parm
*	pSepLst		: 传入Separator Item 数据指针
*   n			: 需要添加多少个
* @retv Item_t * Item项数据
*	NULL : 头分配失败
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
	//如果Item表头不存在便先新建一个表头
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
		pLast->pParentSepa = pSepLst; //应当把所有的普通Item的父SeparatorItem确定
		//if (pSepLst->SepaIndex == 1) {
		//	pLast->Focus = 1;
		//}
	}

	//如果头不空指向头
	pLast  = pSepLst->pItemHead;
	//
	//找到最末尾的项，（其项的尾巴为NULL）
	//
	while (pLast->pItemNext != NULL) {
		indexCnt++;
		pLast = pLast->pItemNext;
	}
	//
	//循环添加节点
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
*   删除一个Item项 删除SwpLst 的Item时同时需要删除其数据项
* @parm
*	ppItemLst	: 这里需要注意的是其传入的是一个二级指针，其目的是什么请自己品
* @retv
*	0 : 删除成功
* －1 : 未找到需要删除的对象 
*/
int DelItem(Item_t * pItem) {
	Separator_t * pSepa;
	Item_t * pBufItem;
	//
	//Item项可以回溯到Separator项 其项里有一个Item表，把这个表给遍历查找到需要删除的再删除
	//
	pSepa = pItem->pParentSepa; //根据Item可以获得其父separator
	pBufItem = pSepa->pItemHead; //根据其父separator可以把Item list的头找出来再遍历出其具体Item
	//
	//先判断头是否应当被删除
	//
	if (pBufItem == pItem) {
		pSepa->pItemHead = pBufItem->pItemNext;
		GUI_ALLOC_Free(pBufItem->hAllocSpace);
		return 0;
	}
	//
	//如果删除的不应该是头就从头下面开始遍历
	//
	pBufItem = pSepa->pItemHead;
	while (pBufItem->pItemNext != NULL) {
		if (pBufItem->pItemNext == pItem) {
			pBufItem->pItemNext = pItem->pItemNext;
			//
			// 删除后遍历所有所有把Item index更改。
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
			pBufItem = pBufItem->pItemNext; //指向下一个。
		}
	}
	//
	//当执行到这里时pBufItem 说明把遍历完Item后没有找到需要删除的Item
	//

	return -1;
}
/*********************************************************************
*
*       DelSpeItem
*   删除一个 Separator Item项 删除Separator 的Item时需要把其放置于空闲列表中 空闲列表不可被删除
* @parm
*	pHeader	: 表头指针 传入指针是为了如果第一个项删除表头所指的项应当更改
* @retv
*	0 : 删除成功
* －1 : 未找到需要删除的对象
*/
int DelSpeItem(Header_t *pHeader) {

}
/*********************************************************************
*
*       _OwnerDraw
*	此函数是SWIPELIST的自定义绘制函数回调。
*	主要作用：根据Item的参数中（user data）的 FOCUSE_BIT 来确定背景的颜色
*  note:发生绘制的时候会绘制屏幕内可见的Item 但并不一定会更新到LCD上去。所以在松开Item的时候我WM_InvalidateWindow了这个Item
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
		//这里下面的代码是根据其Item参数是否有focuse来改变其背景颜色
		//
		pItemData = (Item_t *)SWIPELIST_GetItemUserData(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex); //从Item的User Data里获取到相关的数据
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
				//根据焦点状态绘制不同的background
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
*   SWIPELIST 控件的父窗口回调函数
* @parm
*	pMsg	: 窗口回调参数
*/
static void _cbParent(WM_MESSAGE * pMsg) {
	int ID;
	int NCode;
	static int i,cnt;	
	static int itemIndex, numItems, cpltDisItm;
	static int tpOutPixel, btmInPixel; //窗口顶部露在外面和和底部露在里面的像素
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
				if (LastFocus >= 0) { //需要大于0的原因是因为0一般都做为SeparatorItem
					//
					//取出SWPLST的句柄 且只有Item才能被删除
					//
					hSwp = WM_GetDialogItem(pMsg->hWin, GUI_ID_SWIPELIST0);
					pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus);
					if (pItemData->ItemType == SEPA_TYPE) {
						LastFocus = 0;
					}
					else {
						//
						//删除Item,并且在之前先获取到它的数据先删除数据再删除Item
						//
						pItemData = (Item_t *)SWIPELIST_GetItemUserData(hSwp, LastFocus);
						DelItem(pItemData);
						SWIPELIST_DeleteItem(hSwp, LastFocus);
						//
						//这里需要注意的是如果删除最后一个item时LastFocus便成为非法所以需要处理一下。
						//
						numItems = SWIPELIST_GetNumItems(hSwp);
						LastFocus = (LastFocus >= numItems) ? numItems - 1 : LastFocus;
						//标记需要在主函数里遍历一次两个表打印出来					
						Traversal = 1;
						//
						//如果删除后 最后的焦点是普通Item 标记焦点，否则焦点住上移动或清零
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
/*删除对齐的核心代码从这里开始*/
						//
						//删除后获得其scrollPos 根据其获得当前最顶部的Itemindex
						//
						scrollPos = -SWIPELIST_GetScrollPos(hSwp);
						itemIndex = scrollPos / (ITEM_SIZE + SEP_SIZE);
						//
						//顶部Item移出的pixel和底部还Item还在窗口里面的pixel
						//
						tpOutPixel = scrollPos % (ITEM_SIZE + SEP_SIZE);
						btmInPixel = (WM_GetWindowSizeY(hSwp) % (ITEM_SIZE + SEP_SIZE));
						cpltDisItm = WM_GetWindowSizeY(hSwp) / (ITEM_SIZE + SEP_SIZE);
						//
						//计算出cnt是为了让
						//
						numItems  = SWIPELIST_GetNumItems(hSwp);
						cnt = (numItems <= cpltDisItm) ? numItems - 1 : itemIndex + cpltDisItm;
						//cnt = itemIndex + 6;
						if (tpOutPixel + btmInPixel >= ITEM_SIZE + SEP_SIZE) {
							cnt += 1;
						}
						//
						//从窗口内的第一个Item开始循环
						//
						for (i = itemIndex; i <= cnt; i++) {
							if (i == numItems) break; //需要保证查找的ItemIndex不得越界
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
				//清除上一个获得焦点的Item标记位
				//
				if (LastFocus > 0) {
					pItemData = (Item_t *)SWIPELIST_GetItemUserData(pMsg->hWinSrc, LastFocus);
					//if (pItemData->ItemType == ITEM_TYPE) {
						pItemData->Focus = 0;
					//}
				}
				//
				//获取松开的Item的UserData 这里不用判断是Sepa Item 还是普通Item 因为只有Item才能有点击松开通知
				//
				pItemData = (Item_t *)SWIPELIST_GetItemUserData(pMsg->hWinSrc, itemIndex);//把其值强制转换成一个指针																				   //*pItemData &= 0xFFFFFF00; //这里清除温度的低8位
				pItemData->Focus = 1;

				LastFocus = itemIndex;
				//用于测试 点击 1 Item的时候跳到6去
				//if (itemIndex == 1) {
				//	SWIPELIST_SetScrollPosItem(pMsg->hWinSrc, 5);
				//}
				//
				//无效化Swpelist父窗口使之重绘
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
*   主函数  程序将从这里开始运行
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
	//创建SWIPELIST小部件
	//
	hSwpLst = SWIPELIST_CreateEx(1, 1, xSize / 2-40, ySize - 1, hSwpParent, WM_CF_SHOW | WM_CF_MOTION_Y | WM_CF_HASTRANS, 0, GUI_ID_SWIPELIST0);
	SWIPELIST_SetOwnerDraw(hSwpLst, _OwnerDraw);
	SWIPELIST_SetBkColor(hSwpLst, SWIPELIST_CI_BK_SEP_ITEM, GUI_MAKE_COLOR(0xD74580));
	SWIPELIST_SetDefaultSepSize(SEP_SIZE);
	//
	//创建一个按键用于删除测试
	//
	hButton = BUTTON_CreateEx(203, 5, 35, 20, hSwpParent, WM_CF_SHOW | WM_CF_HASTRANS, 0, GUI_ID_BUTTON0);
	BUTTON_SetText(hButton, "Del");
	//
	//模拟添加Item
	//
	AddSepLst(4);
	AddItemLst(Header.pSepaHeader, 5);//对头SeparatorItem添加
	AddItemLst(Header.pSepaHeader->pSepNext, 4);
	AddItemLst(Header.pSepaHeader->pSepNext->pSepNext,4);
	AddItemLst(Header.pSepaHeader->pSepNext->pSepNext->pSepNext, 4);
	//遍历一次SeparatorLst把每个的Item添加进去
	pSepNode = Header.pSepaHeader;

	GUI_SetColor(GUI_WHITE);
	textYPos = 5;
	i = 0;
	itemIndex = 0;
	//
	//遍历所有SeparatorItem和Item数据并把它们添加到SwipeLst中去。在下面的代码中做了对数据的填充，在实际中数据由添加设备获得
	//
	while (pSepNode != NULL) {
		//pSepNode->pItemHead = AddItemLst(pSepNode->pItemHead, 4);
		//
		//打印出一些SeparatorItem信息在窗口
		//
		strcpy((char*)(pSepNode->SepaName), SepName[i++]);
		sprintf(sepInfoText, "Index:%d\tmHand:%d\tName:%s",pSepNode->SepaIndex,pSepNode->hAllocSpace,pSepNode->SepaName);
		GUI_SetFont(GUI_FONT_16B_ASCII);
		GUI_SetColor(GUI_RED);
		GUI_DispStringAt(sepInfoText, 240, textYPos);
		textYPos += 17;
		//
		//添加一个SeparatorItem时把其所属的Item也完全遍历并添加进SwipLst 并添加Item UserData
		//
		SWIPELIST_AddSepItem(hSwpLst, pSepNode->SepaName, ITEM_SIZE);
		SWIPELIST_SetItemUserData(hSwpLst, itemIndex++, (U32)(pSepNode));
		//
		//记录itemIndex, 设备计数归0，节点移动到下一个节点
		//
		devCnt = 0;
		pItemNode = pSepNode->pItemHead;
		while (pItemNode != NULL) {
			//
			//打印出一些Item信息在窗口
			//
			sprintf(pItemNode->ItemName, "Device%d",devCnt++);
			sprintf(sepInfoText, "--Item:%d\tmHand:%d\tName:%s", pItemNode->ItemIndex,pItemNode->hAllocSpace,pItemNode->ItemName);
			GUI_SetFont(GUI_FONT_10_ASCII);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt(sepInfoText, 240, textYPos);
			textYPos += 17;
			//
			//添加Item到SiwpeList中
			//
			SWIPELIST_AddItem(hSwpLst, pItemNode->ItemName, ITEM_SIZE);
			//
			//建一个Win并把其Attach到Item上
			//
			pItemNode->hAttachWin = TEXT_CreateEx(0, 0, 50, ITEM_SIZE - 2, hSwpParent, WM_CF_SHOW, 0, USER_ID_TEXT + i, "3癈");//"3癈"
			TEXT_SetTextAlign(pItemNode->hAttachWin, TEXT_CF_VCENTER | TEXT_CF_HCENTER);
			TEXT_SetBkColor(pItemNode->hAttachWin, GUI_GRAY);
			TEXT_SetFont(pItemNode->hAttachWin, GUI_FONT_16B_1);
			SWIPELIST_ItemAttachWindow(hSwpLst, itemIndex, pItemNode->hAttachWin, 138, 1);
			//把Item的Data的指针传递给这个Item User Data
			SWIPELIST_SetItemUserData(hSwpLst, itemIndex, (U32)(pItemNode));
			//指向下一个节点 
			pItemNode = pItemNode->pItemNext;
			
			itemIndex++;
		}
		//指向SeparatorItem表的下一个节点
		pSepNode = pSepNode->pSepNext;
	}
	//移动到敏感位置测试是否成功
	SWIPELIST_SetScrollPos(hSwpLst, 15);
	//
	//遍历所有的数据并把每个SeparatorItem的上一个Item的SepSize重新设置一下绕过这个BUG
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
	//以下主循环，在其中执行GUI_Delay()对GUI进行操作并打印相关的信息在屏幕上。
	//
	while (1) {
		if (Traversal) {
			//
			//清除标记位并初始化相应变量
			//
			Traversal = 0;
			textYPos = 5;
			i = 0;
			itemIndex = 0;

			//
			//遍历链表打印链表中的数据 在此之前需要清除显示区域
			//
			GUI_ClearRect(240, 3, LCD_GetXSize(), LCD_GetYSize());
			pSepNode = Header.pSepaHeader;
			while (pSepNode != NULL) {
				//
				//取出Separator的相关数据并打印到屏幕上。
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
					//取出Item的相关数据并打印到屏幕上
					//
					sprintf(pItemNode->ItemName, "Device%d", devCnt++);
					sprintf(sepInfoText, "--Item:%d\tmHand:%d\tName:%s", pItemNode->ItemIndex, pItemNode->hAllocSpace, pItemNode->ItemName);
					GUI_SetFont(GUI_FONT_10_ASCII);
					GUI_SetColor(GUI_WHITE);
					GUI_DispStringAt(sepInfoText, 240, textYPos);
					textYPos += 17;
					//指向下一个节点 
					pItemNode = pItemNode->pItemNext;
				}
				//指向SeparatorItem表的下一个节点
				pSepNode = pSepNode->pSepNext;
			}
		}
		GUI_Delay(50);
	}
}

