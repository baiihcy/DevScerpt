#include "trlist.h"

/*
#define FOREACH_NODE _each_node
#define FOREACH_LIST_BEGINN(_list) \
TR_LIST_NODE *_each_node=_list->pListHead,*_start_node=_each_node;\
while (_each_node && _each_node->pNext!=_start_node) { 
#define FOREACH_LIST_END \
_each_node=_each_node->pNext;}
*/

void MakeCircular(struct TR_LIST *pList)
{
	if (!pList) return;
	pList->m_bCircular=TRUE;
	//printf("\n MakeCircular:Head=%x,Tail=%x",pList->m_pListHead,pList->m_pListTail);
	if (pList->m_pListHead) 
		pList->m_pListHead->pPrev=pList->m_pListTail;
	if (pList->m_pListTail)
		pList->m_pListTail->pNext=pList->m_pListHead;
}

BOOL IsEmpty(struct TR_LIST *pList)
{
	if (!pList || !pList->m_pListHead || !pList->m_pListTail)
		return TRUE;
	return FALSE;
}
int GetCount(struct TR_LIST *pList)
{
	if (!pList || !pList->m_pListHead) return 0;
	int nCount=0;
	TR_LIST_NODE *pNd=pList->m_pListHead;
	TR_LIST_NODE *pStart=pNd;
	do {
		nCount++;
		pNd=pNd->pNext;
	}while (pNd && pNd->pNext!=pStart);
	return nCount;
}
TR_LIST_NODE* SetHead(struct TR_LIST *pList,TR_LIST_NODE *pHead)
{
	if (!pList) return NULL;
	TR_LIST_NODE *pOldHead=pList->m_pListHead;
	pList->m_pListHead=pHead;
	if (!pList->m_pListTail)
		pList->m_pListTail=pHead;
	if (pList->m_bCircular) {
		pList->MakeCircular(pList);
	}
	return pOldHead;
}
TR_LIST_NODE* SetTail(struct TR_LIST *pList,TR_LIST_NODE *pTail)
{
	if (!pList) return NULL;
	TR_LIST_NODE *pOldTail=pList->m_pListTail;
	pList->m_pListTail=pTail;
	if (!pList->m_pListHead)
		pList->m_pListHead=pTail;
	if (pList->m_bCircular) {
		pList->MakeCircular(pList);
	}
	return pOldTail;
}
TR_LIST_NODE* FindNode(struct TR_LIST *pList,void *pData)
{
	if (!pList || !pList->m_pListHead) return NULL;
	TR_LIST_NODE *pNd=pList->m_pListHead;
	TR_LIST_NODE *pStart=pNd;
	do {
		if (pNd->pData==pData) {
			return pNd;
		}
		pNd=pNd->pNext;
	}while (pNd && pNd->pNext!=pStart);
	return NULL;
}
TR_LIST_NODE* NodeInList(struct TR_LIST *pList,TR_LIST_NODE *pNode)
{
	if (!pList || !pList->m_pListHead || !pNode)
		return NULL;
	TR_LIST_NODE *pNd=pList->m_pListHead;
	TR_LIST_NODE *pStart=pNd;
	do {
		if (pNd==pNode) {
			return pNode;
		}
		pNd=pNd->pNext;
	}while (pNd && pNd->pNext!=pStart);
	return NULL;
}
void RemoveNode(struct TR_LIST *pList,TR_LIST_NODE *pNode)
{
	if (!pList || !pNode)
		return ;

	if (pNode->pPrev && pNode->pPrev->pNext==pNode)
		pNode->pPrev->pNext=pNode->pNext;
	if (pNode->pNext && pNode->pNext->pPrev==pNode)
		pNode->pNext->pPrev=pNode->pPrev;

	if (pList->m_pListHead==pNode)
		pList->SetHead(pList,pNode->pNext);
	if (pList->m_pListTail==pNode)
		pList->SetTail(pList,pNode->pPrev);
}
TR_LIST_NODE* InsertNode(struct TR_LIST *pList,TR_LIST_NODE *pPrev,TR_LIST_NODE *pNode)
{
	if (!pList || !pNode)
		return NULL;
	if (NULL==pList->m_pListHead) {
		pList->SetHead(pList,pNode);
		return pNode;
	}
	TR_LIST_NODE *pNext=NULL;
	if (pPrev) pNext=pPrev->pNext;
	else pNext=pList->m_pListHead;	
	
	if (!pPrev || pNode!=pPrev->pNext) {
		pNode->pPrev=pPrev;
		pNode->pNext=pNext;

		if (pPrev) pPrev->pNext=pNode;
		if (pNext) pNext->pPrev=pNode;
	}
	if (pPrev==pList->m_pListTail)
		pList->SetTail(pList,pNode);
	else if (pNext==pList->m_pListHead)
		pList->SetHead(pList,pNode);
	return pNode;
}
TR_LIST_NODE* MoveNode(struct TR_LIST *pList,TR_LIST_NODE *pPrev,TR_LIST_NODE *pNode)
{
	if (!pList || !pNode)
		return NULL;
	if (pPrev && pPrev->pNext!=pNode) {
		pList->RemoveNode(pList,pNode);
		return pList->InsertNode(pList,pPrev,pNode);
	}
	return pNode;
}
void DeleteNode(struct TR_LIST *pList,TR_LIST_NODE *pNode)
{
	if (!pList || !pNode)
		return ;
	pList->RemoveNode(pList,pNode);
	if (pNode) {
		if (pList->OnDelNodeData)
			pList->OnDelNodeData(pNode->pData);
		free(pNode);
	}
}
TR_LIST_NODE* InsertData(struct TR_LIST *pList,TR_LIST_NODE *pPrev,void *pData)
{
	if (!pList || !pData)
		return NULL;
	TR_LIST_NODE *pNode=malloc(sizeof(TR_LIST_NODE));
	if (pNode) {
		memset(pNode,0,sizeof(TR_LIST_NODE));
		pNode->pData=pData;
		if (NULL==pList->InsertNode(pList,pPrev,pNode)) {
			free(pNode);
			printf("\n InsertData error : Failed to InsertNode");
			return NULL;
		}
	} else {
		printf("\n InsertData error : Failed to malloc(TR_LIST_NODE)");
	}
	return pNode;
}
void DeleteAll(struct TR_LIST *pList)
{
	while (pList->m_pListHead)
		pList->DeleteNode(pList,pList->m_pListHead);
}
//////////////////////////////////////////////////////////////////////////

void InitTRList(TR_LIST *pList)
{
	memset(pList,0,sizeof(TR_LIST));
	pList->MakeCircular=MakeCircular;
	pList->IsEmpty=IsEmpty;
	pList->GetCount=GetCount;
	pList->SetHead=SetHead;
	pList->SetTail=SetTail;
	pList->NodeInList=NodeInList;
	pList->FindNode=FindNode;
	pList->InsertNode=InsertNode;
	pList->InsertData=InsertData;
	pList->RemoveNode=RemoveNode;
	pList->DeleteNode=DeleteNode;
	pList->DeleteAll=DeleteAll;
}
TR_LIST* NewTRList()
{
	TR_LIST *pList=malloc(sizeof(TR_LIST));
	if (pList)
		InitTRList(pList);
	return pList;
}
