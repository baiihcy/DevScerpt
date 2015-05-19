#include "dev.h"
#include "main.h"
#include "luascript.h"
#include "scriptif.h"

// BOOL PREPOLLING_CALLBACK(struct _PubDev *pPubDev)//Polling�ص�����
// BOOL RECEIVE_CALLBACK(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)//���Ľ��ջص�����
// BOOL INTERVALSEND_CALLBACK(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pSendNode)//������ͻص�����
// BOOL YKOPERATION_CALLBACK(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)//ң�ز����ص����� bOnOff TRUEΪ��բ��FALSEΪ��բ
// BOOL SETDEVICETIME_CALLBACK(struct _PubDev * pPubDev)//װ�ö�ʱ�ص�����
// BOOL RESETDEVICE_CALLBACK(struct _PubDev * pPubDev)//װ�ø���ص�����
// g_DP ���ݴ������ֱ��ʹ�ü���
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
	pPubDev->SetFrameModule(pPubDev,FRAMEMODULE_MODBUS);//ʹ��MODBUS��Լģ��
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
