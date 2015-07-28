/*
 * CFlashsimplefactory.cpp
 *
 *  Created on: 2015¦~6¤ë18¤é
 *      Author: Coolio
 */

#include "CFlashsimplefactory.h"





CFlashsimplefactory::CFlashsimplefactory(SettingConfgInfo *pCurSettingConfgInfo) {
	// TODO Auto-generated constructor stub
	pmCurSettingConfgInfo = pCurSettingConfgInfo;


}

CFlashsimplefactory::~CFlashsimplefactory() {
	// TODO Auto-generated destructor stub
}

CFlash * CFlashsimplefactory::CreatFalsh(UINT ID) {
	// TODO Auto-generated destructor stub
	IeMMCDriver *pFDriver;
	CFlash *pflash;
	FlashStructure *tFlashStructure;
	CFlashStructureFactory *pFlashStructureFactory;
	pFlashStructureFactory = new CFlashStructureFactory();

	tFlashStructure = NULL;
	pFDriver = NULL;

	if(ID == Th58TE67DDJBA4C){
		tFlashStructure = pFlashStructureFactory->CreatFlashStructure_TH58TE67DDJBA4C(pmCurSettingConfgInfo);
		pFDriver = new CeMMCDriver();

	}else if(ID == Th58TE67DDKBA4C){

		tFlashStructure = pFlashStructureFactory->CreatFlashStructure_TH58TE67DDKBA4C(pmCurSettingConfgInfo);
		pFDriver = new CeMMCDriver();
	}

	pflash = new CFlash(pFDriver,tFlashStructure);
	return pflash;
}
