#include "luascript.h"
#include "dev.h"

BOOL InitScriptBaseclass(lua_State *pLua);
int HandleOnInit(lua_State *pLua, struct _PubDev *pPubDev);
int HandleOnSend(lua_State *pLua, struct _PubDev *pPubDev);
int HandleOnRecv(lua_State *pLua,struct _PubDev *pPubDev, BYTE *pBuffer, int nSize);
int HandleOnYk(lua_State *pLua, struct _PubDev *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff);
int HandleOnReset(lua_State *pLua, struct _PubDev *pPubDev);
int HandleOnSetTime(lua_State *pLua, struct _PubDev *pPubDev);

