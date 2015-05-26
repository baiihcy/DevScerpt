#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

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

#define LUA_SETINTEGER(_lua,_idx,_field,_num) \
{\
	lua_pushinteger((_lua),(_num));\
	lua_setfield((_lua),(_idx),(_field));\
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


typedef struct SCRIPT_CLASS_INFO {
	char szClassName[128];
}SCRIPT_CLASS_INFO;

lua_State* InitLua();
void FreeLua(lua_State* pLua);
SCRIPT_CLASS_INFO* GetScriptClassInfo(const char szClassName[]);
SCRIPT_CLASS_INFO* AddScriptClassInfo(const struct SCRIPT_CLASS_INFO* ScriptClassInfo);
void ScriptName2ClassName(const char *szScriptName,char szClassName[128]);
SCRIPT_CLASS_INFO* CreateScriptClass(lua_State *pLua, char *szScriptClassName);
int BufferToBytes(lua_State *pLua,int index,BYTE *pBuffer,int nSize);
int BytesToBuffer(lua_State *pLua,BYTE *pBuffer,int nSize);

#endif
