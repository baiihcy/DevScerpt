#include "luascript.h"
#include "dev.h"

BOOL InitScriptBaseclass(lua_State *pLua);
int HandleOnInit(lua_State *pLua, struct DEV_CLASS *pPubDev);
int HandleOnSend(lua_State *pLua, struct DEV_CLASS *pPubDev, const LUA_SEND_CALLBACK pSendCallback);
int HandleOnRecv(lua_State *pLua,struct DEV_CLASS *pPubDev, const LUA_RECV_CALLBACK pRecvCallback, BYTE *pBuffer, int nSize);
int HandleOnYk(lua_State *pLua, struct DEV_CLASS *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff);
int HandleOnReset(lua_State *pLua, struct DEV_CLASS *pPubDev);
int HandleOnSetTime(lua_State *pLua, struct DEV_CLASS *pPubDev);

