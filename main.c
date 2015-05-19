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
lua_State *g_pLua=NULL;
BOOL PrePolling(struct _PubDev *pPubDev)
{
	return HandleOnSend(g_pLua,pPubDev);
}
BOOL DefaultRecv(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)
{
	return HandleOnRecv(g_pLua,pPubDev,pRecv,nSize);
}
BOOL ResetDevice(struct _PubDev * pPubDev)
{
	return HandleOnReset(g_pLua,pPubDev);
}
BOOL SetTime(struct _PubDev * pPubDev)
{
	return HandleOnSetTime(g_pLua,pPubDev);
}
BOOL YkSelect(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return HandleOnYk(g_pLua,pPubDev,k_Select,wOutPort,bOnOff);
}
BOOL YkExecute(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return HandleOnYk(g_pLua,pPubDev,k_Execute,wOutPort,bOnOff);
}
BOOL InitCommunication(struct _PubDev *pPubDev)
{
	pPubDev->SetFrameModule(pPubDev,FRAMEMODULE_MODBUS);//使用MODBUS规约模块
	pPubDev->RegisterPrePolling(pPubDev,PrePolling);
	pPubDev->RegisterDefaultRecvCallback(pPubDev,DefaultRecv);
	pPubDev->RegisterResetDevice(pPubDev,ResetDevice);
	pPubDev->RegisterYkOperations(pPubDev,YkSelect,YkExecute,NULL);
	if (NULL==g_pLua)
		g_pLua=InitLua();
	LoadDevScript(g_pLua,pPubDev,"test.lua");
	HandleOnInit(g_pLua,pPubDev);
	return TRUE;
}
