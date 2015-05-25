// ModbusDev.cpp: implementation of the CPubDev class.
//
//////////////////////////////////////////////////////////////////////

#include "dev.h"
#include "scriptif.h"

IoGlobal * pGlobal=NULL;
SCADAUnit*   pScadaUnit=NULL;


////////////////////////////device des////////公共部分//////////////////////////////////////

int nCookie=0;

int* GetCookie()
{
    return &nCookie;
}
void AddCookie()
{
    nCookie++;
}
void ReleaseCookie()
{
    nCookie--;
}

void FreeExDevice(void* pbyDeviceUnit)
{
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit&&pDeviceUnit->pExDeviceUnit)
	{
		DEV_CLASS	* pPubDev=(DEV_CLASS*)pDeviceUnit->pExDeviceUnit;
		if (pPubDev->OnDestroy)
			pPubDev->OnDestroy(pPubDev);
		FreeLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
		///////////////////////////////////////////	///////////////////////////////
		pPubDev->m_InsertSendList.Clear(&pPubDev->m_InsertSendList);
		pPubDev->m_IntervalsendList.Clear(&pPubDev->m_IntervalsendList);
		/*
		while (pPubDev->m_pYxDelayExplain_Head)
		{
			if (pPubDev->DeleteYxDelayExplainNode(pPubDev,pPubDev->m_pYxDelayExplain_Head)==FALSE)
				break;
		}*/
		//////////////////////////////////////////////////////////////////////////
		free(pPubDev->m_NowPollSend.m_pSend);
		free(pPubDev);
		pDeviceUnit->pExDeviceUnit=NULL;
	}
}


void FinishInitExDevice(void* pbyDeviceUnit)
{
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit)
	{
		DEV_CLASS * pPubDev=(DEV_CLASS*)pDeviceUnit->pExDeviceUnit;
		
		if (!InitCommunication(pPubDev)) {
			printf("\n InitCommunication error");
		}
		pPubDev->m_bInitComplete=TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////

///////////////////////////设备特性部分///////////////////////////////////////////////

static void SetFrameModule(struct DEV_CLASS *pPubDev,const char szFrameModbulName[256])
{
	if (pPubDev->m_bInitComplete){
		DEBUG_PRINT(pPubDev,"Only In InitCommunication to Set FrameModbul!");
		return ;
	}

	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	
	strcpy(pDeviceUnit->m_FrameInterface.m_szFrameModbulName,szFrameModbulName);
	if (strchr(szFrameModbulName,'.')==NULL)
		strcat(pDeviceUnit->m_FrameInterface.m_szFrameModbulName,".so");
	DEBUG_PRINT(pPubDev,"SetFrameModule:%s",pDeviceUnit->m_FrameInterface.m_szFrameModbulName);
	return ;
}

//////////////////////////////////////////////////////////////////////////
/*注册操作*/
static BOOL _YkSelect(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkSelect &&
		pPubDev->YkSelect(pPubDev,wOutPort,bOnOff)) {
		pPubDev->m_LastYkKind=k_Select;
	}
	return FALSE;
}
static BOOL _YkExecute(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkExecute &&
		pPubDev->YkExecute(pPubDev,wOutPort,bOnOff)) {
		pPubDev->m_LastYkKind=k_Execute;
	}
	return FALSE;
}
static BOOL _YkCancel(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkCancel &&
		pPubDev->YkCancel(pPubDev,wOutPort,bOnOff)) {
		pPubDev->m_LastYkKind=k_Cancel;
	}
	return FALSE;
}
static BOOL _SetDeviceTime(struct _DeviceUnit * pDeviceUnit,BOOL bFF)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->SetDeviceTime)
		return pPubDev->SetDeviceTime(pPubDev);
	return FALSE;
}
static BOOL _ResetDeviceStatus(struct _DeviceUnit * pDeviceUnit)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->ResetDeviceStatus)
		return pPubDev->ResetDeviceStatus(pPubDev);
	return FALSE;
}
BOOL _SelectWriteFixValue(struct _DeviceUnit * pDeviceUnit,BYTE * pBuf,int bylen,BYTE byQh)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	WORD wStartIdx=g_DP.GetWordValue(pBuf,0,FALSE);
	WORD wNum=g_DP.GetWordValue(pBuf,1,FALSE);
	pPubDev->m_bCache_FixValue_Writing=FALSE;
	pPubDev->m_byCache_FixValue_QH=byQh;
	TRACE("ReadFixValue:StartIndex=%d,Num=%d",wStartIdx,wNum);
	if (pPubDev->ReadFixValue)
		return pPubDev->ReadFixValue(pPubDev,wStartIdx,wNum);
	return TRUE;
}
BOOL _ExecuteWriteFixValue(struct _DeviceUnit * pDeviceUnit,BYTE * pBuf,int bylen,BYTE byQh)
{
	DEV_CLASS* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (bylen<4)
	{
		TRACE("WriteFixValue:Error:bylen=%d",bylen);
		return FALSE;
	}
	WORD wStartIdx=g_DP.GetWordValue(pBuf,0,FALSE);
	WORD wNum=g_DP.GetWordValue(pBuf,1,FALSE);
	WORD wCalcNum=(bylen-4)/2;
	pPubDev->m_bCache_FixValue_Writing=TRUE;
	pPubDev->m_byCache_FixValue_QH=byQh;
	if (wNum!=wCalcNum)
	{
		TRACE("WriteFixValue: Warning:Num=%d,FixValueArrayLen=%d",wNum,bylen-4);
		wNum=MIN(wNum,wCalcNum);
	}
	TRACE("WriteFixValue:StartIndex=%d,Num=%d",wStartIdx,wNum);
	if (pPubDev->WriteFixValue)
		return pPubDev->WriteFixValue(pPubDev,wStartIdx,wNum,(WORD*)&pBuf[4]);
	return TRUE;
}


static inline void RegisterYkSelect(DEV_CLASS *pPubDev,YKSELECT_CALLBACK pfnYkSelect)
{
	pPubDev->YkSelect=pfnYkSelect;
	pPubDev->pDeviceUnit->YkSelect=_YkSelect;
}
static inline void RegisterYkExecute(DEV_CLASS *pPubDev,YKEXECUTE_CALLBACK pfnYkExecute)
{
	pPubDev->YkExecute=pfnYkExecute;
	pPubDev->pDeviceUnit->YkExecute=_YkExecute;
}
static inline void RegisterYkCancel(DEV_CLASS *pPubDev,YKCANCEL_CALLBACK pfnYkCancel)
{
	pPubDev->YkCancel=pfnYkCancel;
	pPubDev->pDeviceUnit->YkCancel=_YkCancel;
}
static inline void RegisterYkOperations(struct DEV_CLASS *pPubDev,YKOPERATION_CALLBACK pfnYkSelect,YKOPERATION_CALLBACK pfnYkExcute,YKOPERATION_CALLBACK pfnYkCancel)
{
	if (pfnYkSelect)
		pPubDev->RegisterYkSelect(pPubDev,pfnYkSelect);
	if (pfnYkExcute)
		pPubDev->RegisterYkExecute(pPubDev,pfnYkExcute);
	if (pfnYkCancel)
		pPubDev->RegisterYkCancel(pPubDev,pfnYkCancel);
}
static inline void RegisterSetDeviceTime(DEV_CLASS *pPubDev,SETDEVICETIME_CALLBACK pfnSetDeviceTime)
{
	pPubDev->SetDeviceTime=pfnSetDeviceTime;
	pPubDev->pDeviceUnit->SetDeviceTime=_SetDeviceTime;
}
static inline void RegisterResetDevice(DEV_CLASS *pPubDev,RESETDEVICE_CALLBACK pfnResetDevice)
{
	pPubDev->ResetDeviceStatus=pfnResetDevice;
	pPubDev->pDeviceUnit->ResetDeviceStatus=_ResetDeviceStatus;
}
static inline void RegisterReadFixValue(struct DEV_CLASS *pPubDev,READFIXEDVALUE_CALLBACK pfnReadFixValue)
{
	pPubDev->ReadFixValue=pfnReadFixValue;
	pPubDev->pDeviceUnit->SelectWriteFixValue=_SelectWriteFixValue;
}
static inline void RegisterWriteFixValue(struct DEV_CLASS *pPubDev,WRITEFIXEDVALUE_CALLBACK pfnWriteFixValue)
{
	pPubDev->WriteFixValue=pfnWriteFixValue;
	pPubDev->pDeviceUnit->ExecuteWriteFixValue=_ExecuteWriteFixValue;
}

//////////////////////////////////////////////////////////////////////////
/*注册事件*/
static inline void RegisterPrePolling(struct DEV_CLASS *pPubDev,PREPOLLING_CALLBACK pfnPrePolling)
{
	pPubDev->PrePolling=pfnPrePolling;
}

static inline void RegisterDefaultRecvCallback(struct DEV_CLASS *pPubDev,RECEIVE_CALLBACK pfnRecvCallBack)
{
	pPubDev->DefaultRecvCallback=pfnRecvCallBack;
}

static inline void RegisterOnDestroy(struct DEV_CLASS *pPubDev,DESTROY_CALLBACK pfnOnDestroy)
{
	pPubDev->OnDestroy=pfnOnDestroy;
}

static inline void RegisterOnReconnect(struct DEV_CLASS *pPubDev,DESTROY_CALLBACK pfnOnReconnect)
{
	pPubDev->OnReconnect=pfnOnReconnect;
}
//////////////////////////////////////////////////////////////////////////
/*发送接收相关*/

static inline BOOL PollSendFrame(struct DEV_CLASS *pPubDev, BYTE *pSend,WORD wSize,const LUA_RECV_CALLBACK pfnRecvCallBack)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	if (pSend && wSize<BUFFER_SIZE) {
		pPubDev->m_NowPollSend.m_cbSendSize=wSize;
		memcpy(pPubDev->m_NowPollSend.m_pSend,pSend,wSize);
		COPY_LUA_CALLBACK(pPubDev->m_NowPollSend.m_pfnRecvCallBack,pfnRecvCallBack);
		
		pDeviceUnit->PollingRawFrame(pDeviceUnit,pSend,wSize,eCid_PollSendFrame);
		return TRUE;
	}
	return FALSE;
}
static BOOL InsertSendFrame(struct DEV_CLASS *pPubDev,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallBack)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	SENDFRAME_LIST *pInsertSendList=&pPubDev->m_InsertSendList;
	SENDFRAME_INFO *pInsertSend=NULL;
	pInsertSend=pInsertSendList->AddHead(pInsertSendList,pSend,cbSendSize,pfnRecvCallBack);
	if (pInsertSend) {
		pDeviceUnit->InsertRawFrame(pDeviceUnit,pSend,cbSendSize,eCid_InsertSendFrame);
		DEBUG_PRINT(pPubDev,"Insert Frame!!!");
		return TRUE;
	} else {
		TRACE("InsertSendFrame Error");
		return FALSE;
	}
}

static inline BYTE* MakeFrame(struct DEV_CLASS *pPubDev,BYTE byCmd, BYTE *pData,BYTE byFrameType, WORD wDataLen, WORD *pwSize)
{
	struct _DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	CFrameUnit *pSFrameUnit=pDeviceUnit->pSFrameUnit;
	if (!pSFrameUnit || !pSFrameUnit->m_bInit) {
		TRACE("MakeFrame error : SFrameUnit invalid!");
		return NULL;
	}
	BYTE *pBuffer=pPubDev->m_pMakeFrameBuffer;
	WORD wSize=sizeof(pPubDev->m_pMakeFrameBuffer);

	pSFrameUnit->Reset(pSFrameUnit);
	pSFrameUnit->SetupFrame(pSFrameUnit,pPubDev->Get_LinkAddr(pPubDev),byCmd,pData,wDataLen,byFrameType,0);
	if (pSFrameUnit->GetSize(pSFrameUnit)>wSize) {
		TRACE("MakeFrame error : FrameSize(%d)>BufferSize(%d)",
			pSFrameUnit->GetSize(pSFrameUnit),wSize);
		return NULL;
	}
	wSize=pSFrameUnit->FrameToBuffer(pSFrameUnit,pBuffer);
	if (pwSize) *pwSize=wSize;
	return pBuffer;
}
static inline BOOL SendFrame(struct DEV_CLASS *pPubDev, BYTE *pSend,WORD wSize,const LUA_RECV_CALLBACK pfnRecvCallBack)
{
	if (pPubDev->m_bInPollingSend)
		pPubDev->PollSendFrame(pPubDev,pSend,wSize,pfnRecvCallBack);
	else
		pPubDev->InsertSendFrame(pPubDev,pSend,wSize,pfnRecvCallBack);
	
	return TRUE;
}

static inline BYTE Get_RecvCID(struct DEV_CLASS *pPubDev)
{return pPubDev->m_byRecvCID;}
static inline BYTE Get_RecvADDR(struct DEV_CLASS *pPubDev)
{return pPubDev->m_byRecvADDR;}
static inline BYTE Get_RecvFrameType(struct DEV_CLASS *pPubDev)
{return pPubDev->m_byRecvFrameType;}

//////////////////////////////////////////////////////////////////////////
/*间隔发送*/

static INTERVALSEND_INFO* RegisterIntervalSend(struct DEV_CLASS *pPubDev,DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback)
{
	INTERVALSEND_LIST *pIntervalSendList=&pPubDev->m_IntervalsendList;
	INTERVALSEND_INFO *pIntervalSend=NULL;
	pIntervalSend=pIntervalSendList->Add(pIntervalSendList,dwIntervalSec,pSend,cbSendSize,pfnRecvCallback);
	if (NULL==pIntervalSend) {
		TRACE("RegisterIntervalSend error");
	}
	return pIntervalSend;
}

static INTERVALSEND_INFO* RegisterIntervalSend_Callback(struct DEV_CLASS *pPubDev,DWORD dwIntervalSec,const LUA_SEND_CALLBACK pfnSendCallback)
{
	INTERVALSEND_LIST *pIntervalSendList=&pPubDev->m_IntervalsendList;
	INTERVALSEND_INFO *pIntervalSend=NULL;
	pIntervalSend=pIntervalSendList->Add_Callback(pIntervalSendList,dwIntervalSec,pfnSendCallback);
	if (NULL==pIntervalSend) {
		TRACE("RegisterIntervalSend_Callback error");
	}
	return pIntervalSend;
}

//////////////////////////////////////////////////////////////////////////
/*数据处理*/
static inline void ExplainYc(struct DEV_CLASS *pPubDev,WORD wYcindex,float fValue)
{
	BYTE pxxxxExData[8];
	*(long*)pxxxxExData=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYcindex);
	*(float*)&pxxxxExData[4]=(float)(fValue);
	ExplainYC(pGlobal,pxxxxExData,8);
}
static inline void ExplainYx(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL bOnoff)
{
	BYTE pxxxxExData[8];
	*(long*)pxxxxExData=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYxindex);
	*(long*)&pxxxxExData[4]=bOnoff?1:0;
	ExplainYX(pGlobal,pxxxxExData,8);
	//////////////////////////////////////////////////////////////////////////
	//删除延时解析遥信
	/*
	YX_DELAY_EXPLAIN_NODE* pNode;
	if (pNode=pPubDev->FindYxDelayExplainNode(pPubDev,wYxindex))
	{
		pPubDev->DeleteYxDelayExplainNode(pPubDev,pNode);
	}*/
}
/*
static void InsertYxDelayExplainNode(struct DEV_CLASS *pPubDev,YX_DELAY_EXPLAIN_NODE InsertionNode)
{
	YX_DELAY_EXPLAIN_NODE *pNode=(YX_DELAY_EXPLAIN_NODE*)malloc(sizeof(YX_DELAY_EXPLAIN_NODE));
	if (pNode)
	{
		memcpy(pNode,&InsertionNode,sizeof(YX_DELAY_EXPLAIN_NODE));
		if (pPubDev->m_pYxDelayExplain_Tail && 
			pPubDev->m_pYxDelayExplain_Tail->tExplainTime < pNode->tExplainTime)
		{
			pPubDev->m_pYxDelayExplain_Tail->pNext=pNode;
			pPubDev->m_pYxDelayExplain_Tail=pNode;
			pNode->pNext=NULL;
		}
		else
		{
			YX_DELAY_EXPLAIN_NODE *pNow=NULL,*pPrev=NULL;
			pNow=pPubDev->m_pYxDelayExplain_Head;
			while (pNow)
			{
				if (pNow->tExplainTime>pNode->tExplainTime)
					break;
				
				pPrev=pNow;
				pNow=pNow->pNext;
			}
			
			if (pPrev==NULL || pPubDev->m_pYxDelayExplain_Head==NULL)//Head
			{
				pNode->pNext=pPubDev->m_pYxDelayExplain_Head;
				pPubDev->m_pYxDelayExplain_Head=pNode;
				if (pPubDev->m_pYxDelayExplain_Tail==NULL)
					pPubDev->m_pYxDelayExplain_Tail=pPubDev->m_pYxDelayExplain_Head;
			}
			else
			{
				pPrev->pNext=pNode;
				pNode->pNext=pNow;
				
				if (pNow==NULL)
					pPubDev->m_pYxDelayExplain_Tail=pNode;
			}
		}
	}//if (pNode)

}
static BOOL DeleteYxDelayExplainNode(struct DEV_CLASS *pPubDev,YX_DELAY_EXPLAIN_NODE* pInsertion)
{
	if (pInsertion==NULL)
		return FALSE;
	YX_DELAY_EXPLAIN_NODE *pNow=NULL,*pPrev=NULL;
	pNow=pPubDev->m_pYxDelayExplain_Head;
	while(pNow)
	{
		if (pNow==pInsertion)
			break;

		pPrev=pNow;
		pNow=pNow->pNext;
	}

	if (pNow==pPubDev->m_pYxDelayExplain_Head)
	{
		pPubDev->m_pYxDelayExplain_Head=pNow->pNext;
		if (pPubDev->m_pYxDelayExplain_Head==NULL)
			pPubDev->m_pYxDelayExplain_Tail=NULL;
		free(pNow);
		return TRUE;
	}
	else if (pNow)
	{
		pPrev->pNext=pNow->pNext;
		if (pPubDev->m_pYxDelayExplain_Tail==pNow)
			pPubDev->m_pYxDelayExplain_Tail=pPrev;
		free(pNow);
		return TRUE;
	}
	return FALSE;
}
static inline YX_DELAY_EXPLAIN_NODE* FindYxDelayExplainNode(struct DEV_CLASS *pPubDev,WORD wYxindex)
{
	YX_DELAY_EXPLAIN_NODE *pNode=pPubDev->m_pYxDelayExplain_Head;
	while (pNode)
	{
		if (pNode->wYxindex==wYxindex)
			return pNode;
		pNode=pNode->pNext;
	}
	return NULL;
}
static inline void DelayExplainYx(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL bOnoff,WORD nDelay)
{
	YX_DELAY_EXPLAIN_NODE YxDelayExplainNode={0};
	YxDelayExplainNode.wYxindex=wYxindex;
	YxDelayExplainNode.bExplainState=bOnoff;
	YxDelayExplainNode.tExplainTime=time(NULL)+nDelay;
	
	TRACE("Delay Explain YX:Index=%d,State=%d,Delay=%dS",wYxindex,bOnoff,nDelay);
	pPubDev->InsertYxDelayExplainNode(pPubDev,YxDelayExplainNode);
}*/
static inline BOOL GetYcValue(struct DEV_CLASS *pPubDev,WORD wYcindex,float *pfReturn)
{
	LONG nIDNo=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYcindex);
	IoGlobal * pIoGlobal=(IoGlobal *)pGlobal;
	SCADAUnit * pScadaUnit=NULL;
	Analog* pAnalog=NULL;
	if(pIoGlobal)
	{
		pScadaUnit=pIoGlobal->pScadaUnit;
		YCUnit *m_node=pScadaUnit->GetYCNode(pScadaUnit,nIDNo);
		if (m_node==NULL || m_node->m_pAnalog==NULL)
			return FALSE;
		pAnalog=m_node->m_pAnalog;
		*pfReturn=pAnalog->m_RealValue;
		return TRUE;
	}
	return FALSE;
}
static inline BOOL GetYxValue(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL *pfReturn)
{
	LONG nIDNo=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYxindex);
	IoGlobal * pIoGlobal=(IoGlobal *)pGlobal;
	SCADAUnit * pScadaUnit=NULL;
	Digital* pDigital=NULL;
	if(pIoGlobal)
	{
		pScadaUnit=pIoGlobal->pScadaUnit;
		YXUnit *m_node=pScadaUnit->GetYXNode(pScadaUnit,nIDNo);
		if (m_node==NULL || m_node->m_pDigital==NULL)
			return FALSE;
		pDigital=m_node->m_pDigital;
		*pfReturn=pDigital->m_RealValue;
		return TRUE;
	}
	return FALSE;
}
static inline void ExplainYxByte(struct DEV_CLASS *pPubDev,WORD wYxindex,BYTE byte)
{
	int i;
	for (i=0;i<8;i++)
	{
		pPubDev->ExplainYx(pPubDev,wYxindex+i,(byte>>i)&1);
	}
}
static inline void RespondResult_YK(struct DEV_CLASS *pPubDev,enum yk_Kind YkKind,BOOL bSuccess)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	ChannelUnit *pChannelUnit=pPubDev->pChannelUnit;
	BYTE byResult=bSuccess?r_Succeed:r_Failed;

	pDeviceUnit->m_rResult=byResult;
	pDeviceUnit->SetInnerEcho(pScadaUnit,pDeviceUnit,k_Yk,YkKind);
	if (YkKind==k_Execute && bSuccess)
		pChannelUnit->SetKeepOne(pChannelUnit,TRUE,3);
}
static inline void RespondResultEasy_YK(struct DEV_CLASS *pPubDev,BOOL bSuccess)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	ChannelUnit *pChannelUnit=pPubDev->pChannelUnit;
	enum yk_Kind YkKind=pPubDev->m_LastYkKind;
	BYTE byResult=bSuccess?r_Succeed:r_Failed;
	
	pDeviceUnit->m_rResult=byResult;
	pDeviceUnit->SetInnerEcho(pScadaUnit,pDeviceUnit,k_Yk,YkKind);
	if (YkKind==k_Execute && bSuccess)
		pChannelUnit->SetKeepOne(pChannelUnit,TRUE,3);
}
static inline void RespondResult_FixValue(struct DEV_CLASS * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[])
{
	if (pPubDev->m_byCache_FixValue_QH==0)
	{
		//内部规约
		WORD wSize=4+wNum*2;
		BYTE *pData=malloc(wSize);
		if (pData==NULL)
		{
			TRACE("RespondResult_FixValue: malloc Failed");
			return;
		}
		pData[0]=pPubDev->m_bCache_FixValue_Writing?k_Execute:k_Select;//func
		pData[1]=pPubDev->m_byCache_FixValue_QH;//qh
		g_DP.SetWordValue(pData,1,wStartIdx,FALSE);
		g_DP.SetWordValue(pData,2,wNum,FALSE);
		pPubDev->InnerChannelSendData(pPubDev,pData,wSize,k_FixValue);
		free(pData);
	}
}
static inline BOOL InitSoeUnit(struct DEV_CLASS *pPubDev,SOE_UNIT *pSoeUnit)
{
	if (pSoeUnit==NULL)
		return FALSE;

	pSoeUnit->m_Time=pPubDev->GetDateTime(pPubDev);
	
	pSoeUnit->m_bYxState=0;
	pSoeUnit->m_nYxIndex=0;
	pSoeUnit->m_fValue=0.0;
	return TRUE;
}

static inline BOOL ExplainSoe(struct DEV_CLASS *pPubDev,SOE_UNIT *pSoeUnit)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	ChannelUnit *pChannelUnit=pPubDev->pChannelUnit;
	DataTable * pDataTable=pDeviceUnit->pDataTable;

	if (!pSoeUnit||!pDataTable||!pDataTable->pYXTable)
		return FALSE;

	YXTable *pYXTable=pDataTable->pYXTable;


	struct timeval tv;
	tv.tv_sec=time(NULL);
	struct tm* p=localtime(&tv.tv_sec);

	YXUnit * pYXUnit;
	YXSoeUnit * pYXSoeUnit=(YXSoeUnit*)malloc(sizeof(YXSoeUnit));
	if(!pScadaUnit||!pScadaUnit->SendSOE)
	{
		free(pYXSoeUnit);
		return FALSE;
	}

	TIME_UNIT *pTimeUnit=&pSoeUnit->m_Time;
	p->tm_year=pTimeUnit->Year-1900;
	p->tm_mon=pTimeUnit->Mon-1;
	p->tm_mday=pTimeUnit->Mday;
	p->tm_hour=pTimeUnit->Hour;
	p->tm_min=pTimeUnit->Min;
	p->tm_sec=pTimeUnit->Sec;
	tv.tv_sec=timelocal(p);
	tv.tv_usec=(pTimeUnit->MS)*1000;//微秒



	LONG nIDNo;
	SetIDNo(ID_IDLDEVICE,pChannelUnit->m_ChannelNo,pDeviceUnit->m_DeviceNo,pSoeUnit->m_nYxIndex,&nIDNo);
	pYXUnit=pYXTable->GetYXNode(pYXTable,nIDNo);
	if (!pYXUnit) 
	{
		free(pYXSoeUnit);
		return FALSE;
	}
	memcpy(&pYXSoeUnit->m_SoeTime,&tv,sizeof(struct timeval));
	pYXSoeUnit->ToValue=pSoeUnit->m_bYxState?1:0;
	pYXSoeUnit->pYXUnit=pYXUnit;
	pYXSoeUnit->pNext=NULL;
	pScadaUnit->SendSOE(pScadaUnit,pYXSoeUnit);
	
	DEBUG_PRINT(pPubDev,"ExplainSoe: %d-%d-%d %d:%d:%d.%d;Index:%ld,State:%ld,Value:%.2f",
		pTimeUnit->Year,pTimeUnit->Mon,pTimeUnit->Mday,
		pTimeUnit->Hour,pTimeUnit->Min,pTimeUnit->Sec,pTimeUnit->MS,
		pSoeUnit->m_nYxIndex,pSoeUnit->m_bYxState,pSoeUnit->m_fValue);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
/*设备信息*/
static inline TIME_UNIT GetDateTime(struct DEV_CLASS *pPubDev)
{
	TIME_UNIT DataTime;
	
	struct timeval tv={0};
	tv.tv_sec=time(NULL);
	struct tm* p=localtime(&tv.tv_sec);
	
	DataTime.Year=p->tm_year+1900;
	DataTime.Mon=p->tm_mon+1;
	DataTime.Mday=p->tm_mday;
	DataTime.Wday=p->tm_wday;
	DataTime.Yday=p->tm_yday;
	DataTime.Hour=p->tm_hour;
	DataTime.Min=p->tm_min;
	DataTime.Sec=p->tm_sec;
	DataTime.MS=tv.tv_usec/1000%1000;
	
	return DataTime;
}

int Get_DeviceNo(struct DEV_CLASS *pPubDev)
{return pPubDev->pDeviceUnit->m_DeviceNo;};
BYTE Get_ChannelNo(struct DEV_CLASS *pPubDev)
{return pPubDev->pChannelUnit->m_ChannelNo;}
static inline BYTE Get_LinkAddr(struct DEV_CLASS *pPubDev)
{return pPubDev->pDeviceUnit->m_LinkAddr;}
static inline void Set_LinkAddr(struct DEV_CLASS *pPubDev,BYTE byAddr)
{pPubDev->pDeviceUnit->m_LinkAddr=byAddr;}
static inline const char*Get_LinkAddr2(struct DEV_CLASS *pPubDev)
{return pPubDev->pDeviceUnit->m_LinkAddr2;}
static inline const char*Get_Param1(struct DEV_CLASS *pPubDev)
{return pPubDev->pDeviceUnit->m_Param1;}
static inline const char*Get_Param2(struct DEV_CLASS *pPubDev)
{return pPubDev->pDeviceUnit->m_Param2;}

//////////////////////////////////////////////////////////////////////////
/*设备操作*/
static inline void InnerChannelSendData(struct DEV_CLASS *pPubDev,BYTE *pData,int nSize,BYTE byCID)
{
	BYTE *pSend=malloc(nSize+3);
	if (pSend==NULL) 
	{
		TRACE("InnerChannelSendData: malloc Failed");
		return;
	}
	pSend[0]=pScadaUnit->m_BaseChannelNo;
	pSend[1]=pPubDev->pChannelUnit->m_ChannelNo;
	pSend[2]=pPubDev->pDeviceUnit->m_DeviceNo;
	memcpy(pSend+3,pData,nSize);
	InnerChannelMailData(pGlobal,byCID,0xff,pSend,3+nSize);
	free(pSend);
}
static inline void SetKeepSend(struct DEV_CLASS *pPubDev,int iKeepSec)
{pPubDev->pChannelUnit->SetKeepOne(pPubDev->pChannelUnit,TRUE,iKeepSec);}
static inline void ReleaseKeepSend(struct DEV_CLASS *pPubDev)
{pPubDev->pChannelUnit->SetKeepOne(pPubDev->pChannelUnit,FALSE,0);}

//////////////////////////////////////////////////////////////////////////
/*接口*/
static BOOL ExplainLinkData(struct _DeviceUnit * pDeviceUnit,BYTE *pBuf,WORD bySize)
{
//	printf("ExplainLinkData");
	//只要帧校验通过就需要返回TRUE
	if(!pDeviceUnit
		||!pDeviceUnit->pExDeviceUnit
		||!pDeviceUnit->pChannel)
		return FALSE;
	
	DEV_CLASS * pPubDev=(DEV_CLASS*)pDeviceUnit->pExDeviceUnit;
	ChannelUnit * pChannelUnit=pDeviceUnit->pChannel;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pChannelUnit->m_ChannelNo);
	if (!pLuaChannel) return FALSE;

	CFrameUnit *pRFrame=pDeviceUnit->pRFrameUnit;
	SENDFRAME_LIST *pInsertSendList=&pPubDev->m_InsertSendList;
	SENDFRAME_INFO *pSendInfo=NULL;
	BYTE mCID;
	BYTE mAddr;
	BYTE * pData=NULL;
	int datalen=0;
	
	if (pDeviceUnit->m_EchoCID==eCid_PollSendFrame) /*轮训*/ {
		pSendInfo=&pPubDev->m_NowPollSend;
	}
	else if (pDeviceUnit->m_EchoCID==eCid_InsertSendFrame) /*插帧*/ { 
		pSendInfo=pInsertSendList->GetTail(pInsertSendList);
	}

	if (NULL==pSendInfo) {
		TRACE("ExplainLinkData error : Invalid pSendInfo , EchoCID(%d)",pDeviceUnit->m_EchoCID);
		return FALSE;
	}
	
	if(pRFrame)
	{
		if (pRFrame->BufferToFrame&&pRFrame->BufferToFrame(pRFrame,pBuf,bySize)) {
			pPubDev->m_byRecvFrameType=pRFrame->m_FrameType;
			mCID=pPubDev->m_byRecvCID=pRFrame->m_CID;
			mAddr=pPubDev->m_byRecvADDR=pRFrame->m_Addr;
			pData=pRFrame->m_pData;
			datalen=pRFrame->m_wLength;
			if (mAddr!=pPubDev->Get_LinkAddr(pPubDev))
			{
				DEBUG_PRINT(pPubDev,"ExplainLink Addr Error!");
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	} else {
		pData=pBuf;
		datalen=bySize;
	}
	
	BOOL bRet=TRUE;
	if (LUA_CALLBACK_VALID(pSendInfo->m_pfnRecvCallBack)) {
		bRet = (HandleOnRecv(pPubDev,pSendInfo->m_pfnRecvCallBack,pData,datalen)>0);
	} else if (pPubDev->DefaultRecvCallback) {
		bRet = pPubDev->DefaultRecvCallback(pPubDev,pData,datalen);
	}

	if (bRet && !pPubDev->m_bConnectionState && pPubDev->OnReconnect)
	{
		pPubDev->m_bConnectionState=TRUE;
		pPubDev->OnReconnect(pPubDev);
	}

	if (pSendInfo==pInsertSendList->GetTail(pInsertSendList))
		pInsertSendList->DelTail(pInsertSendList);
	return bRet;
}

static BOOL RunPolling(struct _DeviceUnit * pDeviceUnit)
{
	if(!pDeviceUnit||!pDeviceUnit->pExDeviceUnit||!pDeviceUnit->pChannel)
		return FALSE;
	
	ChannelUnit * pChannelUnit=pDeviceUnit->pChannel;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pChannelUnit->m_ChannelNo);
	if (!pLuaChannel) return FALSE;
	DEV_CLASS * pPubDev=(DEV_CLASS*)pDeviceUnit->pExDeviceUnit;	
	//////////////////////////////////////////////////////////////////////////
	if (!pPubDev->m_bInitialized) {
		pPubDev->m_bInitialized=TRUE;
		HandleOnInit(pPubDev);
	}
	//Delay Explain Yx
	/*
	if (pPubDev->m_pYxDelayExplain_Head)
	{
		while (pPubDev->m_pYxDelayExplain_Head && 
			pPubDev->m_pYxDelayExplain_Head->tExplainTime<time(NULL))
		{
			YX_DELAY_EXPLAIN_NODE *pNode=pPubDev->m_pYxDelayExplain_Head;
			
			TRACE("Explain YX(Delay):Index=%d,State=%d",pNode->wYxindex,pNode->bExplainState);
			pPubDev->ExplainYx(pPubDev,pNode->wYxindex,pNode->bExplainState);
			if (pPubDev->DeleteYxDelayExplainNode(pPubDev,pNode)==FALSE)
				free(pNode);//手动释放
		}
	}*/
	//////////////////////////////////////////////////////////////////////////
	if (pPubDev->pDeviceUnit->m_bBreakStatus)
		pPubDev->m_bConnectionState=FALSE;
		
	pPubDev->m_bInPollingSend=TRUE;
	
	if (pPubDev->PrePolling!=NULL && pPubDev->PrePolling(pPubDev))
	{
		goto __Polling_End;
	}

	SENDFRAME_INFO *pSendframeInfo=NULL;
	INTERVALSEND_LIST *pIntervalSendList=&pPubDev->m_IntervalsendList;
	INTERVALSEND_INFO *pIntervalSend=pIntervalSendList->GetTimeout(pIntervalSendList);
	if (pIntervalSend) {
		if (pIntervalSend->m_bUseSendCallback) {
			if (HandleOnSend(pPubDev,pIntervalSend->m_pfnSendCallback)>0)
				goto __Polling_End;
		}
		else if (pIntervalSend->m_pSendframeInfo) {
			pSendframeInfo=pIntervalSend->m_pSendframeInfo;
			if (pPubDev->SendFrame(pPubDev,
				pSendframeInfo->m_pSend,
				pSendframeInfo->m_cbSendSize,
				pSendframeInfo->m_pfnRecvCallBack)) {
				goto __Polling_End;
			}
		}
	}
	/*
	if (pPubDev->m_pPollSendFrame_Now==NULL)
	{
		if (pPubDev->m_pPollSendFrame_Head)
			pPubDev->m_pPollSendFrame_Now=pPubDev->m_pPollSendFrame_Head;
		else goto __Error_End;
	}
	else
	{
		if (pPubDev->m_pPollSendFrame_Now->m_pNext==NULL)
		{
			TRACE("ERROR:POLL链表已断开");
			goto __Error_End;
		}
		pPubDev->m_pPollSendFrame_Now=pPubDev->m_pPollSendFrame_Now->m_pNext;
	}
	
	SENDFRAME_INFO *pPollInfo=&(pPubDev->m_pPollSendFrame_Now->m_PollSendFrameInfo);
	pPubDev->PollSendFrame(pPubDev,pPollInfo->m_byFrameType,pPollInfo->m_byCmd,pPollInfo->m_pSend,pPollInfo->m_cbSendSize,pPollInfo->m_pfnReceiveCallback);
	*/
	
//__Error_End:
	pPubDev->m_bInPollingSend=FALSE;
	return FALSE;

__Polling_End:
	pPubDev->m_bInPollingSend=FALSE;
	return TRUE;

}

static void InitPubDevDev(DEV_CLASS	* pPubDev)
{
	if(pPubDev)
	{
		memset(pPubDev,0,sizeof(DEV_CLASS));
		//////////////////////////////////////////////////////////////////////////
		/*注册操作*/
		pPubDev->RegisterYkSelect=RegisterYkSelect;
		pPubDev->RegisterYkExecute=RegisterYkExecute;
		pPubDev->RegisterYkCancel=RegisterYkCancel;
		pPubDev->RegisterYkOperations=RegisterYkOperations;
		pPubDev->RegisterSetDeviceTime=RegisterSetDeviceTime;
		pPubDev->RegisterResetDevice=RegisterResetDevice;
		pPubDev->RegisterReadFixValue=RegisterReadFixValue;
		pPubDev->RegisterWriteFixValue=RegisterWriteFixValue;
		//////////////////////////////////////////////////////////////////////////
		/*注册事件*/
		pPubDev->RegisterPrePolling=RegisterPrePolling;
		pPubDev->RegisterOnDestroy=RegisterOnDestroy;
		pPubDev->RegisterOnReconnect=RegisterOnReconnect;
		pPubDev->RegisterDefaultRecvCallback=RegisterDefaultRecvCallback;
		//////////////////////////////////////////////////////////////////////////
		/*发送接收相关*/
		//pPubDev->RegisterPollSendFrame=RegisterPollSendFrame;
		//pPubDev->RegisterPollSendFrame_ModbusAsk=RegisterPollSendFrame_ModbusAsk;
		//pPubDev->DestroyPollSendFrame=DestroyPollSendFrame;
		pPubDev->MakeFrame=MakeFrame;
		pPubDev->PollSendFrame=PollSendFrame;
		pPubDev->InsertSendFrame=InsertSendFrame;
		pPubDev->SendFrame=SendFrame;
		pPubDev->Get_RecvCID=Get_RecvCID;
		pPubDev->Get_RecvADDR=Get_RecvADDR;
		pPubDev->Get_RecvFrameType=Get_RecvFrameType;
		//////////////////////////////////////////////////////////////////////////
		/*间隔发送*/
		pPubDev->RegisterIntervalSend=RegisterIntervalSend;
		pPubDev->RegisterIntervalSend_Callback=RegisterIntervalSend_Callback;
		//////////////////////////////////////////////////////////////////////////
		/*数据处理*/		
		pPubDev->RespondResult_YK=RespondResult_YK;
		pPubDev->RespondResultEasy_YK=RespondResultEasy_YK;
		pPubDev->RespondResult_FixValue=RespondResult_FixValue;
		pPubDev->GetYcValue=GetYcValue;
		pPubDev->GetYxValue=GetYxValue;
		pPubDev->ExplainYc=ExplainYc;
		pPubDev->ExplainYx=ExplainYx;
		pPubDev->ExplainYxByte=ExplainYxByte;
		pPubDev->InitSoeUnit=InitSoeUnit;
		pPubDev->ExplainSoe=ExplainSoe;
		/*
		pPubDev->InsertYxDelayExplainNode=InsertYxDelayExplainNode;
		pPubDev->DeleteYxDelayExplainNode=DeleteYxDelayExplainNode;
		pPubDev->FindYxDelayExplainNode=FindYxDelayExplainNode;
		pPubDev->DelayExplainYx=DelayExplainYx;*/
		//////////////////////////////////////////////////////////////////////////
		/*设备信息*/
		pPubDev->GetDateTime=GetDateTime;
		pPubDev->Get_DeviceNo=Get_DeviceNo;
		pPubDev->Get_ChannelNo=Get_ChannelNo;
		pPubDev->Get_LinkAddr=Get_LinkAddr;
		pPubDev->Set_LinkAddr=Set_LinkAddr;
		pPubDev->Get_LinkAddr2=Get_LinkAddr2;
		pPubDev->Get_Param1=Get_Param1;
		pPubDev->Get_Param2=Get_Param2;
		//////////////////////////////////////////////////////////////////////////
		/*设备操作*/
		pPubDev->SetFrameModule=SetFrameModule;
		pPubDev->InnerChannelSendData=InnerChannelSendData;
		pPubDev->SetKeepSend=SetKeepSend;
		pPubDev->ReleaseKeepSend=ReleaseKeepSend;
		
		
		InitSendframeList(&pPubDev->m_InsertSendList);
		InitIntervalsendList(&pPubDev->m_IntervalsendList);
		pPubDev->m_NowPollSend.m_pSend=pPubDev->m_pSendBuffer;
	}
}


void InitExDevice(void* pbyDeviceUnit,void *pbyIoGlobal)
{
	pGlobal=(IoGlobal*)pbyIoGlobal;
	if(!pGlobal||!pGlobal->pScadaUnit)
		return;
	pScadaUnit=pGlobal->pScadaUnit;
	
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit)
	{
		DEV_CLASS	* pPubDev=(DEV_CLASS*)malloc(sizeof(DEV_CLASS));
		InitPubDevDev(pPubDev);

		pPubDev->pDeviceUnit=(DeviceUnit*)pDeviceUnit;
		pPubDev->pChannelUnit=(ChannelUnit*)pDeviceUnit->pChannel;
		NewLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
		if (LoadDevScript(pPubDev,pDeviceUnit->m_FrameInterface.m_szFrameModbulName)>0) {
			//Load success
		} else {
			pDeviceUnit->m_FrameInterface.m_szFrameModbulName[0]='\0';
		}

		pDeviceUnit->pExDeviceUnit=pPubDev;
		pDeviceUnit->RunPolling=RunPolling;
		pDeviceUnit->ExplainLinkData=ExplainLinkData;		
	}
}


