#include <pthread.h>
#include "dev.h"
#include "luascript.h"

typedef struct LUA_CHANNEL {
	int nRef;
	lua_State *pLua;
	pthread_mutex_t mutex;
}LUA_CHANNEL;

BOOL InitScriptBaseclass(lua_State *pLua);
LUA_CHANNEL* GetLuaChannel(int nChannelNo);
LUA_CHANNEL* NewLuaChannel(int nChannelNo);
void FreeLuaChannel(int nChannelNo);
int LoadDevScript(DEV_CLASS *pPubDev, char *szScriptName);
int CallInterface(DEV_CLASS *pPubDev,const char *szInterfce, int nArg, int nResult);
int HandleOnInit(DEV_CLASS *pPubDev);
int HandleOnSend(DEV_CLASS *pPubDev, const LUA_SEND_CALLBACK pSendCallback);
int HandleOnRecv(DEV_CLASS *pPubDev, const LUA_RECV_CALLBACK pRecvCallback, BYTE *pBuffer, int nSize);
int HandleOnYk(DEV_CLASS *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff);
int HandleOnReset(DEV_CLASS *pPubDev);
int HandleOnSetTime(DEV_CLASS *pPubDev);

