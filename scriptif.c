#include "scriptif.h"

//////////////////////////////////////////////////////////////////////////
LUA_CHANNEL g_LuaChannels[CHANNEL_MAX]={};
LUA_CHANNEL* GetLuaChannel(int nChannelNo)
{
	if (nChannelNo<0 || nChannelNo>=CHANNEL_MAX) 
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
	if (nChannelNo<0 || nChannelNo>=CHANNEL_MAX) 
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
	if (nChannelNo<0 || nChannelNo>=CHANNEL_MAX) 
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
//////////////////////////////////////////////////////////////////////////
int LoadDevScript(DEV_CLASS *pPubDev, char *szScriptName)
{
	if (pPubDev==NULL || szScriptName==NULL)  return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	char szClassName[128];
	ScriptName2ClassName(szScriptName,szClassName);
	SCRIPT_CLASS_INFO *pScriptClassInfo=GetScriptClassInfo(szClassName);
	
	if (NULL==pScriptClassInfo) /*未曾加载*/ {
		char szFileName[255];
		sprintf(szFileName,"devices/%s",szScriptName);
		if (luaL_dofile(pLua,szFileName)!=LUA_OK) {
			printf("\n LoadDevScript error : \n%s\n",lua_tostring(pLua,-1));
			lua_pop(pLua,1); //pop error message
			return -1;
		}
		if (lua_getglobal(pLua,STR_DEV_SCRIPT)==LUA_TNIL) {
			lua_pop(pLua,1);
			return 0;
		}
		
		pScriptClassInfo=CreateScriptClass(pLua,szClassName);
		//Reset DevScript
		lua_pushnil(pLua);
		lua_setglobal(pLua, STR_DEV_SCRIPT);
	}
	
	if (pScriptClassInfo) {
		int DevInstanceKey = pPubDev->Get_DeviceNo(pPubDev);
		lua_newtable(pLua);
		lua_pushlightuserdata(pLua,(void*)pPubDev);
		lua_setfield(pLua,-2,STR_DEV_UDATA);
		//struct DEVICE_CALSS *pDevClass = lua_newuserdata(pLua, sizeof(struct DEVICE_CALSS));
		lua_getfield(pLua, LUA_REGISTRYINDEX, STR_DEVS_INSTANCE);
		lua_pushinteger(pLua, DevInstanceKey);
		lua_pushvalue(pLua, -3);
		lua_settable(pLua, -3);
		lua_pop(pLua, 1); //pop STR_DEVS_INSTANCE
		
		
		//TOP = pDevClass
		luaL_getmetatable(pLua, szClassName);
		lua_setmetatable(pLua, -2);
		pPubDev->pScriptClassInfo=(void*)pScriptClassInfo;

		//Set FrameModel
		lua_getfield(pLua,-1,"frame_model");
		const char *pFrameModel=lua_tostring(pLua,-1);
		if (pFrameModel) pPubDev->SetFrameModule(pPubDev,pFrameModel);
		lua_pop(pLua,1); //pop frame_model


		lua_pop(pLua, 1); //pop pDevClass
		return 1;
	}
	return 0;
}

/*
说明:调用装置接口，调用前需要先把参数压栈；调用成功后结果会压栈
返回值:如果调用成功则返回1;如果找不到接口则返回0;出错则返回-1
*/
int CallInterface(DEV_CLASS *pPubDev, const char *szInterfce, int nArg, int nResult)
{
	if (!pPubDev || !szInterfce || nArg<0 || nResult<0) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
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
			if (lua_pcall(pLua, 1+nArg, nResult, 0)!=LUA_OK) {
				printf("\n Call Interface[%s] error : \n%s\n",szInterfce,lua_tostring(pLua,-1));
				lua_pop(pLua,1); //pop error message
				iRet=-1;
			}
			iRet=1;
		}
		else lua_pop(pLua, 1); //pop szInterfce
	}
	if (!iRet) nResult=0;
	lua_remove(pLua, -(nResult+1)); //pop DevInstance
	lua_remove(pLua, -(nResult+1)); //pop STR_DEVS_INSTANCE
	return iRet;
}

/*
说明:触发OnInit接口
返回值:找不到OnInit接口直接返回TRUE，否则返回OnInit接口的返回值
*/
int HandleOnInit(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=TRUE;

	LuaChannelLock(pLuaChannel);
	if (CallInterface(pPubDev,"OnInit",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
说明:触发OnSend接口
返回值:找不到OnSend接口直接返回FALSE，否则返回OnSend接口的返回值
*/
int HandleOnSend(DEV_CLASS *pPubDev, const LUA_SEND_CALLBACK pSendCallback )
{
	if (!pPubDev) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	printf("\n TOP=%d",lua_gettop(pLua));
	BOOL bRet=TRUE;
	LUA_SEND_CALLBACK pInterface;
	if (pSendCallback) strcpy(pInterface,pSendCallback);
	else strcpy(pInterface,"OnSend");

	LuaChannelLock(pLuaChannel);
	if (CallInterface(pPubDev,pInterface,0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
说明:触发OnRecv接口
返回值:找不到OnRecv接口直接返回TRUE，否则返回OnRecv接口的返回值
*/
int HandleOnRecv(DEV_CLASS *pPubDev, const LUA_RECV_CALLBACK pRecvCallback, BYTE *pBuffer, int nSize )
{
	if (!pPubDev ||  !pBuffer || nSize<=0) 
		return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	LUA_RECV_CALLBACK pInterface;
	if (pRecvCallback) strcpy(pInterface,pRecvCallback);
	else strcpy(pInterface,"OnRecv");
	BOOL bRet=TRUE;

	LuaChannelLock(pLuaChannel);
	if (BytesToBuffer(pLua,pBuffer,nSize)>=0) {
		if (CallInterface(pPubDev,pInterface,1,1)>0) {
			bRet=lua_toboolean(pLua,-1);
			lua_pop(pLua, 1); //pop lua_pcall return value
		}

		lua_pop(pLua,1); //pop buffer
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
说明:触发OnYkSelect/OnYkExecute接口
返回值:找不到接口直接返回FALSE，否则返回接口的返回值
*/
int HandleOnYk(DEV_CLASS *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff)
{
	if (!pPubDev) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
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
		// 	case k_Cancel:
		// 		break;
	default:
		return 0;
		break;
	}
	pPubDev->m_LastYkKind=YkKind;
	
	LuaChannelLock(pLuaChannel);
	lua_pushinteger(pLua,byYkGroup);
	lua_pushinteger(pLua,bYkOnoff);
	if (CallInterface(pPubDev,szInterface,2,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	lua_pop(pLua,2); //pop CallInterface args
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
说明:触发OnReset接口
返回值:找不到OnReset接口直接返回FALSE，否则返回OnReset接口的返回值
*/
int HandleOnReset(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=FALSE;
	
	LuaChannelLock(pLuaChannel);
	if (CallInterface(pPubDev,"OnReset",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

/*
说明:触发OnSetTime接口
返回值:找不到OnSetTime接口直接返回FALSE，否则返回OnSetTime接口的返回值
*/
int HandleOnSetTime(DEV_CLASS *pPubDev)
{
	if (!pPubDev) return -1;
	LUA_CHANNEL *pLuaChannel=GetLuaChannel(pPubDev->Get_ChannelNo(pPubDev));
	if (!pLuaChannel) return -1;
	lua_State *pLua=pLuaChannel->pLua;
	BOOL bRet=FALSE;
	
	LuaChannelLock(pLuaChannel);
	if (CallInterface(pPubDev,"OnSetTime",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	LuaChannelUnlock(pLuaChannel);
	return bRet;
}

//////////////////////////////////////////////////////////////////////////

/*参数:frame_name;返回值:load_success*/
int luafunc_LoadFrame(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkstring(pLua,2);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		const char *pFrameName=lua_tostring(pLua,2);
		if (pFrameName) {
			pPubDev->SetFrameModule(pPubDev,pFrameName);
			lua_pushboolean(pLua,TRUE);
			return 1;
		}
	}
	return 0;
}
/*参数:buffer,recv_proc,interval_sec*/
int luafunc_PollSend(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		BYTE pBuffer[256];
		WORD wBufferSize=256;
		wBufferSize=BufferToBytes(pLua,2,pBuffer,wBufferSize);
		if (wBufferSize>0) {
			const char *pRecvProc=lua_tostring(pLua,3);
			DWORD dwInterval=lua_tointeger(pLua,4);
			if (pPubDev->RegisterIntervalSend(pPubDev,dwInterval,pBuffer,wBufferSize,pRecvProc)) {
				lua_pushboolean(pLua,TRUE);
				return 1;
			}
		}
	}
	return 0;
}
/*参数:send_proc,interval_sec*/
int luafunc_PollSendProc(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkstring(pLua,2);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		const char *pSendProc=lua_tostring(pLua,2);
		DWORD dwInterval=lua_tointeger(pLua,3);
		if (pPubDev->RegisterIntervalSend_Callback(pPubDev,dwInterval,pSendProc)) {
			lua_pushboolean(pLua,TRUE);
			return 1;
		}
	}
	return 0;
}
/*返回值:device_no*/
int luafunc_GetDeviceNo(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushinteger(pLua,pPubDev->Get_DeviceNo(pPubDev));
		return 1;
	}
	return 0;
}
/*返回值:channel_no*/
int luafunc_GetChannelNo(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushinteger(pLua,pPubDev->Get_ChannelNo(pPubDev));
		return 1;
	}
	return 0;
}
/*返回值:addr*/
int luafunc_GetLinkAddr(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushinteger(pLua,pPubDev->Get_LinkAddr(pPubDev));
		return 1;
	}
	return 0;
}
/*返回值:addr2*/
int luafunc_GetLinkAddr2(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushstring(pLua,pPubDev->Get_LinkAddr2(pPubDev));
		return 1;
	}
	return 0;
}
/*返回值:Param1*/
int luafunc_GetParam1(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushstring(pLua,pPubDev->Get_Param1(pPubDev));
		return 1;
	}
	return 0;
}
/*返回值:Param2*/
int luafunc_GetParam2(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		lua_pushstring(pLua,pPubDev->Get_Param2(pPubDev));
		return 1;
	}
	return 0;
}
/*参数:addr*/
int luafunc_SetLinkAddr(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		int bIsNum;
		int nAddr=lua_tointegerx(pLua,2,&bIsNum);
		if (bIsNum) {
			pPubDev->Set_LinkAddr(pPubDev,nAddr);
		}
	}
	return 0;
}
/*参数:yc_index;返回值:yx_value*/
int luafunc_GetYXOne(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		BOOL bYxValue;
		int nYxIndex=lua_tointeger(pLua,2);
		if (nYxIndex>0) {
			if (pPubDev->GetYxValue(pPubDev,nYxIndex-1,&bYxValue)) {
				lua_pushinteger(pLua,bYxValue);
				return 1;
			}
		}
	}
	return 0;
}
/*参数:yx_index;返回值:yc_value*/
int luafunc_GetYCOne(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	if (!lua_istable(pLua,1)) return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	if (pPubDev) {
		float fYcValue=0;
		int nYcIndex=lua_tointeger(pLua,2);
		if (nYcIndex>0) {
			if (pPubDev->GetYcValue(pPubDev,nYcIndex-1,&fYcValue)) {
				lua_pushnumber(pLua,fYcValue);
				return 1;
			}
		}
	}
	return 0;
}
/*参数:yx_index,yx_value*/
int luafunc_MailYXOne(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	luaL_checkinteger(pLua,3);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYxIndex=lua_tointeger(pLua,2);
			int nYxValue=lua_tointeger(pLua,3);
			if (nYxIndex>=0) {
				pPubDev->ExplainYx(pPubDev,nYxIndex,nYxValue);
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:yx_index,yx_byte*/
int luafunc_MailYXByte(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	luaL_checkinteger(pLua,3);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYxIndex=lua_tointeger(pLua,2);
			BYTE byYx=(BYTE)lua_tointeger(pLua,3);
			if (nYxIndex>=0) {
				pPubDev->ExplainYxByte(pPubDev,nYxIndex,byYx);
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:yc_index,yc_value*/
int luafunc_MailYCOne(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	luaL_checknumber(pLua,3);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYcIndex=lua_tointeger(pLua,2);
			float fValue=lua_tonumber(pLua,3);
			if (nYcIndex>=0) {
				pPubDev->ExplainYc(pPubDev,nYcIndex,fValue);	
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*返回值:soe_unit={year,month,day,hour,min,sec,ms,yx_point,yx_state,action_value}*/
int luafunc_NewSOEUnit(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			SOE_UNIT soe;
			pPubDev->InitSoeUnit(pPubDev,&soe);
			lua_newtable(pLua);
			LUA_SETINTEGER(pLua,-1,"year",soe.m_Time.Year);
			LUA_SETINTEGER(pLua,-1,"month",soe.m_Time.Mon);
			LUA_SETINTEGER(pLua,-1,"day",soe.m_Time.Mday);
			LUA_SETINTEGER(pLua,-1,"hour",soe.m_Time.Hour);
			LUA_SETINTEGER(pLua,-1,"min",soe.m_Time.Min);
			LUA_SETINTEGER(pLua,-1,"sec",soe.m_Time.Sec);
			LUA_SETINTEGER(pLua,-1,"ms",soe.m_Time.MS);
			LUA_SETINTEGER(pLua,-1,"yx_point",-1);
			LUA_SETINTEGER(pLua,-1,"yx_value",0);
			LUA_SETINTEGER(pLua,-1,"action_value",0);
			return 1;
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:soe_unit={year,month,day,hour,min,sec,ms,yx_point,yx_state,action_value}*/
int luafunc_MailSOE(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			if (lua_istable(pLua,2)) {
				int bIsNum;
				int number;
				SOE_UNIT soe;
				pPubDev->InitSoeUnit(pPubDev,&soe);
				
				
				LUA_GETINTEGER(pLua,2,"year",number,&bIsNum);
				if (bIsNum) soe.m_Time.Year=number;
				LUA_GETINTEGER(pLua,2,"month",number,&bIsNum);
				if (bIsNum) soe.m_Time.Mon=number;
				LUA_GETINTEGER(pLua,2,"day",number,&bIsNum);
				if (bIsNum) soe.m_Time.Mday=number;
				LUA_GETINTEGER(pLua,2,"hour",number,&bIsNum);
				if (bIsNum) soe.m_Time.Hour=number;
				LUA_GETINTEGER(pLua,2,"min",number,&bIsNum);
				if (bIsNum) soe.m_Time.Min=number;
				LUA_GETINTEGER(pLua,2,"sec",number,&bIsNum);
				if (bIsNum) soe.m_Time.Sec=number;
				LUA_GETINTEGER(pLua,2,"ms",number,&bIsNum);
				if (bIsNum) soe.m_Time.MS=number;
				
				LUA_GETINTEGER(pLua,2,"yx_point",soe.m_nYxIndex,NULL);
				LUA_GETINTEGER(pLua,2,"yx_value",soe.m_bYxState,NULL);
				LUA_GETNUMBER(pLua,2,"action_value",soe.m_fValue,NULL);
				if (soe.m_nYxIndex>0) {
					soe.m_bYxState=soe.m_bYxState;
					pPubDev->ExplainSoe(pPubDev,&soe);
				}
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:yk_result*/
int luafunc_MailYKResult(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TBOOLEAN);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			BOOL bResult=lua_toboolean(pLua,2);
			pPubDev->RespondResult_YK(pPubDev,pPubDev->m_LastYkKind,bResult);
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:cmd,buffer,frame_type;返回值:frame_buffer*/
int luafunc_MakeFrame(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkinteger(pLua,2);
	luaL_checktype(pLua,3,LUA_TTABLE);
	if (!lua_istable(pLua,1))
		return 0;
	lua_getfield(pLua,1,STR_DEV_UDATA);
	DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1);
	if (pPubDev) {
		BYTE byCmd=(BYTE)lua_tointeger(pLua,2);
		BYTE byFrameType=(BYTE)lua_tointeger(pLua,4);
		if (lua_istable(pLua,3)) {
			char pBuffer[256];
			int nSize=sizeof(pBuffer);
			nSize=BufferToBytes(pLua,3,pBuffer,nSize);
			if (nSize>0) {
				BYTE *pFrameBuffer=NULL;
				WORD wFrameSize=0;
				pFrameBuffer=pPubDev->MakeFrame(pPubDev,byCmd,pBuffer,byFrameType,nSize,&wFrameSize);
				if (pFrameBuffer && wFrameSize>0 && 
					BytesToBuffer(pLua,pFrameBuffer,wFrameSize)>0) {
					return 1;
				}
			}
		}
	}
	
	return 0;
}
/*参数:buffer,recv_proc*/
int luafunc_Send(lua_State *pLua)
{	
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			if (lua_istable(pLua,2)) {
				char pBuffer[256];
				int nSize=sizeof(pBuffer);
				nSize=BufferToBytes(pLua,2,pBuffer,nSize);
				if (nSize>0) {
					const char *pRecvProcName=lua_tostring(pLua,3);

					pPubDev->SendFrame(pPubDev,pBuffer,nSize,pRecvProcName);
				}
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:keep_sec*/
int luafunc_SetKeepSend(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int sec=lua_tointeger(pLua,2);
			pPubDev->SetKeepSend(pPubDev,sec);
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:print_string*/
int luafunc_DebugPrint(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkstring(pLua,2);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		DEV_CLASS *pPubDev=(DEV_CLASS*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			const char *pStr=lua_tostring(pLua,2);
			TRACE(pStr);
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}

static struct luaL_Reg funcsScriptBaseClass[] = {
	{"LoadFrame", luafunc_LoadFrame},
	{"PollSend", luafunc_PollSend},
	{"PollSendProc", luafunc_PollSendProc},
	{"GetDeviceNo", luafunc_GetDeviceNo},
	{"GetChannelNo", luafunc_GetChannelNo},
	{"GetLinkAddr", luafunc_GetLinkAddr},
	{"GetLinkAddr2", luafunc_GetLinkAddr2},
	{"GetParam1", luafunc_GetParam1},
	{"GetParam2", luafunc_GetParam2},
	{"SetLinkAddr", luafunc_SetLinkAddr},
	{"GetYXOne", luafunc_GetYXOne},
	{"GetYCOne", luafunc_GetYCOne},
	{"MailYXOne", luafunc_MailYXOne},
	{"MailYXByte", luafunc_MailYXByte},
	{"MailYCOne", luafunc_MailYCOne},
	{"NewSOEUnit", luafunc_NewSOEUnit},
	{"MailSOE", luafunc_MailSOE},
	{"MailYKResult", luafunc_MailYKResult},
	{"MakeFrame", luafunc_MakeFrame},
	{"Send", luafunc_Send},
	{"SetKeepSend", luafunc_SetKeepSend},
	{"DebugPrint", luafunc_DebugPrint},
	{NULL, NULL}
};
BOOL InitScriptBaseclass(lua_State *pLua)
{
	if (NULL==pLua)
		return FALSE;
	//////////////////////////////////////////////////////////////////////////
	//创建ScriptBaseClass元表
	luaL_newmetatable(pLua, STR_SCRIPTE_BASECLASS);
	//元表.__index=元表
	lua_pushvalue(pLua, -1);
	lua_setfield(pLua, -2, "__index");
	luaL_setfuncs(pLua,funcsScriptBaseClass,0);
	lua_pop(pLua,1); //pop STR_SCRIPTE_BASECLASS
	//////////////////////////////////////////////////////////////////////////
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// static struct luaL_Reg funcsBufferBaseClass[]= {
// 	{"DWordToFloat",luafunc_DWordToFloat},
// 	{"FloatToDWord",luafunc_FloatToDWord}
// };
//////////////////////////////////////////////////////////////////////////
