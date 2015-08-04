/*
 * TH58TE67DDJBA4C.cpp
 *
 *  Created on: 2015�~6��17��
 *      Author: Coolio
 */

#include "TH58TE67DDKBA4C.h"
#include "IFlashStructure.h"
#include <string.h>
extern TurboPageInfo *creatTurboPageInfo(SettingConfgInfo *CurSettingConfgInfo);


TH58TE67DDKBA4C::TH58TE67DDKBA4C() {
	// TODO Auto-generated constructor stub
	pmFlashStructure = new FlashStructure();
	pmFlashFwScheme = new Flash_FwScheme();
}

TH58TE67DDKBA4C::~TH58TE67DDKBA4C() {
	// TODO Auto-generated destructor stub
}
//--------------------------------------
FlashStructure * TH58TE67DDKBA4C::CreatFlashStructure(SettingConfgInfo *pCurSettingConfgInfo) {
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
	pmFlashFwScheme->LessBlock=84;
	pmFlashFwScheme->LessPage=256;
	pmFlashFwScheme->BlockPage=256;
	pmFlashFwScheme->PageSEC=32;
	pmFlashFwScheme->SelectPlane=2;
	pmFlashFwScheme->PlaneBlock=1066;

	if(pCurSettingConfgInfo->ForceCE!=0)
		pmFlashFwScheme->SelectNO=pCurSettingConfgInfo->ForceCE;
	if(pCurSettingConfgInfo->ForceCH!=0)
		pmFlashFwScheme->ChannelNO=pCurSettingConfgInfo->ForceCH;
	if(pmFlashFwScheme->InterleaveNO== 0)
		pmFlashFwScheme->InterleaveNO = pCurSettingConfgInfo->ForceCE;		// 0x122
	if(pmFlashFwScheme->VB_Block== 0)
		pmFlashFwScheme->VB_Block= pCurSettingConfgInfo->ForceCE * pCurSettingConfgInfo->ForceCH;	// 0x130
	if(pmFlashFwScheme->Select_VB== 0)
		pmFlashFwScheme->Select_VB= pCurSettingConfgInfo->ForceCH;			// 0x131
    //Capacity
	PhysicalCapacity = PhysicalCapacity<<(pmFlashFwScheme->Model5 &0x0F);
	PhysicalCapacity = PhysicalCapacity * (pCurSettingConfgInfo->ForceCE) * (pCurSettingConfgInfo->ForceCH);
	pmFlashFwScheme->Capacity = PhysicalCapacity;
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
