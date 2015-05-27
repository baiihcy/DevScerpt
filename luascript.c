#include <string.h>
#include "dev.h"
#include "luascript.h"
#include "scriptif.h"

/*
˵����������λ�õ�Ԫ���__index��ѹջ
������idxջ����λ��
����ֵ�����������λ��������Ԫ����1
*/
int luaGetBaseclass(lua_State *pLua,int idx)
{
	if (!pLua) return 0;
	if (lua_getmetatable(pLua,idx)==0)
		return 0;
	lua_getfield(pLua,-1,"__index");
	lua_remove(pLua,-2); //pop metatable
	return 1;
}

int GetScriptClass(lua_State *pLua,const char szClassName[])
{
	if (!pLua) return 0;
	
	BOOL bScriptClassValid=FALSE;
	if (lua_getfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_METATABLE_SET)==LUA_TTABLE) {
		if (lua_getfield(pLua,-1,szClassName)==LUA_TTABLE) {
			if (lua_getfield(pLua,-1,"__index")==LUA_TTABLE) {
				//TOP ScriptClass
				if (luaGetBaseclass(pLua,-1)>0) {
					luaL_getmetatable(pLua,STR_SCRIPT_BASECLASS);
					if (lua_compare(pLua,-2,-1,LUA_OPEQ)) {
						bScriptClassValid=TRUE;
					}
					lua_pop(pLua,2); //pop ScriptBaseclass,ScriptBaseclass
				}
				
				//TOP ScriptClass
				//�ű�����Ч
				if (bScriptClassValid) {
					lua_pushvalue(pLua,-1);
					lua_setfield(pLua,LUA_REGISTRYINDEX,"TmpScriptClass");
				}
			}
			lua_pop(pLua,1); //pop ScriptClass
		}
		lua_pop(pLua,1); //pop ScriptClass_metatable
	}
	lua_pop(pLua,1); //pop STR_SCRIPT_CLASS_METATABLE_SET

	if (bScriptClassValid) {
		lua_getfield(pLua,LUA_REGISTRYINDEX,"TmpScriptClass");

		lua_pushnil(pLua);
		lua_setfield(pLua,LUA_REGISTRYINDEX,"TmpScriptClass");
		return 1;
	}
	
	return 0;
}

int GetScriptClassMetatable(lua_State *pLua,const char szClassName[])
{
	if (!pLua) return 0;
	if (lua_getfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_METATABLE_SET)!=LUA_TTABLE) {
		lua_pop(pLua,1); //pop STR_SCRIPT_CLASS_METATABLE_SET
		return 0;
	}
	
	if (lua_getfield(pLua,-1,szClassName)!=LUA_TTABLE) {
		lua_pop(pLua,2); //pop STR_SCRIPT_CLASS_METATABLE_SET,ScriptClass_metatable
		return 0;
	}

	lua_remove(pLua,-2);
	return 1;
}

BOOL HaveScriptClass(lua_State *pLua,const char szClassName[])
{
	if (!pLua) return FALSE;
	if (GetScriptClass(pLua,szClassName)>0) {
		lua_pop(pLua,1); //pop ScriptClass
		return TRUE;
	}
	return FALSE;
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
		return 0;

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
		return 0;
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
		iRet=0;
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
	//ע��table ScriptClassSet
	lua_newtable(pLua);
	lua_setfield(pLua, LUA_REGISTRYINDEX, STR_SCRIPT_CLASS_METATABLE_SET);
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	//ע��table ScriptClass
	lua_newtable(pLua);
	lua_setfield(pLua, LUA_REGISTRYINDEX, STR_SCRIPT_CLASS_SET);
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
����:��ջ��TABLE��ɽű��࣬������ջ
����:pLua:LUA״̬����szScriptClassName:ָ���ű�������
����ֵ:��������ɹ���ű����Ѿ����ڷ���1������������
*/
int CreateScriptClass(lua_State *pLua, char *szScriptClassName)
{
	if (!pLua || !szScriptClassName)
		return 0;
	if (!lua_istable(pLua,-1))
		return 0;
	//TOP STR_DEV_SCRIPT
	//����Ԫ��ΪScriptBaseClass
	if (!luaL_getmetatable(pLua,STR_SCRIPT_BASECLASS)) {
		lua_pop(pLua, 1); //pop STR_SCRIPT_BASECLASS
		return 0;
	}
	lua_setmetatable(pLua,-2);


	//TOP STR_DEV_SCRIPT
	//����ű���
	if (lua_getfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_SET)!=LUA_TTABLE) {
		lua_newtable(pLua);

		lua_pushvalue(pLua,-1);
		lua_setfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_SET);
	}
	lua_pushvalue(pLua,-2); //push STR_DEV_SCRIPT
	lua_setfield(pLua, -2, szScriptClassName);
	lua_pop(pLua,1); //pop STR_SCRIPT_CLASS_SET


	//TOP = ScriptClass
	//ע��Ԫ��
	if (luaL_newmetatable(pLua, szScriptClassName)) {
		lua_pushvalue(pLua,-2); //push STR_DEV_SCRIPT
		lua_setfield(pLua, -2, "__index");
		//TOP ScriptClass_metatable
		if (lua_getfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_METATABLE_SET)!=LUA_TTABLE) {
			lua_newtable(pLua);
			
			lua_pushvalue(pLua,-1);
			lua_setfield(pLua,LUA_REGISTRYINDEX,STR_SCRIPT_CLASS_METATABLE_SET);
		}
		lua_pushvalue(pLua,-2); //push ScriptClass_metatable
		lua_setfield(pLua,-2,szScriptClassName);
		lua_pop(pLua,2); //pop STR_SCRIPT_CLASS_METATABLE_SET,ScriptClass_metatable
		
		//Ԫ��ŵ� STR_SCRIPT_CLASS_METATABLE_SET ��֮���Ƴ���LUA_REGISTERYINDEX�е�Ԫ����ֹ������ͻ
		lua_pushnil(pLua);
		lua_setfield(pLua,LUA_REGISTRYINDEX,szScriptClassName);
	}
	
	return 1;
}

