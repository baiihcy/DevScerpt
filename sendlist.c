#include "sendlist.h"

//////////////////////////////////////////////////////////////////////////
SENDFRAME_INFO* NewSendframeInfo(BYTE *pSend,WORD cbSendSize,LUA_RECV_CALLBACK pfnRecvCallback,BOOL bRecvUseFrame)
{
	SENDFRAME_INFO *pInfo=malloc(sizeof(SENDFRAME_INFO));
	memset(pInfo,0,sizeof(SENDFRAME_INFO));
	if (NULL==pInfo) {
		printf("\n NewSendframeInfo error : Failed to malloc(SENDFRAME_INFO)");
		return NULL;
	}
	pInfo->m_pSend=malloc(cbSendSize);
	if (NULL==pInfo->m_pSend) {
		free(pInfo);
		printf("\n NewSendframeInfo error : Failed to malloc(SENDFRAME_INFO.m_pSend)");
		return NULL;
	}
	COPY_LUA_CALLBACK(pInfo->m_pfnRecvCallBack,pfnRecvCallback);
	pInfo->m_bRecvUseFrame=bRecvUseFrame;
	return pInfo;
}
void FreeSendframeInfo(SENDFRAME_INFO *pInfo)
{
	if (!pInfo) return;
	if (pInfo->m_pSend)
		free(pInfo->m_pSend);
	free(pInfo);
}
//////////////////////////////////////////////////////////////////////////

SENDFRAME_INFO* AddHead(struct SENDFRAME_LIST* pList,BYTE *pSend,WORD cbSendSize,LUA_RECV_CALLBACK pfnRecvCallback,BOOL bRecvUseFrame)
{
	if (!pList) return NULL;
	DWORD dwID=pList->m_dwAddedCount+1;
	TR_LIST *pTRList=&pList->m_TRList;
	SENDFRAME_INFO *pInfo=NewSendframeInfo(pSend,cbSendSize,pfnRecvCallback,bRecvUseFrame);
	if (NULL==pInfo) 
		goto __err;

	TR_LIST_NODE *pNode=pNode=pTRList->InsertData(pTRList,NULL,pInfo);
	if (NULL==pNode)
		goto __err;
	pInfo->m_dwID=dwID;
	
	pList->m_dwAddedCount++;
	return pInfo;

__err:
	if (pNode) {
		pNode->pData=NULL;
		pTRList->DeleteNode(pTRList,pNode);
	}
	FreeSendframeInfo(pInfo);
	printf("\n SENDFRAME_LIST.AddHead Error ID(%lu)",dwID);
	return NULL;
}
void DelTail(struct SENDFRAME_LIST* pList)
{
	if (!pList) return ;
	TR_LIST *pTRList=&pList->m_TRList;
	pTRList->DeleteNode(pTRList,pTRList->m_pListTail);
}
SENDFRAME_INFO* GetTail(struct SENDFRAME_LIST* pList)
{
	if (!pList) return NULL;
	TR_LIST *pTRList=&pList->m_TRList;
	if (pTRList->m_pListTail)
		return (SENDFRAME_INFO*)pTRList->m_pListTail->pData;
	return NULL;
}
void SendframeList_Clear(struct SENDFRAME_LIST* pList)
{
	if (!pList) return ;
	TR_LIST *pTRList=&pList->m_TRList;
	pTRList->DeleteAll(pTRList);
}
void OnDelete_SendframeInfo(void *pData)
{
	SENDFRAME_INFO *pInfo=(SENDFRAME_INFO *)pData;
	FreeSendframeInfo(pInfo);
}
void InitSendframeList(SENDFRAME_LIST *pSendframeList)
{
	memset(pSendframeList,0,sizeof(SENDFRAME_LIST));
	InitTRList(&pSendframeList->m_TRList);
	pSendframeList->m_TRList.OnDelNodeData=OnDelete_SendframeInfo;
	pSendframeList->AddHead=AddHead;
	pSendframeList->DelTail=DelTail;
	pSendframeList->GetTail=GetTail;
	pSendframeList->Clear=SendframeList_Clear;
}

SENDFRAME_LIST* NewSendframeList()
{
	SENDFRAME_LIST *pList=malloc(sizeof(SENDFRAME_LIST));
	if (!pList) {
		printf("\n NewSendframeList error : Failed to malloc");
		return NULL;
	}
	InitSendframeList(pList);
	return pList;
}
//////////////////////////////////////////////////////////////////////////

INTERVALSEND_INFO* NewIntervalsendInfo(DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,LUA_RECV_CALLBACK pfnRecvCallback,BOOL bRecvUseFrame)
{
	INTERVALSEND_INFO *pInfo=malloc(sizeof(INTERVALSEND_INFO));
	memset(pInfo,0,sizeof(INTERVALSEND_INFO));
	if (NULL==pInfo) {
		printf("\n NewIntervalsendInfo error : Failed to malloc(INTERVALSEND_INFO)");
		return NULL;
	}
	pInfo->m_pSendframeInfo=NewSendframeInfo(pSend,cbSendSize,pfnRecvCallback,bRecvUseFrame);
	if (NULL==pInfo->m_pSendframeInfo) {
		free(pInfo);
		printf("\n NewIntervalsendInfo error : Failed to malloc(SENDFRAME_INFO.m_pSendframeInfo)");
		return NULL;
	}
	pInfo->m_dwIntervalSec=dwIntervalSec;
	pInfo->m_bUseSendCallback=FALSE;
	return pInfo;
}
INTERVALSEND_INFO* NewIntervalsendInfo_Callback(DWORD dwIntervalSec,LUA_SEND_CALLBACK pfnSendCallback)
{
	INTERVALSEND_INFO *pInfo=malloc(sizeof(INTERVALSEND_INFO));
	memset(pInfo,0,sizeof(INTERVALSEND_INFO));
	if (NULL==pInfo) {
		printf("\n NewIntervalsendInfo_Callback error: Failed to malloc(INTERVALSEND_INFO)");
		return NULL;
	}
	pInfo->m_dwIntervalSec=dwIntervalSec;
	COPY_LUA_CALLBACK(pInfo->m_pfnSendCallback,pfnSendCallback);
	pInfo->m_bUseSendCallback=TRUE;
	
	return pInfo;
}
void FreeIntervalInfo(INTERVALSEND_INFO *pInfo)
{
	if (pInfo) {
		FreeSendframeInfo(pInfo->m_pSendframeInfo);
	}
}
//////////////////////////////////////////////////////////////////////////
INTERVALSEND_INFO* Add(struct INTERVALSEND_LIST* pList,DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,LUA_RECV_CALLBACK pfnRecvCallback,BOOL bRecvUseFrame)
{
	if (!pList) return NULL;
	DWORD dwID=pList->m_dwAddedCount+1;
	TR_LIST *pTRList=&pList->m_TRList;
	INTERVALSEND_INFO *pInfo=NewIntervalsendInfo(dwIntervalSec,pSend,cbSendSize,pfnRecvCallback,bRecvUseFrame);
	if (NULL==pInfo) 
		goto __err;
	
	TR_LIST_NODE *pNode=pNode=pTRList->InsertData(pTRList,pTRList->m_pListTail,pInfo);
	if (NULL==pNode)
		goto __err;
	
	if (!pList->m_pNowNode)
		pList->m_pNowNode=pNode;
	pInfo->m_dwID=dwID;
	pList->m_dwAddedCount++;
	return pInfo;
	
__err:
	if (pNode) {
		pNode->pData=NULL;
		pTRList->DeleteNode(pTRList,pNode);
	}
	FreeIntervalInfo(pInfo);
	printf("\n INTERVALSEND_LIST.Add Error ID(%lu)",dwID);
	return NULL;
}
INTERVALSEND_INFO* Add_Callback(struct INTERVALSEND_LIST* pList,DWORD dwIntervalSec,LUA_SEND_CALLBACK pfnSendCallback)
{
	if (!pList) return NULL;
	DWORD dwID=pList->m_dwAddedCount+1;
	TR_LIST *pTRList=&pList->m_TRList;
	INTERVALSEND_INFO *pInfo=NewIntervalsendInfo_Callback(dwIntervalSec,pfnSendCallback);
	if (NULL==pInfo) 
		goto __err;
	
	TR_LIST_NODE *pNode=pNode=pTRList->InsertData(pTRList,pTRList->m_pListTail,pInfo);
	if (NULL==pNode)
		goto __err;
	
	if (!pList->m_pNowNode)
		pList->m_pNowNode=pNode;
	pInfo->m_dwID=dwID;
	pList->m_dwAddedCount++;
	return pInfo;
	
__err:
	if (pNode) {
		pNode->pData=NULL;
		pTRList->DeleteNode(pTRList,pNode);
	}
	FreeIntervalInfo(pInfo);
	printf("\n INTERVALSEND_LIST.Add_Callback Error ID(%lu)",dwID);
	return NULL;
}
INTERVALSEND_INFO* GetTimeout(struct INTERVALSEND_LIST* pList)
{
	if (!pList) return NULL;
	//TR_LIST *pTRList=&pList->m_TRList;
	time_t tNow=time(NULL);
	TR_LIST_NODE *pNode=pList->m_pNowNode,*pStart=pNode;
	INTERVALSEND_INFO *pInfo=NULL;
	while (pNode && pNode->pNext!=pStart) {
		pInfo=(INTERVALSEND_INFO*)pNode->pData;
		if (pInfo) {
			if (tNow-pInfo->m_tSendTime>pInfo->m_dwIntervalSec)
				return pInfo;
		}
		pNode=pNode->pNext;
	}
	if (pList->m_pNowNode)
		pList->m_pNowNode=pList->m_pNowNode->pNext;
	return NULL;
}
void IntervalSendList_Clear(struct INTERVALSEND_LIST* pList)
{
	if (!pList) return ;
	TR_LIST *pTRList=&pList->m_TRList;
	pTRList->DeleteAll(pTRList);
}
void OnDelete_IntervalsendInfo(void *pData)
{
	INTERVALSEND_INFO *pInfo=(INTERVALSEND_INFO*)pData;
	FreeIntervalInfo(pInfo);
}
void InitIntervalsendList(INTERVALSEND_LIST *pIntervalsendList)
{
	memset(pIntervalsendList,0,sizeof(INTERVALSEND_LIST));
	InitTRList(&pIntervalsendList->m_TRList);
	pIntervalsendList->m_TRList.OnDelNodeData=OnDelete_IntervalsendInfo;
	pIntervalsendList->m_TRList.MakeCircular(&pIntervalsendList->m_TRList);
	pIntervalsendList->Add=Add;
	pIntervalsendList->Add_Callback=Add_Callback;
	pIntervalsendList->GetTimeout=GetTimeout;
	pIntervalsendList->Clear=IntervalSendList_Clear;
}
INTERVALSEND_LIST* NewIntervalsendList()
{
	INTERVALSEND_LIST *pList=malloc(sizeof(INTERVALSEND_LIST));
	if (!pList) {
		printf("\n NewIntervalsendList error : Failed to malloc");
		return NULL;
	}
	InitIntervalsendList(pList);
	return pList;
}
