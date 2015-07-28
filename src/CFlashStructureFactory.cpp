/*
 * CFlashFactory.cpp
 *
 *  Created on: 2015¦~6¤ë17¤é
 *      Author: Coolio
 */

#include "CFlashStructureFactory.h"
#include "TH58TE67DDJBA4C.h"
#include "TH58TE67DDKBA4C.h"

CFlashStructureFactory::CFlashStructureFactory() {
	// TODO Auto-generated constructor stub
	pmFlashStructure = NULL;
}

CFlashStructureFactory::~CFlashStructureFactory() {
	// TODO Auto-generated destructor stub
}

FlashStructure *CFlashStructureFactory::CreatFlashStructure_TH58TE67DDJBA4C(SettingConfgInfo *pCurSettingConfgInfo) {
	pmFlashStructure = new TH58TE67DDJBA4C();
	return pmFlashStructure->CreatFlashStructure(pCurSettingConfgInfo);
}

FlashStructure *CFlashStructureFactory::CreatFlashStructure_TH58TE67DDKBA4C(SettingConfgInfo *pCurSettingConfgInfo) {
	pmFlashStructure = new TH58TE67DDKBA4C();
	return pmFlashStructure->CreatFlashStructure(pCurSettingConfgInfo);
}
