#include <string.h>
#include "dev.h"
#include "luascript.h"
#include "scriptif.h"

int g_nScriptClassInfo=0;
SCRIPT_CLASS_INFO g_arrScriptClassInfo[MAX_SCRIPT_CLASS_INFO];
SCRIPT_CLASS_INFO* GetScriptClassInfo(const char szClassName[])
{
	int i;
	for (i=0;i<g_nScriptClassInfo;i++) {
		if (strcmp(szClassName,g_arrScriptClassInfo[i].szClassName)==0)
			return &g_arrScriptClassInfo[i];
	}
	return NULL;
}

SCRIPT_CLASS_INFO* AddScriptClassInfo(const struct SCRIPT_CLASS_INFO* ScriptClassInfo)
{
	if (g_nScriptClassInfo<MAX_SCRIPT_CLASS_INFO) {
		int index=g_nScriptClassInfo++;
		g_arrScriptClassInfo[index]=*ScriptClassInfo;
		return &g_arrScriptClassInfo[index];
	} else {
		printf("\n AddScriptClassInfo error : ScriptClassInfo num > SCRIPT_CLASS_INFO_MAX");
	}
	return NULL;
}

void ScriptName2ClassName(const char *szScriptName,char szClassName[128])
{
	if (!szScriptName || !szClassName)
		return;
	sprintf(szClassName,"scriptclass_%s",szScriptName);
	char *p=strchr(szClassName,'.');
	*p='\0';
}
//////////////////////////////////////////////////////////////////////////
/*
��devscriptbase.lua�ű��� functiong new_buffer ������table ת��Ϊ�ֽ�����
index����ָ��table�ڴ�ջ��λ��
*/
int BufferToBytes(lua_State *pLua,int index,BYTE *pBuffer,int nSize)
{
	if (!pBuffer || !nSize) 
		return 0;
	if (!pLua || !lua_istable(pLua,index))
		return -1;

	lua_getfield(pLua,index,"arr");
	if (!lua_istable(pLua,-1)) {
		lua_pop(pLua,1); //pop arr
		return 0;
	}

	lua_getfield(pLua,index,"len");
	int len=lua_tointeger(pLua,-1);
	lua_getfield(pLua,index,"offset");
	int offset=lua_tointeger(pLua,-1);
	lua_pop(pLua,2); //pop len,offset

	int nBytes=len-offset;
	int i=0;
	if (offset==0) //push key
		lua_pushnil(pLua);
	else
		lua_pushinteger(pLua,offset);

	do {
		lua_next(pLua,-2); //next arr
		pBuffer[i++]=lua_tointeger(pLua,-1)&0xff;
		lua_pop(pLua,1); //pop value
	}while ( i<nBytes && i<nSize);

	lua_pop(pLua,2); //pop key,arr
	return i;
}

/*
���ֽ�����ת��Ϊdevscriptbase.lua�ű��� functiong new_buffer ������table
����ֵ:�����ɹ����tableѹ��luaջ�в�����nSize���������κβ���������-1
*/
int BytesToBuffer(lua_State *pLua,BYTE *pBuffer,int nSize)
{
	if (!pLua)
		return -1;
	if (!pBuffer || nSize<=0) 
		return 0;
	int iRet=1;
	lua_createtable(pLua,0,0); //push new table(buffer)
	if (luaL_getmetatable(pLua,STR_BUFFER_BASECLASS)) {
		lua_setmetatable(pLua,-2);
		
		lua_createtable(pLua,nSize,0); //push new table(arr)
		int i;
		for (i=0;i<nSize;i++) {
			//printf("\n arr[%d]=%02x",i+1,pBuffer[i]);
			//lua_pushinteger(pLua,i+1); //push key
			lua_pushinteger(pLua,pBuffer[i]); //push value
			//lua_settable(pLua,-3);
			lua_rawseti(pLua,-2,i+1);
		}
		lua_setfield(pLua,-2,"arr"); //pop table(arr)
		
		lua_pushinteger(pLua,nSize); //push len
		lua_setfield(pLua,-2,"len"); //pop len
		
		lua_pushinteger(pLua,0); //push offset
		lua_setfield(pLua,-2,"offset"); //pop offset

		/*DEBUG
		lua_getfield(pLua,-1,"PrintBuffer");
		lua_pushvalue(pLua,-2);
		lua_pcall(pLua,1,0,0);*/
	}
	else {
		iRet=-1;
		lua_pop(pLua,2); //pop new table(arr) & buffer_baseclass
	}
	return iRet;
}

lua_State* InitLua()
{
    lua_State *pLua = luaL_newstate();  /* opens Lua */
    luaL_openlibs(pLua);   /* opens the standard libraries */
    luaopen_math(pLua);
	
	if (luaL_dofile(pLua,"devices/devscriptbase.lua")!=LUA_OK)
	{
		printf("���� devices/devscriptbase.lua ʧ��!\n");
		return NULL;
	}
	
	if (!InitScriptBaseclass(pLua)) {
		lua_close(pLua);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	luaL_newmetatable(pLua,STR_BUFFER_BASECLASS);
	lua_getglobal(pLua,STR_BUFFER_BASECLASS);
	lua_setfield(pLua, -2, "__index");
	lua_pop(pLua,1); //pop STR_BUFFER_BASECLASS
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//ע��table ScriptClass
	lua_newtable(pLua);
	lua_setfield(pLua, LUA_REGISTRYINDEX, STR_SCRIPT_CLASS);
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	//ע��table DevsInstance 
	lua_newtable(pLua);
	lua_setfield(pLua, LUA_REGISTRYINDEX, STR_DEVS_INSTANCE);
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	//ע��table �ص���������
	lua_newtable(pLua);
	lua_setfield(pLua, LUA_REGISTRYINDEX, STR_CALLBACK);
	//////////////////////////////////////////////////////////////////////////
	return pLua;
}

void FreeLua(lua_State *pLua)
{
	lua_close(pLua);
}

/*
����:�����ű��ಢ���ؽű�����Ϣ
����:pLua:LUA״̬����szScriptClassName:
����ֵ:��������ɹ����Ѿ��ű�������򷵻ؽű�����Ϣָ�룬���򷵻�NULL
*/
SCRIPT_CLASS_INFO* CreateScriptClass(lua_State *pLua, char *szScriptClassName)
{
	if (!pLua || !szScriptClassName)
		return NULL;
	SCRIPT_CLASS_INFO* pScriptClassInfo=GetScriptClassInfo(szScriptClassName);
	if (pScriptClassInfo) //�Ѿ����ڣ�����Ҫ���¼���
		return pScriptClassInfo;

	/*
	lua_getfield(pLua, LUA_REGISTRYINDEX, STR_SCRIPT_CLASS);
	lua_getfield(pLua, -1, szScriptClassName);

	BOOL bHasExist = !(lua_isnil(pLua, -1));
	lua_pop(pLua, 1); //pop szScriptClassName
	if (bHasExist) //�ű��������Ѵ���
		return 1;
	*/

	//TOP = STR_SCRIPT_CLASS
	//����ӿڲ�����Ԫ��ΪScriptBaseClass
	if (lua_getglobal(pLua,STR_DEV_SCRIPT)==LUA_TNIL) {
		lua_pop(pLua, 1); //pop DevScript
		return 0;
	}
	lua_setfield(pLua, -2, szScriptClassName);
	lua_getfield(pLua, -1, szScriptClassName);
	luaL_getmetatable(pLua,STR_SCRIPTE_BASECLASS);
	lua_setmetatable(pLua,-2);

	//TOP = szScriptClassName
	//ע��Ԫ��
	if (luaL_newmetatable(pLua, szScriptClassName)) {
		lua_pushvalue(pLua,-2);
		lua_setfield(pLua, -2, "__index");
		lua_pop(pLua, 1); //pop szMetatableName

		SCRIPT_CLASS_INFO ScriptClassInfo;
		memset(&ScriptClassInfo,0,sizeof(&ScriptClassInfo));
		strcpy(ScriptClassInfo.szClassName,szScriptClassName);
		pScriptClassInfo=AddScriptClassInfo(&ScriptClassInfo);
	}
	

	lua_pop(pLua, 2); //pop szScriptClassName , STR_SCRIPT_CLASS
	return pScriptClassInfo;
}

