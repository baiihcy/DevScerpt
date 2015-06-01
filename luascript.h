#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <pthread.h>
#include "dev.h"

#ifndef H_LUASCRIPT
#define H_LUASCRIPT

#define STR_DEV_SCRIPT "DevScript"
#define STR_DEV_UDATA "dev_udata"
#define STR_SCRIPT_BASECLASS "script_baseclass"
#define STR_SCRIPT_CLASS_METATABLE_SET "script_class_metatable_set"
#define STR_SCRIPT_CLASS_SET "script_class_set"
#define STR_DEVS_INSTANCE "devs_instance"
#define STR_BUFFER_BASECLASS "buffer_baseclass"
#define STR_CALLBACK "callback_array"
#define STR_STATIC_CALLBACK "static_callback_array"
#define MAX_SCRIPT_CLASS_INFO 128
#define LUA_SETINTEGER(_lua,_idx,_field,_num) \
{\
	lua_pushinteger((_lua),(_num));\
	lua_setfield((_lua),(_idx)-1,(_field));\
}
#define LUA_GETINTEGER(_lua,_idx,_field,_num,_pisnum) \
{\
	lua_getfield((_lua),(_idx),(_field));\
	_num=lua_tointegerx((_lua),-1,(_pisnum));\
	lua_pop(_lua,1);\
}
#define LUA_GETNUMBER(_lua,_idx,_field,_num,_pisnum) \
{\
	lua_getfield((_lua),(_idx),(_field));\
	_num=lua_tonumberx((_lua),-1,(_pisnum));\
	lua_pop(_lua,1);\
}

typedef struct LUA_CHANNEL {
	int nRef;
	lua_State *pLua;
	pthread_mutex_t mutex;
}LUA_CHANNEL;

int luaGetBaseclass(lua_State *pLua,int idx);
void luaScriptName2ClassName(const char *szScriptName,char szClassName[128]);
int luaBufferToBytes(lua_State *pLua,int index,BYTE *pBuffer,int nSize);
int luaBytesToBuffer(lua_State *pLua,BYTE *pBuffer,int nSize);
int luaGetBaseclass(lua_State *pLua,int idx);
int luaGetScriptClassMetatable(lua_State *pLua,const char szClassName[]);
int luaGetScriptClass(lua_State *pLua,const char szClassName[]);
BOOL luaHaveScriptClass(lua_State *pLua,const char szClassName[]);
int luaCreateScriptClass(lua_State *pLua, char *szScriptClassName);

LUA_CHANNEL* GetLuaChannel(int nChannelNo);
LUA_CHANNEL* NewLuaChannel(int nChannelNo);
void FreeLuaChannel(int nChannelNo);
void LuaChannelLock(LUA_CHANNEL *pLuaChannel);
void LuaChannelUnlock(LUA_CHANNEL *pLuaChannel);

lua_State* InitLua();
void FreeLua(lua_State* pLua);
int luaLoadDevScript(DEV_CLASS *pPubDev, char *szScriptName);
int luaCallInterface(DEV_CLASS *pPubDev,const char *szInterfce, int nArg, int nResult);
int luaHandleOnInit(DEV_CLASS *pPubDev);
int luaHandleOnSend(DEV_CLASS *pPubDev, const LUA_SEND_CALLBACK pSendCallback);
int luaHandleOnRecv(DEV_CLASS *pPubDev, const LUA_RECV_CALLBACK pRecvCallback, BYTE *pBuffer, int nSize);
int luaHandleOnYk(DEV_CLASS *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff);
int luaHandleOnReset(DEV_CLASS *pPubDev);
int luaHandleOnSetTime(DEV_CLASS *pPubDev);

#endif //#ifndef H_LUASCRIPT
