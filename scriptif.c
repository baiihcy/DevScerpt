#include "scriptif.h"

/*参数:frame_name;返回值:load_success*/
int luafunc_LoadFrame(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checkstring(pLua,2);
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
/*参数:interval_sec,buffer,recv_proc,use_frame*/
int luafunc_PollSend(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:interval_sec,send_proc*/
int luafunc_PollSendPeoc(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	printf("\n luafunc_PollSendPeoc");
	return 0;
}
/*返回值:device_no*/
int luafunc_GetDeviceNo(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	if (!lua_istable(pLua,1) || LUA_TLIGHTUSERDATA!=lua_getfield(pLua,1,STR_DEV_UDATA)) {
		lua_pop(pLua,1); //pop STR_DEV_UDATA
		return -1;
	}
	CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYxIndex=lua_tointeger(pLua,2);
			int nYxValue=lua_tointeger(pLua,3);
			printf("\n MailYXOne %d %d",nYxIndex,nYxValue);
			if (nYxIndex>0) {
				pPubDev->ExplainYx(pPubDev,nYxIndex+1,nYxValue);
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYxIndex=lua_tointeger(pLua,2);
			BYTE byYx=(BYTE)lua_tointeger(pLua,3);
			if (nYxIndex>0) {
				pPubDev->ExplainYxByte(pPubDev,nYxIndex+1,byYx);
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			int nYcIndex=lua_tointeger(pLua,2);
			float fValue=lua_tonumber(pLua,3);
			//printf("\n %d %f",nYcIndex,fValue);
			if (nYcIndex>0) {
				pPubDev->ExplainYc(pPubDev,nYcIndex-1,fValue);	
			}
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:soe_table={year,month,day,hour,min,sec,ms,yx_point,yx_state,action_value}*/
int luafunc_MailSOE(lua_State *pLua)
{
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
					soe.m_bYxState=soe.m_bYxState+1;
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			BOOL bResult=lua_toboolean(pLua,2);
			pPubDev->RespondResult_YK(pPubDev,pPubDev->YkOperation,bResult);
		}
	}
	lua_pop(pLua,1); //pop STR_DEV_UDATA
	return 0;
}
/*参数:cmd,buffer,frame_type*/
int luafunc_MakeFrame(lua_State *pLua)
{
	struct DEVICE_CALSS *pDevClass=lua_touserdata(pLua,1/*,STR_SCRIPTE_BASECLASS*/);
	printf("\n luafunc_MakeFrame");
	return 0;
}
/*参数:buffer,recv_proc,use_frame*/
int luafunc_Send(lua_State *pLua)
{	
	luaL_checktype(pLua,1,LUA_TTABLE);
	luaL_checktype(pLua,2,LUA_TTABLE);
	if (lua_istable(pLua,1) && LUA_TLIGHTUSERDATA==lua_getfield(pLua,1,STR_DEV_UDATA)) {
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
		if (pPubDev) {
			if (lua_istable(pLua,2)) {
				char pBuffer[256];
				int nSize=sizeof(pBuffer);
				nSize=BufferToBytes(pLua,2,pBuffer,nSize);
				if (nSize>0) {
					pPubDev->SendFrame(pPubDev,0,pBuffer[0],pBuffer+1,nSize-1,NULL);
					
					const char *pRecvProcName=lua_tostring(pLua,3);
					if (pRecvProcName) {
						strcpy(pPubDev->szRecvProc,pRecvProcName);
						pPubDev->bUseRecvProc=TRUE;
					}
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
		CPubDev *pPubDev=(CPubDev*)lua_touserdata(pLua,-1);
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
	{"PollSendPeoc", luafunc_PollSendPeoc},
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

/*
说明:触发OnInit接口
返回值:找不到OnInit接口直接返回TRUE，否则返回OnInit接口的返回值
*/
int HandleOnInit(lua_State *pLua, struct _PubDev *pPubDev)
{
	if (!pLua || !pPubDev) return -1;
	BOOL bRet=TRUE;
	if (CallInterface(pLua,pPubDev,"OnInit",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	return bRet;
}

/*
说明:触发OnSend接口
返回值:找不到OnSend接口直接返回FALSE，否则返回OnSend接口的返回值
*/
int HandleOnSend(lua_State *pLua, struct _PubDev *pPubDev)
{
	if (!pLua || !pPubDev) return -1;
	BOOL bRet=TRUE;

	if (CallInterface(pLua,pPubDev,"OnSend",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	return bRet;
}

/*
说明:触发OnRecv接口
返回值:找不到OnRecv接口直接返回TRUE，否则返回OnRecv接口的返回值
*/
int HandleOnRecv(lua_State *pLua, struct _PubDev *pPubDev, BYTE *pBuffer, int nSize)
{
	if (!pLua || !pPubDev ||  !pBuffer || nSize<=0) 
		return -1;
	BOOL bRet=TRUE;
	BOOL bDone=FALSE;
	if (BytesToBuffer(pLua,pBuffer,nSize)>=0) {
		if (!bDone && pPubDev->bUseRecvProc) {
			pPubDev->bUseRecvProc=FALSE;
			if (CallInterface(pLua,pPubDev,pPubDev->szRecvProc,1,1)) {
				bRet=lua_toboolean(pLua,-1);
				lua_pop(pLua, 1); //pop lua_pcall return value
				bDone=TRUE;
			}
		}

		if (!bDone && CallInterface(pLua,pPubDev,"OnRecv",1,1)>0) {
			bRet=lua_toboolean(pLua,-1);
			lua_pop(pLua, 1); //pop lua_pcall return value
			bDone=TRUE;
		}

		lua_pop(pLua,1); //pop buffer
	}
	return bRet;
}

/*
说明:触发OnYkSelect/OnYkExecute接口
返回值:找不到接口直接返回FALSE，否则返回接口的返回值
*/
int HandleOnYk(lua_State *pLua, struct _PubDev *pPubDev, enum yk_Kind YkKind, BYTE byYkGroup, BOOL bYkOnoff)
{
	if (!pLua || !pPubDev) return -1;
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
	pPubDev->YkOperation=YkKind;
	lua_pushinteger(pLua,byYkGroup);
	lua_pushinteger(pLua,bYkOnoff);
	if (CallInterface(pLua,pPubDev,szInterface,2,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	lua_pop(pLua,2); //pop CallInterface args
	return bRet;
}

/*
说明:触发OnReset接口
返回值:找不到OnReset接口直接返回FALSE，否则返回OnReset接口的返回值
*/
int HandleOnReset(lua_State *pLua, struct _PubDev *pPubDev)
{
	if (!pLua || !pPubDev) return -1;
	BOOL bRet=FALSE;
	if (CallInterface(pLua,pPubDev,"OnReset",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	return bRet;
}

/*
说明:触发OnSetTime接口
返回值:找不到OnSetTime接口直接返回FALSE，否则返回OnSetTime接口的返回值
*/
int HandleOnSetTime(lua_State *pLua, struct _PubDev *pPubDev)
{
	if (!pLua || !pPubDev) return -1;
	BOOL bRet=FALSE;
	if (CallInterface(pLua,pPubDev,"OnSetTime",0,1)>0) {
		bRet=lua_toboolean(pLua,-1);
		lua_pop(pLua, 1); //pop lua_pcall return value
	}
	return bRet;
}