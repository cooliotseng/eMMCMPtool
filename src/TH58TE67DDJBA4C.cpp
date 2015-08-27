/*
 * TH58TE67DDJBA4C.cpp
 *
 *  Created on: 2015�~6��17��
 *      Author: Coolio
 */

#include "TH58TE67DDJBA4C.h"
#include "IFlashStructure.h"
#include <string.h>
extern TurboPageInfo *creatTurboPageInfo(SettingConfgInfo *CurSettingConfgInfo);


TH58TE67DDJBA4C::TH58TE67DDJBA4C() {
	// TODO Auto-generated constructor stub
	pmFlashStructure = new FlashStructure();
	pmFlashFwScheme = new Flash_FwScheme();
}

TH58TE67DDJBA4C::~TH58TE67DDJBA4C() {
	// TODO Auto-generated destructor stub
}
//--------------------------------------
FlashStructure * TH58TE67DDJBA4C::CreatFlashStructure(SettingConfgInfo *pCurSettingConfgInfo) {
	// TODO Auto-generated destructor stub
	UINT   tFLH_ID[8] = {152,222,148,147,118,215,8,4};
	UINT PhysicalCapacity=0x200000;

	memcpy( &pmFlashFwScheme->FLH_ID,&tFLH_ID,sizeof(UINT)*8);
	pmFlashFwScheme->Model0=101;
	pmFlashFwScheme->Model1=255;
	pmFlashFwScheme->Model2=49;
	pmFlashFwScheme->Model3=149;
	pmFlashFwScheme->Model4=8;
	pmFlashFwScheme->Model5=35;
	pmFlashFwScheme->Model6=24;
	pmFlashFwScheme->Model7=0;
	pmFlashFwScheme->LessBlock=68;
	pmFlashFwScheme->LessPage=256;
	pmFlashFwScheme->BlockPage=256;
	pmFlashFwScheme->PageSEC=32;
	pmFlashFwScheme->SelectPlane=2;
	pmFlashFwScheme->PlaneBlock=1058;

	pmFlashStructure->FlashFwScheme = pmFlashFwScheme;
    //PlaneNum
	pmFlashStructure->PlaneNum = 0x01 << ((pmFlashFwScheme->Model6 & 0x30) >> 4);
	//BlockPage
	pmFlashStructure->BlockPage = pmFlashFwScheme->BlockPage;
	//PageSize
	pmFlashStructure->PageSize = getPageSize(pmFlashFwScheme);

	pmFlashStructure->ChipSelectNum = pCurSettingConfgInfo->ForceCE;

	pmFlashStructure->ChannelNum = pCurSettingConfgInfo->ForceCH;

    //EntryItemNum
	pmFlashStructure->EntryItemNum = getEntryItemNum(pmFlashFwScheme);

	pmFlashStructure->ForceCE = pCurSettingConfgInfo->ForceCE;

	pmFlashStructure->ForceCH =  pCurSettingConfgInfo->ForceCH;

	pmFlashStructure->BaseFType = Toshiba_M_2P;

	pmFlashStructure->turbopageinfo = creatTurboPageInfo(pCurSettingConfgInfo);

	return pmFlashStructure;
}
