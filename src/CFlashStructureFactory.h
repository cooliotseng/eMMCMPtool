/*
 * CFlashFactory.h
 *
 *  Created on: 2015¦~6¤ë17¤é
 *      Author: Coolio
 */

#ifndef CFLASHSTRUCTUREFACTORY_H_
#define CFLASHSTRUCTUREFACTORY_H_
#include "common.h"
#include "IFlashStructure.h"

class CFlashStructureFactory {
	IFlashStructure * pmFlashStructure;
public:
	CFlashStructureFactory();
	virtual ~CFlashStructureFactory();
	FlashStructure *CreatFlashStructure_TH58TE67DDJBA4C(SettingConfgInfo *pCurSettingConfgInfo);
	FlashStructure *CreatFlashStructure_TH58TE67DDKBA4C(SettingConfgInfo *pCurSettingConfgInfo);
};



#endif /* CFLASHSTRUCTUREFACTORY_H_ */
