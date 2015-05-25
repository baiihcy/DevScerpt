#include "types.h"
#include "trlist.h"

typedef struct SENDFRAME_INFO
{
	DWORD m_dwID;
	BYTE *m_pSend;
	WORD m_cbSendSize;
	//BOOL (*m_pfnReceiveCallback)(struct _PubDev *pPubDev,BYTE *pSend,int nSize) ;
	LUA_RECV_CALLBACK m_pfnRecvCallBack; /*lua script*/
}SENDFRAME_INFO;

typedef struct SENDFRAME_LIST
{
	TR_LIST m_TRList;
	DWORD m_dwAddedCount;

	SENDFRAME_INFO* (*AddHead)(struct SENDFRAME_LIST* pList,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback);
	void (*DelTail)(struct SENDFRAME_LIST* pList);
	SENDFRAME_INFO* (*GetTail)(struct SENDFRAME_LIST* pList);
	void (*Clear)(struct SENDFRAME_LIST* pList);
}SENDFRAME_LIST;

typedef struct INTERVALSEND_INFO
{
	DWORD m_dwID;
	time_t m_tSendTime;
	DWORD m_dwIntervalSec;
	
	BOOL m_bUseSendCallback;
	LUA_SEND_CALLBACK m_pfnSendCallback;
	SENDFRAME_INFO *m_pSendframeInfo;
}INTERVALSEND_INFO;

typedef struct INTERVALSEND_LIST
{
	TR_LIST m_TRList;
	TR_LIST_NODE *m_pNowNode;
	DWORD m_dwAddedCount;

	INTERVALSEND_INFO* (*Add)(struct INTERVALSEND_LIST* pList,DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback);
	INTERVALSEND_INFO* (*Add_Callback)(struct INTERVALSEND_LIST* pList,DWORD dwIntervalSec,const LUA_SEND_CALLBACK pfnSendCallback);
	INTERVALSEND_INFO* (*GetTimeout)(struct INTERVALSEND_LIST* pList);
	void (*Clear)(struct INTERVALSEND_LIST* pList);

}INTERVALSEND_LIST;

SENDFRAME_INFO* NewSendframeInfo(BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback);
void FreeSendframeInfo(SENDFRAME_INFO *pInfo);
void InitSendframeList(SENDFRAME_LIST *pSendframeList);
SENDFRAME_LIST* NewSendframeList();

INTERVALSEND_INFO* NewIntervalsendInfo(DWORD dwIntervalSec,BYTE *pSend,WORD cbSendSize,const LUA_RECV_CALLBACK pfnRecvCallback);
INTERVALSEND_INFO* NewIntervalsendInfo_Callback(DWORD dwIntervalSec,const LUA_SEND_CALLBACK pfnSendCallback);
void FreeIntervalInfo(INTERVALSEND_INFO *pInfo);
void InitIntervalsendList(INTERVALSEND_LIST *pIntervalsendList);
INTERVALSEND_LIST* NewIntervalsendList();
