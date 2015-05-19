#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "dev.h"

#ifndef H_LUASCRIPT
#define H_LUASCRIPT
#define STR_DEV_SCRIPT "DevScript"
#define STR_DEV_UDATA "dev_udata"
#define STR_SCRIPTE_BASECLASS "script_baseclass"
#define STR_SCRIPT_CLASS "script_class"
#define STR_DEVS_INSTANCE "devs_instance"
#define STR_BUFFER_BASECLASS "buffer_baseclass"
#define STR_CALLBACK "callback_array"
#define STR_STATIC_CALLBACK "static_callback_array"
#define MAX_SCRIPT_CLASS_INFO 128

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


typedef struct SCRIPT_CLASS_INFO {
	char szClassName[128];
}SCRIPT_CLASS_INFO;

typedef struct DEVICE_CALSS {
	struct _PubDev *pPubDev;
	struct SCRIPT_CLASS_INFO *pScriptClassInfo;
	BOOL bUseRecvProc;
	char szRecvProc[128];
	enum yk_Kind YkOperation;
}DEVICE_CALSS;

lua_State* InitLua();
SCRIPT_CLASS_INFO* CreateScriptClass(lua_State *pLua, char *szScriptClassName);
int LoadDevScript(lua_State *pLua,struct _PubDev *pPubDev, char *szScriptName);
int CallInterface(lua_State *pLua,struct _PubDev *pPubDev,const char *szInterfce, int nArg, int nResult);
int BufferToBytes(lua_State *pLua,int index,BYTE *pBuffer,int nSize);
int BytesToBuffer(lua_State *pLua,BYTE *pBuffer,int nSize);

#endif
