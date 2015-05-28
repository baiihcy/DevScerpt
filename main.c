#include "dev.h"
#include "main.h"
#include "luascript.h"
#include "scriptif.h"

// BOOL PREPOLLING_CALLBACK(struct _PubDev *pPubDev)//Polling回调函数
// BOOL RECEIVE_CALLBACK(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)//报文接收回调函数
// BOOL INTERVALSEND_CALLBACK(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pSendNode)//间隔发送回调报文
// BOOL YKOPERATION_CALLBACK(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)//遥控操作回调函数 bOnOff TRUE为分闸，FALSE为合闸
// BOOL SETDEVICETIME_CALLBACK(struct _PubDev * pPubDev)//装置对时回调函数
// BOOL RESETDEVICE_CALLBACK(struct _PubDev * pPubDev)//装置复归回调函数
// g_DP 数据处理对象，直接使用即可

BOOL PrePolling(struct DEV_CLASS *pPubDev)
{
	return HandleOnSend(pPubDev,NULL);
}
BOOL DefaultRecv(struct DEV_CLASS *pPubDev,BYTE *pRecv,int nSize)
{
	return HandleOnRecv(pPubDev,NULL,pRecv,nSize);
}
BOOL ResetDevice(struct DEV_CLASS * pPubDev)
{
	return HandleOnReset(pPubDev);
}
BOOL SetTime(struct DEV_CLASS * pPubDev)
{
	return HandleOnSetTime(pPubDev);
}
BOOL YkSelect(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return HandleOnYk(pPubDev,k_Select,wOutPort,bOnOff);
}
BOOL YkExecute(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return HandleOnYk(pPubDev,k_Execute,wOutPort,bOnOff);
}
BOOL YkCancel(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return HandleOnYk(pPubDev,k_Cancel,wOutPort,bOnOff);
}
BOOL InitCommunication(struct DEV_CLASS *pPubDev)
{
	pPubDev->RegisterPrePolling(pPubDev,PrePolling);
	pPubDev->RegisterDefaultRecvCallback(pPubDev,DefaultRecv);
	pPubDev->RegisterResetDevice(pPubDev,ResetDevice);
	pPubDev->RegisterSetDeviceTime(pPubDev,SetTime);
	pPubDev->RegisterYkOperations(pPubDev,YkSelect,YkExecute,YkCancel);
	//LoadDevScript(pPubDev,"test.lua");
	//HandleOnInit(pPubDev);
	return TRUE;
}
