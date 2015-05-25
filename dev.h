
#ifndef H_DEVICE
#define H_DEVICE

#include "types.h"
#include "sendlist.h"
#include "main.h"
#include "DataProcess.h"

#define BUFFER_SIZE 1024
#define MAX_CHANNEL 32

enum
{
	eCid_PollSendFrame=0xf0,
	eCid_InsertSendFrame=0xff
};

/*
//遥信延迟解析，暂时不用
typedef struct YX_DELAY_EXPLAIN_NODE_TAG
{
	WORD wYxindex;
	BYTE bExplainState;
	time_t tExplainTime;
	struct YX_DELAY_EXPLAIN_NODE_TAG *pNext;
}YX_DELAY_EXPLAIN_NODE;
*/
//////////////////////////////////////////////////////////////////////////
typedef struct DEV_CLASS
{
	DeviceUnit * pDeviceUnit;
	ChannelUnit *pChannelUnit;
	//////////////////////////////////////////////////////////////////////////
	//LUA SCRIPT
	void *pScriptClassInfo;
	//////////////////////////////////////////////////////////////////////////
	/*注册操作*/
	void (*RegisterYkSelect)(struct DEV_CLASS *pPubDev,YKSELECT_CALLBACK pfnYkSelect);
	void (*RegisterYkExecute)(struct DEV_CLASS *pPubDev,YKEXECUTE_CALLBACK pfnYkExcute);
	void (*RegisterYkCancel)(struct DEV_CLASS *pPubDev,YKCANCEL_CALLBACK pfnYkCancel);
	void (*RegisterYkOperations)(struct DEV_CLASS *pPubDev,YKOPERATION_CALLBACK pfnYkSelect,YKOPERATION_CALLBACK pfnYkExcute,YKOPERATION_CALLBACK pfnYkCancel);
	void (*RegisterSetDeviceTime)(struct DEV_CLASS *pPubDev,SETDEVICETIME_CALLBACK pfnSetDeviceTime);
	void (*RegisterResetDevice)(struct DEV_CLASS *pPubDev,RESETDEVICE_CALLBACK pfnResetDevice);
	void (*RegisterReadFixValue)(struct DEV_CLASS *pPubDev,READFIXEDVALUE_CALLBACK pfnReadFixValue);
	void (*RegisterWriteFixValue)(struct DEV_CLASS *pPubDev,WRITEFIXEDVALUE_CALLBACK pfnWriteFixValue);
	//////////////////////////////////////////////////////////////////////////
	/*注册事件*/
	void (*RegisterPrePolling)(struct DEV_CLASS *pPubDev,PREPOLLING_CALLBACK pfnPolling);
	void (*RegisterDefaultRecvCallback)(struct DEV_CLASS *pPubDev,RECEIVE_CALLBACK pfnRecvCallBack);
	void (*RegisterOnDestroy)(struct DEV_CLASS *pPubDev,DESTROY_CALLBACK pfnOnDestroy);
	void (*RegisterOnReconnect)(struct DEV_CLASS *pPubDev,DESTROY_CALLBACK pfnOnDestroy);
	//////////////////////////////////////////////////////////////////////////
	/*发送接收相关*/
	//SENDFRAME_LIST_NODE* (*RegisterPollSendFrame)(struct DEV_CLASS *pPubDev,BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack);
	//SENDFRAME_LIST_NODE* (*RegisterPollSendFrame_ModbusAsk)(struct _PubDev *pPubDev,BYTE byCmd, WORD wRegAddr,WORD wRegNum,RECEIVE_CALLBACK pfnRecvCallBack);
	//BOOL (*DestroyPollSendFrame)(struct DEV_CLASS *pPubDev,SENDFRAME_LIST *pPollSendFrameNode);
	BOOL (*PollSendFrame)(struct DEV_CLASS *pPubDev, BYTE *pSend,WORD wSize,const LUA_RECV_CALLBACK pfnRecvCallBack);
	BOOL (*InsertSendFrame)(struct DEV_CLASS *pPubDev, BYTE *pSend,WORD wSize,const LUA_RECV_CALLBACK pfnRecvCallBack);
	void (*ClearInsertSendFrame)(struct DEV_CLASS *pPubDev);
	BYTE* (*MakeFrame)(struct DEV_CLASS *pPubDev,BYTE byCmd, BYTE *pData,BYTE byFrameType, WORD wDataLen, WORD *pwSize);
	BOOL (*SendFrame)(struct DEV_CLASS *pPubDev, BYTE *pSend,WORD wSize,const LUA_RECV_CALLBACK pfnRecvCallBack);
	BYTE (*Get_RecvCID)(struct DEV_CLASS *pPubDev);
	BYTE (*Get_RecvADDR)(struct DEV_CLASS *pPubDev);
	BYTE (*Get_RecvFrameType)(struct DEV_CLASS *pPubDev);
	//////////////////////////////////////////////////////////////////////////
	/*间隔发送*/
	INTERVALSEND_INFO* (*RegisterIntervalSend)(struct DEV_CLASS *pPubDev,DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback);
	INTERVALSEND_INFO* (*RegisterIntervalSend_Callback)(struct DEV_CLASS *pPubDev,DWORD dwIntervalSec,const LUA_SEND_CALLBACK pfnSendCallback);
	/* 已经封装到INTERVALSEND_LIST
	INTERVALSEND_LIST_NODE* (*RegisterIntervalSend)(struct DEV_CLASS *pPubDev,BOOL bOnceOnly,WORD wIntervalSec,INTERVALSEND_CALLBACK pfnIntervalSendCallBack);
	BOOL (*DestroyIntervalSend)(struct DEV_CLASS *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendListNode);
	BOOL (*IntervalSend_Polling)(struct DEV_CLASS *pPubDev);
	BOOL (*IntervalSend_Init)(struct DEV_CLASS *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendListNode);
	*/
	//////////////////////////////////////////////////////////////////////////
	/*数据处理*/
	BOOL (*GetYcValue)(struct DEV_CLASS *pPubDev,WORD wYcindex,float *pfReturn);
	BOOL (*GetYxValue)(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL *pfReturn);
	void (*RespondResult_YK)(struct DEV_CLASS *pPubDev,enum yk_Kind YkKind,BOOL bSuccess);
	void (*RespondResultEasy_YK)(struct DEV_CLASS *pPubDev,BOOL bSuccess);
	void (*RespondResult_FixValue)(struct DEV_CLASS * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[]);
	void (*ExplainYc)(struct DEV_CLASS *pPubDev,WORD wYcindex,float fValue);
	void (*ExplainYx)(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL bOnoff);
	void (*ExplainYxByte)(struct DEV_CLASS *pPubDev,WORD wYxindex,BYTE byte);
	BOOL (*InitSoeUnit)(struct DEV_CLASS *pPubDev,SOE_UNIT *pSoeUnit);
	BOOL (*ExplainSoe)(struct DEV_CLASS *pPubDev,SOE_UNIT *pSoeUnit);
	//void (*InsertYxDelayExplainNode)(struct DEV_CLASS *pPubDev,YX_DELAY_EXPLAIN_NODE InsertionNode);
	//BOOL (*DeleteYxDelayExplainNode)(struct DEV_CLASS *pPubDev,YX_DELAY_EXPLAIN_NODE* pInsertion);
	//YX_DELAY_EXPLAIN_NODE* (*FindYxDelayExplainNode)(struct DEV_CLASS *pPubDev,WORD wYxindex);
	//void (*DelayExplainYx)(struct DEV_CLASS *pPubDev,WORD wYxindex,BOOL bOnoff,WORD nDelay);
	//////////////////////////////////////////////////////////////////////////
	/*设备信息*/
	TIME_UNIT (*GetDateTime)(struct DEV_CLASS *pPubDev);
	BYTE (*Get_ChannelNo)(struct DEV_CLASS *pPubDev);
	int (*Get_DeviceNo)(struct DEV_CLASS *pPubDev);
	BYTE (*Get_LinkAddr)(struct DEV_CLASS *pPubDev);
	const char* (*Get_LinkAddr2)(struct DEV_CLASS *pPubDev);
	const char* (*Get_Param1)(struct DEV_CLASS *pPubDev);
	const char* (*Get_Param2)(struct DEV_CLASS *pPubDev);
	void (*Set_LinkAddr)(struct DEV_CLASS *pPubDev,BYTE byAddr);
	//////////////////////////////////////////////////////////////////////////
	/*设备操作*/
	void (*SetFrameModule)(struct DEV_CLASS *pPubDev,const char szFrameModbulName[256]);
	void (*InnerChannelSendData)(struct DEV_CLASS *pPubDev,BYTE *pData,int nSize,BYTE byCID);
	void (*SetKeepSend)(struct DEV_CLASS *pPubDev,int iKeepSec);
	void (*ReleaseKeepSend)(struct DEV_CLASS *pPubDev);
	//////////////////////////////////////////////////////////////////////////
	//User-Defined Fuction
	BOOL (*PrePolling)(struct DEV_CLASS *pPubDev);
	BOOL (*DefaultRecvCallback)(struct DEV_CLASS *pPubDev,BYTE *pRecv,int nSize);
	BOOL (*YkSelect)(struct DEV_CLASS * pPubDev,WORD wOutPort,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*YkExecute)(struct DEV_CLASS * pPubDev,WORD wOutPort,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*YkCancel)(struct DEV_CLASS * pPubDev,WORD byOut,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*ReadFixValue)(struct DEV_CLASS * pPubDev,WORD wStartIdx,WORD wNum);
	BOOL (*WriteFixValue)(struct DEV_CLASS * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[]);
	BOOL (*SetDeviceTime)(struct DEV_CLASS* pPubDev);
	BOOL (*ResetDeviceStatus)(struct DEV_CLASS * pPubDev);
	void (*OnDestroy)(struct DEV_CLASS *pPubDev);
	void (*OnReconnect)(struct DEV_CLASS *pPubDev);
	//////////////////////////////////////////////////////////////////////////

	/*发送接收信息 属性*/
	BOOL m_bInitialized;
	BOOL m_bInPollingSend;
	BYTE m_byRecvCID;
	BYTE m_byRecvADDR;
	BYTE m_byRecvFrameType;
	BYTE m_pMakeFrameBuffer[BUFFER_SIZE];
	BYTE m_pSendBuffer[BUFFER_SIZE];
	SENDFRAME_INFO m_NowPollSend;
	SENDFRAME_LIST m_InsertSendList;
	
	/*间隔发送 属性*/
	INTERVALSEND_LIST m_IntervalsendList;
	
	
	/*延迟遥信解析 属性*/
	/*暂不开启
	YX_DELAY_EXPLAIN_NODE  *m_pYxDelayExplain_Head;
	YX_DELAY_EXPLAIN_NODE  *m_pYxDelayExplain_Tail;
	*/

	/*设备 属性*/
	BOOL m_bRawMode;
	BOOL m_bInitComplete;
	BOOL m_bConnectionState;
	enum yk_Kind m_LastYkKind;

	/*定值 属性*/
	BOOL m_bCache_FixValue_Writing;
	BYTE m_byCache_FixValue_QH;
	//////////////////////////////////////////////////////////////////////////

	//DEVICE_VARIABLES Vars;
}DEV_CLASS;

#endif
