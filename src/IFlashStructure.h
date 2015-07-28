/*
 * IFlashStructure.h
 *
 *  Created on: 2015¦~6¤ë17¤é
 *      Author: Coolio
 */

#ifndef IFLASHSTRUCTURE_H_
#define IFLASHSTRUCTURE_H_
#include "common.h"


class IFlashStructure {
public:
	IFlashStructure();
	virtual ~IFlashStructure();
	virtual FlashStructure *CreatFlashStructure(SettingConfgInfo *pCurSettingConfgInfo)=0;
	INT getEntryItemNum(Flash_FwScheme *pFlashFwScheme);
	UINT getPageSize(Flash_FwScheme *pFlashFwScheme);
};

#endif /* IFLASHSTRUCTURE_H_ */
