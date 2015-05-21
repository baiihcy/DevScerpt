#include "types.h"
#pragma once

struct DEV_CLASS;
#define GetPPubDevFromPDeviceUnit(_pDeviceUnit) (struct DEV_CLASS*)_pDeviceUnit->pExDeviceUnit
extern IoGlobal * pGlobal;
extern SCADAUnit*   pScadaUnit;
extern BOOL InitCommunication(struct DEV_CLASS *);



typedef struct DEVICE_VARIABLES_TAG
{
	void* pDevScriptInstance;
}DEVICE_VARIABLES;
