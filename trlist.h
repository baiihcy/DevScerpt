#include "types.h"

#ifndef H_TR_LIST
#define H_TR_LIST

struct TR_LIST;

typedef struct TR_LIST_NODE
{
	void *pData;
	struct TR_LIST_NODE *pPrev;
	struct TR_LIST_NODE *pNext;
}TR_LIST_NODE;

typedef struct TR_LIST
{
	TR_LIST_NODE *m_pListHead;
	TR_LIST_NODE *m_pListTail;
	BOOL m_bCircular;
	void (*MakeCircular)(struct TR_LIST *pList);
	BOOL (*IsEmpty)(struct TR_LIST *pList);
	int (*GetCount)(struct TR_LIST *pList);
	TR_LIST_NODE* (*SetHead)(struct TR_LIST *pList,TR_LIST_NODE *pHead);
	TR_LIST_NODE* (*SetTail)(struct TR_LIST *pList,TR_LIST_NODE *pTail);
	TR_LIST_NODE* (*NodeInList)(struct TR_LIST *pList,TR_LIST_NODE *pNode);
	TR_LIST_NODE* (*FindNode)(struct TR_LIST *pList,void *pData);
	TR_LIST_NODE* (*InsertNode)(struct TR_LIST *pList,TR_LIST_NODE *pPrev,TR_LIST_NODE *pNode);
	TR_LIST_NODE* (*InsertData)(struct TR_LIST *pList,TR_LIST_NODE *pPrev,void *pData);
	void (*RemoveNode)(struct TR_LIST *pList,TR_LIST_NODE *pNode);
	void (*DeleteNode)(struct TR_LIST *pList,TR_LIST_NODE *pNode);
	void (*DeleteAll)(struct TR_LIST *pList);
	
	void (*OnDelNodeData)(void* pData);
}TR_LIST;

void InitTRList(TR_LIST *pList);
TR_LIST* NewTRList();

#endif
