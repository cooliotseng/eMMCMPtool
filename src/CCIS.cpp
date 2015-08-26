/*
 * CCIS.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#include "CCIS.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

extern ULONG FileSize(FILE *fp);

CCIS::CCIS() {
	// TODO Auto-generated constructor stub
	pmCISInfo = NULL;
	pmflash = NULL;
	pmCISDL = NULL;
}

CCIS::CCIS(ICISDL *pCISDL,SettingConfgInfo *CurSettingConfgInfo,CFlash *pflash,CRootTable *roottable,ULONG ISPTotalByte) {
	// TODO Auto-generated constructor stub
	ULONG	ExtFileRealByte,EachTxSize;
	UINT Status = false;
	DWORD	dwBytesRead;
	FILE	*VenFile;
	BYTE 	tPageSEC;
	UINT 	tBlockPage;
	BYTE	ValueTmp=0;
	TurboPageInfo *turbopageinfo = pflash->getTurboPageInfo();

	pmflash = pflash;
	pmCISDL = pCISDL;
	tPageSEC = pmflash->getPageSEC();
	tBlockPage = pmflash->getBlockPage();
	memset(Original_EraseCnt,0,sizeof(UINT)*4);

	if( tPageSEC == 32)
		EachTxSize = 12*1024;	// 16K_Flash, Each Page Write 12K
	else
		EachTxSize = 6*1024;

	pmCISInfo=new eMMC_CIS_INFO();
	pmCISInfo->CE_NUM = pmflash->getChipSelectNum();
	pmCISInfo->CH_NUM = pmflash->getChannelNum();
	pmCISInfo->FLH_DRV = CurSettingConfgInfo->FlashDriving;
	pmCISInfo->CTL_DRV = CurSettingConfgInfo->ControllerDriving;
	pmCISInfo->CPU_CLK = CurSettingConfgInfo->CisData.Reserved2[4];
	pmCISInfo->Encryption = CurSettingConfgInfo->CisData.Reserved2[5];
	pmCISInfo->PlaneBlock = pmflash->getPlaneBlock();
	pmCISInfo->FW_PAGE = (BYTE)(ISPTotalByte/EachTxSize); // This Code Must Be After BIN File Has Read
	pmCISInfo->CIS_PAGE = (BYTE)(sizeof(eMMC_CIS_INFO)/EachTxSize);
	pmCISInfo->CIS_Version = 0x01; // Sherlock_20140801, For A2CMD PairMaps
	pmCISInfo->EACH_PAGE = (BYTE)(EachTxSize/1024);

	if(CurSettingConfgInfo->TestProcedureMask & Proc_DisableReset)	//No Reset
		pmCISInfo->PRE_LOAD_DATA = 0;	// If NO_RESET, 		PreLoad = 0
	else if(CurSettingConfgInfo->LunTypeBitMap & 0x0F) // Sherlock_20140801, .FormatLunBitMap & 0x0F
		pmCISInfo->PRE_LOAD_DATA = 1;	// If RESET & LUN, 		Preload = 1
	else
		pmCISInfo->PRE_LOAD_DATA = 0;	// If RESET & NO_LUN, 	PreLoad = 0

	if(tBlockPage == 64)
		ValueTmp = 0; //[1:0] = xx00b
	else if(tBlockPage == 128)
		ValueTmp = 1; //[1:0] = xx01b
	else if(tBlockPage == 256)
		ValueTmp = 2; //[1:0] = xx10b
	else if(tBlockPage == 512)
		ValueTmp = 3; //[1:0] = xx11b

	if(tPageSEC == 32)
		ValueTmp = (ValueTmp & (0xF3)) + 0; // 16K_Page, [3:2] = 00xxb
	else if(tPageSEC == 8)
		ValueTmp = (ValueTmp & (0xF3)) + 4; //   4K_Page, [3:2] = 01xxb
	else if(tPageSEC == 16)
		ValueTmp = (ValueTmp & (0xF3)) + 8; //   8K_Page, [3:2] = 10xxb

	pmCISInfo->FLH_INFO = ValueTmp;

	memcpy(&eCISADDR[0],getCISBlocksAddr(CurSettingConfgInfo,roottable),sizeof(UINT)*4);

	memcpy(&pmCISInfo->CIS_RowAddr[0],&eCISADDR[0],sizeof(UINT)*4);

		// Copy Turbo Page
		pmCISInfo->TP_NUM = turbopageinfo->TurboPageNUM;
		for(int i=0; i<(int)pmCISInfo->TP_NUM; i++)
			pmCISInfo->TurboPage[i] = turbopageinfo->TurboPage[i];


		// Set CSD & ExtCSD Data, Sherlock_20131122
		pmCISInfo->CheckSumLen = (USHORT)offsetof(eMMC_CIS_INFO, Bit_Map[0]);
		memcpy(pmCISInfo->CSD, Default_CSD, sizeof(Default_CSD));
		memcpy(pmCISInfo->Ext_CSD, Default_ExtCSD, sizeof(Default_ExtCSD));

		// ----- 1. Get VenCID File -----
		ExtFileRealByte = 0;
		if(CurSettingConfgInfo->VenCIDFileName.length() != 0)
		{

			VenFile = fopen(CurSettingConfgInfo->VenCIDFileName.c_str(),"r");
				if(VenFile != NULL)
				{
					ExtFileRealByte = FileSize(VenFile); // Get Real ExtCSD Size
					if(ExtFileRealByte == 16) // Protection
					{
						dwBytesRead =fread(pmCISInfo->CID,sizeof(char),16,VenFile);
					}

					fclose(VenFile);
				}

		}


		// ----- 2. Get VenCSD File -----
		ExtFileRealByte = 0;
		if(CurSettingConfgInfo->VenCSDFileName.length() != 0)
		{
			VenFile = fopen(CurSettingConfgInfo->VenCSDFileName.c_str(),"r");

				if(VenFile != NULL)
				{
					ExtFileRealByte = FileSize(VenFile); // Get Real ExtCSD Size
					if(ExtFileRealByte == 16) // Protection
					{
						dwBytesRead =fread(pmCISInfo->CSD,sizeof(char),16,VenFile);

					}

					fclose(VenFile);
				}

		}


		// ----- 3. Get ExtCSD File -----

		ExtFileRealByte = 0;
		if(CurSettingConfgInfo->ExtCSDFileName.length() != 0)
		{

			VenFile = fopen(CurSettingConfgInfo->ExtCSDFileName.c_str(),"r");
				if(VenFile != NULL)
				{
					ExtFileRealByte = FileSize(VenFile); // Get Real ExtCSD Size
					if(ExtFileRealByte == 512) // Protection
					{
						dwBytesRead =fread(pmCISInfo->Ext_CSD,sizeof(char),512,VenFile);

					}

					fclose(VenFile);
				}

		}

}

CCIS::~CCIS() {
	// TODO Auto-generated destructor stub
}
ICISDL * CCIS::getCISDL() {
	// TODO Auto-generated constructor stub
	return pmCISDL;
}

UINT CCIS::DownloadCIS(SettingConfgInfo *CurSettingConfgInfo,CRootTable *pRootTable) {
	// TODO Auto-generated constructor stub
	UINT Status = false;

	pmCISDL->Execute(CurSettingConfgInfo,pmflash,pRootTable,pmCISInfo,eCISADDR,&Original_EraseCnt[0],pmflash->isOldVersionCISExit(),0);

	return Status;
}

UINT CCIS::ClearCIS() {
	// TODO Auto-generated constructor stub
	return 0;
}

eMMC_CIS_INFO * CCIS::getCISInfo() {
	// TODO Auto-generated constructor stub
	return pmCISInfo;
}

UINT* CCIS::getCISBlocksAddr(SettingConfgInfo *CurSettingConfgInfo,CRootTable *roottable) {
	// TODO Auto-generated constructor stub

	BlockRecTable CisIspTB;
	TableCFG TableCfg;
	Flash_FwScheme*ptFLH_FwScheme = pmflash->getFlashFwScheme();
	ROOT_VARS *RootTable = roottable->getRootTable();
	UINT	index,Status;
	UINT	Address;

	memset(eCISADDR,0xFF,sizeof(UINT)*4);
	memset(Original_EraseCnt,0,sizeof(UINT));

	memset(&TableCfg, 0x00, sizeof(TableCFG));
	TableCfg.MaxChipFind = 1;
	TableCfg.MaxPageFind = 0x40000; // Sherlock_20141007
	if(pmflash->getPlaneNum() == 1)
		TableCfg.MaxVaildCISBlockFind = 1; // Get 2 Blocks for eCIS
	else
		TableCfg.MaxVaildCISBlockFind = 2; // Get 4 Blocks for eCIS

	TableCfg.MaxTableChipFind = 1;	// Only Search Chip_0
	TableCfg.MaxLogBlock = 24;		// No Use
	TableCfg.MaxFreeBlock = 16;	// No Use
	TableCfg.SelectPlane = ptFLH_FwScheme->SelectPlane;
	TableCfg.InternalChip = (0x01 << (ptFLH_FwScheme->FLH_ID[2]&0x03));
	TableCfg.BasicBlock= 0x200<<((ptFLH_FwScheme->Model5&0xF0)>>4);	// Basic Block Number;
	TableCfg.ExtendedBlock = ptFLH_FwScheme->LessBlock;					// Add Extended Block
	TableCfg.AddressPage = ptFLH_FwScheme->BlockPage;
	TableCfg.LessPage = ptFLH_FwScheme->LessPage;
	TableCfg.PageSEC = ptFLH_FwScheme->PageSEC;
	TableCfg.ErrorBitRate = (CurSettingConfgInfo->LunTypeBitMap2 & BitErrorRate);
	TableCfg.ECCSET = ptFLH_FwScheme->Model3 & 0x0F;
	TableCfg.Type|=MustMarkBad_TYPE;//(BIT2); //mark bad when block stress test fail or write system table fail retry 3 times
	TableCfg.BaseFType = pmflash->getFlashType();
	TableCfg.Mode = 0;
	TableCfg.ED3EraseCount = 0;
	TableCfg.RsvInfo = 0;

	if(CurSettingConfgInfo->LunTypeBitMap & DumpDebugMemory)//dump memory for firmware debug use
		TableCfg.Type|=(BIT1);

	CisIspTB.pBlockRec = new BlockRec[TableCfg.MaxVaildCISBlockFind*2];
	CisIspTB.ItemNum = 0;
//�p�G���ª�cis table��X����Address(eCISADDR)
	if(!(CurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)  &&
		!(((RootTable[0].dwRTBLVersion & 0xFFFF0000) != 0x52540000)  &&
			(pmflash->isOldVersionCISExit())))  //Sherlock_20131015, Get New CIS Address ,Special Function of Keep old CIS data For RE-MP ,Cody_20150217
	{   //�qscan flash ���G����X��good block���D�@�ӥi�H�Ϊ�CIS ADDRESS
		Status = getGootlCISABlocksAddr(roottable->getUFDBlockMap(), &CisIspTB, &TableCfg);

		for(index=0; index<TableCfg.MaxVaildCISBlockFind*2; index++) //Block 4,5,6,7 for eCIS Address
		{
			Address = (CisIspTB.pBlockRec[index].CEIndex<<4)|(CisIspTB.pBlockRec[index].CHIndex);
			Address = (Address<<24)|(((ULONG)pmflash->getBlockPage())*(CisIspTB.pBlockRec[index].EntryIndex*8+CisIspTB.pBlockRec[index].BlockIndex));
			eCISADDR[index] = Address;
			Original_EraseCnt[index] = CisIspTB.pBlockRec[index].EraseCount;
		}
	}else if(pmflash->isOldVersionCISExit()){
		pmflash->getOldVersionCISAddress(&eCISADDR[0]);
	}


	return &eCISADDR[0];
}

UINT CCIS::getGootlCISABlocksAddr(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pCisIspTB, LPTableCFG pTableCfg) {
	// TODO Auto-generated constructor stub
	return 0;
}
