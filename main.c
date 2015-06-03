#include "dev.h"
#include "main.h"
#include "luascript.h"
#include <signal.h>   /* for signal */
#include <execinfo.h> /* for backtrace() */

BOOL g_bRegSignal=FALSE;
void DumpBacktrace()
{
	int i,nPtrs;
	void *buffer[100];
	char **strings;
	nPtrs=backtrace(buffer,100);
	printf("\n backtrace() returned %d addresses",nPtrs);

	strings=backtrace_symbols(buffer,nPtrs);
	if (strings==NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
	for (i=0;i<nPtrs;i++) {
		printf("\n [%02d] %s",i,strings[i]);
	}
	free(strings);
}

void SignalHander(int signo)
{
	printf("\n\n=========>>>catch signal %d (%s) <<<=========",
		signo, (char *)strsignal(signo));
	printf("\nDump stack start...");
	DumpBacktrace();
	printf("\nDump stack end...");
	/* �ָ��������ź� */
	signal(signo, SIG_DFL);
	raise(signo);

}

void PrintSigno(int signo)
{
    static int i = 0;
	printf("\n\n=========>>>catch signal %d (%s) i = %d <<<=========",
	signo, (char *)strsignal(signo), i++);
	printf("\nDump stack start...");
	DumpBacktrace();
	printf("\nDump stack end...");

}

void RegSignal()
{
	signal(SIGINT, SignalHander);
	signal(SIGSEGV, SignalHander);
	signal(SIGUSR1, SignalHander);
	printf("\nCurrent function calls list is: ");
	printf("\n----------------------------------");
	DumpBacktrace();
	printf("\n----------------------------------\n");

}

// BOOL PREPOLLING_CALLBACK(struct _PubDev *pPubDev)//Polling�ص�����
// BOOL RECEIVE_CALLBACK(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)//���Ľ��ջص�����
// BOOL INTERVALSEND_CALLBACK(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pSendNode)//������ͻص�����
// BOOL YKOPERATION_CALLBACK(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)//ң�ز����ص����� bOnOff TRUEΪ��բ��FALSEΪ��բ
// BOOL SETDEVICETIME_CALLBACK(struct _PubDev * pPubDev)//װ�ö�ʱ�ص�����
// BOOL RESETDEVICE_CALLBACK(struct _PubDev * pPubDev)//װ�ø���ص�����
// g_DP ���ݴ������ֱ��ʹ�ü���

BOOL PrePolling(struct DEV_CLASS *pPubDev)
{
	return luaHandleOnSend(pPubDev,NULL);
}
BOOL DefaultRecv(struct DEV_CLASS *pPubDev,BYTE *pRecv,int nSize)
{
	return luaHandleOnRecv(pPubDev,NULL,pRecv,nSize);
}
BOOL ResetDevice(struct DEV_CLASS * pPubDev)
{
	return luaHandleOnReset(pPubDev);
}
BOOL SetTime(struct DEV_CLASS * pPubDev)
{
	return luaHandleOnSetTime(pPubDev);
}
BOOL YkSelect(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return luaHandleOnYk(pPubDev,k_Select,wOutPort,bOnOff);
}
BOOL YkExecute(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return luaHandleOnYk(pPubDev,k_Execute,wOutPort,bOnOff);
}
BOOL YkCancel(struct DEV_CLASS *pPubDev,WORD wOutPort,BOOL bOnOff)
{
	return luaHandleOnYk(pPubDev,k_Cancel,wOutPort,bOnOff);
}
BOOL InitCommunication(struct DEV_CLASS *pPubDev)
{
	pPubDev->RegisterPrePolling(pPubDev,PrePolling);
	pPubDev->RegisterDefaultRecvCallback(pPubDev,DefaultRecv);
	pPubDev->RegisterResetDevice(pPubDev,ResetDevice);
	pPubDev->RegisterSetDeviceTime(pPubDev,SetTime);
	pPubDev->RegisterYkOperations(pPubDev,YkSelect,YkExecute,YkCancel);
	if (!g_bRegSignal) {
		g_bRegSignal=TRUE;
		RegSignal();
	}
	//LoadDevScript(pPubDev,"test.lua");
	//HandleOnInit(pPubDev);
	return TRUE;
}
