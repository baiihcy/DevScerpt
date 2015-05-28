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
