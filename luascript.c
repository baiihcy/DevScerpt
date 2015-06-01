#include <string.h>
#include "dev.h"
#include "luascript.h"
#include "scriptif.h"

LUA_CHANNEL g_LuaChannels[MAX_CHANNEL]={};
//////////////////////////////////////////////////////////////////////////
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

/*
˵�����ӽű���Ԫ������Ѱ��ָ��Ԫ��ѹջ
������szClassName:�ű�������
����ֵ���ҵ�ָ��Ԫ����1����Ԫ��ѹջ�����򷵻�0������
*/
int luaGetScriptClassMetatable(lua_State *pLua,const char szClassName[])
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
	
	lua_remove(pLua,-2); //POP STR_SCRIPT_CLASS_METATABLE_SET
	return 1;
}

/*
˵�����ӽű��༯����Ѱ��ָ���ű��ಢѹջ
������szClassName:�ű�������
����ֵ���ҵ�ָ���ű��෵��1��ѹջ�����򷵻�0������
*/
int luaGetScriptClass(lua_State *pLua,const char szClassName[])
{
	if (!pLua) return 0;
	
	lua_pushnil(pLua); //���һ��λ�ñ�����
	BOOL bScriptClassValid=FALSE;
	if (!luaGetScriptClassMetatable(pLua,szClassName)) {
		lua_pop(pLua,1); //pop nil
		return 0;
	}

	//TOP ScriptClass_metatable
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
			lua_copy(pLua,-1,-3);
		}
	}
	lua_pop(pLua,1); //pop ScriptClass
	lua_pop(pLua,1); //pop ScriptClass_metatable

	if (bScriptClassValid) {
		return 1;
	} else {
		lua_pop(pLua,1); //pop nil (ԭ����Ϊ�˷Ž��������λ��)
		return 0;
	}
}

BOOL luaHaveScriptClass(lua_State *pLua,const char szClassName[])
{
	if (!pLua) return FALSE;
	if (luaGetScriptClass(pLua,szClassName)>0) {
		lua_pop(pLua,1); //pop ScriptClass
		return TRUE;
	}
	return FALSE;
}

void luaScriptName2ClassName(const char *szScriptName,char szClassName[128])
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
int luaBufferToBytes(lua_State *pLua,int index,BYTE *pBuffer,int nSize)
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
int luaBytesToBuffer(lua_State *pLua,BYTE *pBuffer,int nSize)
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

//////////////////////////////////////////////////////////////////////////
//LUA CHANNEL
LUA_CHANNEL* GetLuaChannel(int nChannelNo)
{
	if (nChannelNo<0 || nChannelNo>=MAX_CHANNEL) 
		return NULL;
	LUA_CHANNEL *pLuaChannel=&g_LuaChannels[nChannelNo];
	if (pLuaChannel->nRef<=0 || NULL==pLuaChannel->pLua) {
		printf("\n GetLuaChannel error : ref=%d,pLua=%x",pLuaChannel->nRef,(unsigned int)pLuaChannel->pLua);
		return NULL;
	}
	return pLuaChannel;
}
LUA_CHANNEL* NewLuaChannel(int nChannelNo)
{
	if (nChannelNo<0 || nChannelNo>=MAX_CHANNEL) 
		return NULL;
	LUA_CHANNEL *pLuaChannel=&g_LuaChannels[nChannelNo];
	if (0==pLuaChannel->nRef) {
		pLuaChannel->pLua=InitLua();
		if (!pLuaChannel->pLua) {
			printf("\n NewLuaChannel error : Failed to InitLua");
			return NULL;
		}
		//pLuaChannel->mutex=PTHREAD_MUTEX_INITIALIZER;
		if (pthread_mutex_init(&pLuaChannel->mutex,NULL)!=0) {
			printf("\n pthread_mutex_init error");
		}
		pLuaChannel->nRef++;
		return pLuaChannel;
	} else if (pLuaChannel->nRef>0) {
		pLuaChannel->nRef++;
		return pLuaChannel;
	} else {
		pLuaChannel->nRef=0;
		return NULL;
	}
}
void LuaChannelLock(LUA_CHANNEL *pLuaChannel)
{
	if (!pLuaChannel) return;
	pthread_mutex_lock(&pLuaChannel->mutex);
}
void LuaChannelUnlock(LUA_CHANNEL *pLuaChannel)
{
	if (!pLuaChannel) return;
	pthread_mutex_unlock(&pLuaChannel->mutex);
}
void FreeLuaChannel(int nChannelNo) 
{
	if (nChannelNo<0 || nChannelNo>=MAX_CHANNEL) 
		return ;
	LUA_CHANNEL *pLuaChannel=&g_LuaChannels[nChannelNo];
	if (pLuaChannel->nRef>0) {
		pLuaChannel->nRef--;
		if (0==pLuaChannel->nRef) {
			FreeLua(pLuaChannel->pLua);
			pthread_mutex_destroy(&pLuaChannel->mutex);
			pLuaChannel->pLua=NULL;
		} 
	}
}
//LUA CHANNEL
//////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
int luaLoadDevScript(DEV_CLASS *pPubDev, char *szScriptName)
{
	if (!pPubDev || !szScriptName)  return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	char szClassName[128];
	luaScriptName2ClassName(szScriptName,szClassName);
	
	BOOL bHaveScriptClass=luaHaveScriptClass(pLua,szClassName);
	if (!bHaveScriptClass) {
		//δ������
		char szFileName[255];
		sprintf(szFileName,"devices/%s",szScriptName);
		if (luaL_dofile(pLua,szFileName)!=LUA_OK) {
			printf("\n LoadDevScript error : \n%s\n",lua_tostring(pLua,-1));
			lua_pop(pLua,1); //pop error message
			return 0;
		}
		if (lua_getglobal(pLua,STR_DEV_SCRIPT)!=LUA_TTABLE) {
			printf("\n Invalid ScriptDev , type %s",lua_typename(pLua,lua_type(pLua,-1)));
			lua_pop(pLua, 1); //pop DevScript
			return 0;
		}
		
		//TOP DevScript
		if (luaCreateScriptClass(pLua,szClassName)>0) {
			bHaveScriptClass=TRUE;
			lua_pop(pLua,1); //pop ScriptClass
		}
		//Reset DevScript
		lua_pushnil(pLua);
		lua_setglobal(pLua, STR_DEV_SCRIPT);
	}
	if (bHaveScriptClass) {
		int DevInstanceKey = pPubDev->Get_DeviceNo(pPubDev);
		lua_newtable(pLua); //push DevInstance
		//����Ԫ��
		if (luaGetScriptClassMetatable(pLua,szClassName)) 
			lua_setmetatable(pLua,-2);
		else 
			printf("\n LoadDevScript error : Invalid ScriptClassMetatable (%s)",szClassName);
		
		//�����û�����
		lua_pushlightuserdata(pLua,(void*)pPubDev);
		lua_setfield(pLua,-2,STR_DEV_UDATA);

		if (lua_getfield(pLua, LUA_REGISTRYINDEX, STR_DEVS_INSTANCE)!=LUA_TTABLE) {
			lua_newtable(pLua);
			lua_pushvalue(pLua,-1);
			lua_setfield(pLua,LUA_REGISTRYINDEX,STR_DEVS_INSTANCE);
		}
		lua_pushinteger(pLua, DevInstanceKey);
		lua_pushvalue(pLua, -3); //push DevInstance
		lua_settable(pLua, -3);
		lua_pop(pLua, 1); //pop STR_DEVS_INSTANCE
		
		//////////////////////////////////////////////////////////////////////////
		//Set FrameModel
		lua_getfield(pLua,-1,"frame_model");
		const char *pFrameModel=lua_tostring(pLua,-1);
		if (pFrameModel) pPubDev->SetFrameModule(pPubDev,pFrameModel);
		lua_pop(pLua,1); //pop frame_model
		//////////////////////////////////////////////////////////////////////////
		
		lua_pop(pLua, 1); //pop DevInstance
		return 1;
	}
	return 0;
}

/*
����:��ջ��TABLE��ɽű��࣬������ջ
����:pLua:LUA״̬����szScriptClassName:ָ���ű�������
����ֵ:��������ɹ���ű����Ѿ����ڷ���1������������
*/
int luaCreateScriptClass(lua_State *pLua, char *szScriptClassName)
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


/*
˵��:����װ�ýӿڣ�����ǰ��Ҫ�ȰѲ���ѹջ�����óɹ�������ѹջ
����ֵ:������óɹ��򷵻�1;����Ҳ����ӿ��򷵻�0;�����򷵻�-1
*/
int luaCallInterface(DEV_CLASS *pPubDev, const char *szInterfce, int nArg, int nResult)
{
	if (!pPubDev || !szInterfce || nArg<0 || nResult<0) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	int DevInstanceKey = pPubDev->Get_DeviceNo(pPubDev);
	int iRet=0;
	lua_getfield(pLua, LUA_REGISTRYINDEX, STR_DEVS_INSTANCE); //push STR_DEVS_INSTANCE
	lua_pushinteger(pLua, DevInstanceKey);
	lua_gettable(pLua, -2); //push DevInstance
	if (!lua_isnil(pLua, -1)) {
		lua_getfield(pLua,-1,szInterfce);
		if (lua_isfunction(pLua, -1)) {
			lua_pushvalue(pLua, -2); //push DevInstance
			if (nArg>0) {
				int iArg;
				for (iArg=0;iArg<nArg;iArg++)
					lua_pushvalue(pLua,-(4+nArg));
			}
			if (lua_pcall(pLua, 1+nArg, nResult, 0)==LUA_OK) {
				iRet=1;
			}else{
				printf("\n Call Interface[%s] error : \n%s\n",szInterfce,lua_tostring(pLua,-1));
				lua_pop(pLua,1); //pop error message
			}
		} else {
			TRACE("Invalid Interface [%s]",szInterfce);
			lua_pop(pLua, 1); //pop szInterfce
		}
	}
	if (!iRet) nResult=0;
	lua_remove(pLua, -(nResult+1)); //pop DevInstance
	lua_remove(pLua, -(nResult+1)); //pop STR_DEVS_INSTANCE
	return iRet;
}

/*
˵��:����OnInit�ӿ�
����ֵ:�Ҳ���OnInit�ӿ�ֱ�ӷ���TRUE�����򷵻�OnInit�ӿڵķ���ֵ
*/
int luaHandleOnInit(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=TRUE;

	LuaChannelLock(pLuaChannel);
	if (luaCallInterface(pPubDev,"OnInit",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
˵��:����OnSend�ӿ�
����ֵ:�Ҳ���OnSend�ӿ�ֱ�ӷ���FALSE�����򷵻�OnSend�ӿڵķ���ֵ
*/
int luaHandleOnSend(DEV_CLASS *pPubDev, const LUA_SEND_CALLBACK pSendCallback )
{
	if (!pPubDev) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	printf("\n ch%02d TOP=%d",pPubDev->Get_ChannelNo(pPubDev),lua_gettop(pLua));
	BOOL bRet=TRUE;
	LUA_SEND_CALLBACK pInterface;
	if (pSendCallback) strcpy(pInterface,pSendCallback);
	else strcpy(pInterface,"OnSend");

	LuaChannelLock(pLuaChannel);
	if (luaCallInterface(pPubDev,pInterface,0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
˵��:����OnRecv�ӿ�
����ֵ:�Ҳ���OnRecv�ӿ�ֱ�ӷ���TRUE�����򷵻�OnRecv�ӿڵķ���ֵ
*/
int luaHandleOnRecv(DEV_CLASS *pPubDev, const LUA_RECV_CALLBACK pRecvCallback, BYTE *pBuffer, int nSize )
{
	if (!pPubDev ||  !pBuffer || nSize<=0) 
		return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	LUA_RECV_CALLBACK pInterface;
	if (pRecvCallback) strcpy(pInterface,pRecvCallback);
	else strcpy(pInterface,"OnRecv");
	BOOL bRet=TRUE;

	LuaChannelLock(pLuaChannel);
	if (luaBytesToBuffer(pLua,pBuffer,nSize)>=0) {
		if (luaCallInterface(pPubDev,pInterface,1,1)>0) {
			bRet=lua_toboolean(pLua,-1);
			lua_pop(pLua, 1); //pop lua_pcall return value
		}

		lua_pop(pLua,1); //pop buffer
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
˵��:����OnYkSelect/OnYkExecute�ӿ�
����ֵ:�Ҳ����ӿ�ֱ�ӷ���FALSE�����򷵻ؽӿڵķ���ֵ
*/
int luaHandleOnYk(DEV_CLASS *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff)
{
	if (!pPubDev) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=FALSE;
	char szInterface[64];
	switch  (YkKind) {
	case k_Select:
		strcpy(szInterface,"OnYkSelect");
		break;
	case k_Execute:
		strcpy(szInterface,"OnYkExecute");
		break;
	case k_Cancel:
		strcpy(szInterface,"OnYkCancel");
		break;
	default:
		return 0;
		break;
	}
	pPubDev->m_LastYkKind=YkKind;
	
	LuaChannelLock(pLuaChannel);
	lua_pushinteger(pLua,byYkGroup);
	lua_pushinteger(pLua,bYkOnoff);
	if (luaCallInterface(pPubDev,szInterface,2,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	lua_pop(pLua,2); //pop CallInterface args
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
˵��:����OnReset�ӿ�
����ֵ:�Ҳ���OnReset�ӿ�ֱ�ӷ���FALSE�����򷵻�OnReset�ӿڵķ���ֵ
*/
int luaHandleOnReset(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=FALSE;
	
	LuaChannelLock(pLuaChannel);
	if (luaCallInterface(pPubDev,"OnReset",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
˵��:����OnSetTime�ӿ�
����ֵ:�Ҳ���OnSetTime�ӿ�ֱ�ӷ���FALSE�����򷵻�OnSetTime�ӿڵķ���ֵ
*/
int luaHandleOnSetTime(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return 0;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return 0;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=FALSE;
	
	LuaChannelLock(pLuaChannel);
	if (luaCallInterface(pPubDev,"OnSetTime",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}
