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
#include <unistd.h>

extern ULONG FileSize(FILE *fp);

CCIS::CCIS() {
	// TODO Auto-generated constructor stub
	pmCISInfo = NULL;
	pmflash = NULL;
	pmCISDL = NULL;
	pmroottable = NULL;
	memset(eCISADDR,0xFF,sizeof(UINT)*4);
}

CCIS::CCIS(CRootTable *proottable,ICISDL *pCISDL,SettingConfgInfo *CurSettingConfgInfo,CFlash *pflash,CRootTable *roottable,ULONG ISPTotalByte) {
	// TODO Auto-generated constructor stub
	ULONG	ExtFileRealByte,EachTxSize;
	UINT Status = false;
	DWORD	dwBytesRead;
	FILE	*VenFile;
	BYTE 	tPageSEC;
	BYTE	ValueTmp=0;
	TurboPageInfo *turbopageinfo = pflash->getTurboPageInfo();

	pmflash = pflash;
	pmCISDL = pCISDL;
	pmroottable = proottable;
	tPageSEC = pmflash->getPageSEC();
	m_BlockPage = pmflash->getBlockPage();
	memset(Original_EraseCnt,0,sizeof(UINT)*4);
	memset(eCISADDR,0xFF,sizeof(UINT)*4);
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

	if(m_BlockPage == 64)
		ValueTmp = 0; //[1:0] = xx00b
	else if(m_BlockPage == 128)
		ValueTmp = 1; //[1:0] = xx01b
	else if(m_BlockPage == 256)
		ValueTmp = 2; //[1:0] = xx10b
	else if(m_BlockPage == 512)
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
	UINT Status = 0;

	Status = pmCISDL->Execute(CurSettingConfgInfo,pmflash,pRootTable,eCISADDR,&Original_EraseCnt[0],0);

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
	UINT	Status = false;
	BOOL	AR_Patch = false;
	BYTE	Plane = 2;
	UINT 	EntryIndex = 0, BlockIndex = 0;

	Plane = pTableCfg->SelectPlane;

	if(pTableCfg->Mode == FlashSorting_Mode)
	{
		if((pTableCfg->BaseFType & FT_Cell_Level) == FT_MLC)
			Status = MLCSortingFromTable(pUFDBlockMap, pCisIspTB, pTableCfg,(pTableCfg->MaxVaildCISBlockFind*2));
		else
			Status = TLCSortingFromTable(pUFDBlockMap, pCisIspTB, pTableCfg,(pTableCfg->MaxVaildCISBlockFind*2));
		return Status;
	}

	if(pTableCfg->MaxVaildCISBlockFind == 0) // Sherlock_20110928, Special Flag of Write/Read Test in PreMPTestMode, "0" Is Enable
	{
		pTableCfg->MaxVaildCISBlockFind = 1; // Set Real Read Write Test Block Number Is 2 Block of Each Die
		Status=PreMPTestFromTable(pUFDBlockMap, pCisIspTB, pTableCfg, (pTableCfg->MaxVaildCISBlockFind*2));
		return Status;
	}

	if(!Status)
	{
		if(Plane == 4)
   			Status=FindQuadriBlockFromTable(pUFDBlockMap, pCisIspTB, pTableCfg, (pTableCfg->MaxVaildCISBlockFind*2));
		else
			Status=FindPairBlockFromTable(pUFDBlockMap, pCisIspTB, pTableCfg, (pTableCfg->MaxVaildCISBlockFind*2));
	}

	return Status;
}

UINT CCIS::MLCSortingFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount)
{ // Sherlock_20130314, Only Use in SortingTool. ReWrite From Find Pair Block From Table
	BOOL	Status=false;
	BYTE 	CE, CH, ECCValue, EraseStatus, Retry=0, RetryMax=10, CmdErrorCnt=0, Plane;
	WORD	BlockIndex;
	UINT	Lun1End, Lun2Start, BlockEnd, MarkBadCnt=0;	// For 2 Die
	ULONG	ChipAddress, Address, TxLen, Cnt;			// For Mark Bad
	BYTE	*pTxBuf;
	BYTE	Debug_Mode = 0;				// For Debug Testing

	// ----- Dump Sorting Record Open -----
	BYTE 	DumpRec = 0;
	if(pTableCfg->Type & DmpForFW_TYPE)		DumpRec = 1;
	FILE * SortingRec;

	// ---------- Set Initial Values ----------
	CE = (BYTE)((pTableCfg->ED3EraseCount & 0x0000FF00)>>8);	// TLC Sorting ECC Limit
	CH = (BYTE)(pTableCfg->ED3EraseCount & 0x000000FF);		// MLC Sorting ECC Limit
	Plane = pTableCfg->SelectPlane;

	if(Debug_Mode == 1)
	{
		ItemCount = 32 ;						// Only Test 32 Block for Debugging
		pTableCfg->Type &= (~MustMarkBad_TYPE);	// Not Mark Bad in Debugging
	}
	else
		pTableCfg->Type |= MustMarkBad_TYPE;	//Mark Bad Enable

	// ---------- Get 2Die Block Range ----------
	if((pTableCfg->InternalChip==2) && (pTableCfg->ExtendedBlock!=0)) // With EntendedBlock (Toshiba & Samsung 2Die)
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = 2 * pTableCfg->BasicBlock;
		BlockEnd = 3 *pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
	}
	else if(pTableCfg->InternalChip==2)	// No ExtendedBlock (Micron 2Die)
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = pTableCfg->BasicBlock;
		BlockEnd = 2 *pTableCfg->BasicBlock;
	}
	else
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = 0;
		BlockEnd = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
	}

	// ---------- Erase All Block (By Single Block) ----------
	for(CE=0; CE<pUFDBlockMap->ChipSelectNum; CE++)
	{
		for(CH=0; CH<pUFDBlockMap->ChannelNum; CH++)
		{
			ChipAddress = (CE<<28)|(CH<<24);
			for(BlockIndex=0; BlockIndex<BlockEnd; BlockIndex++)
			{
				if((pTableCfg->InternalChip==2) && (pTableCfg->ExtendedBlock!=0))
				{
					if((Lun1End <= BlockIndex) && (BlockIndex < Lun2Start)) // Non-Exist Block, Skip Write
						continue;
				}
				Address = ChipAddress|(BlockIndex*m_BlockPage);
				Retry = 0;
ReDo_EraseBlock:
				Status = pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);
				if(!Status)
				{
					usleep(20);
					Retry++;
					if(Retry < RetryMax)
						goto ReDo_EraseBlock;
					else
						goto EndOfMLCSorting;
				}
			}
		}
	}


	// ---------- Write All Block (By Virtual Page) ----------
	TxLen = 32*1024; // Must 32K
	pTxBuf = new BYTE[TxLen];
	for(Cnt=0; Cnt<TxLen;Cnt++)
		pTxBuf[Cnt] = rand();

	Retry = 0;
ReDo_FillMainFIFO:
	Status = pmflash->FillMainFIFO(MI_SORT_FillFIFO, TxLen, pTxBuf);

	if(!Status)
	{
		usleep(20000);
		Retry++;
		if(Retry < RetryMax)
			goto ReDo_FillMainFIFO;
		else
			goto EndOfMLCSorting;
	}


	for(BlockIndex=0; BlockIndex<Lun1End; BlockIndex+=Plane)
	{
		if(Debug_Mode == 1)
		{
			if (BlockIndex>ItemCount)
				break;
		}
		Retry = 0;
ReDo_MLCVBWrite:
		Status = pmflash->MLCVBWrite(MI_SORT_COPYMLC, (WORD)BlockIndex, (WORD)Lun2Start);

		if(!Status)
		{
			usleep(20000);
			Retry++;
			if(Retry < RetryMax)
				goto ReDo_MLCVBWrite;
			else
				goto EndOfMLCSorting;
		}
	}

	// ---------- Read All Block ECC & Erase (By Single Block) ----------
	for(CE=0; CE<pUFDBlockMap->ChipSelectNum; CE++)
	{
		for(CH=0; CH<pUFDBlockMap->ChannelNum; CH++)
		{
			ChipAddress = (CE<<28)|(CH<<24);
			for(BlockIndex=0; BlockIndex<BlockEnd; BlockIndex++)
			{
				if(Debug_Mode == 1)
				{
					if (BlockIndex>ItemCount)
						break;
				}
				if((pTableCfg->InternalChip==2) && (pTableCfg->ExtendedBlock!=0))
				{
					if((Lun1End <= BlockIndex) && (BlockIndex < Lun2Start)) // Non-Exist Block, Skip Read
						continue;
				}

				Address = ChipAddress|(BlockIndex*m_BlockPage);
				if(BlockECCRead(CE, CH, 0, BlockIndex, pTableCfg, &ECCValue))
				{

				}
				else
					MarkBadCnt++;

				if(ECCValue == 0xCC)
				{
					pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
				}

				if(ECCValue == 0xFF)
				{
					if(++CmdErrorCnt > RetryMax)
						goto EndOfMLCSorting;
				}
			}
		}
	}

EndOfMLCSorting:
	pUFDBlockMap->BadBlockCnt[0] = MarkBadCnt; // Sent Total Mark Bad Count To UI

	if(pTxBuf)
		delete pTxBuf;

	return Status;
}

UINT CCIS::TLCSortingFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount)
{ // Sherlock_20130314. Only Use in SortingTool. ReWrite From Find Pair Block From Table
	BOOL	Status=false;
	BYTE 	CE, CH, ECCValue, Plane, VB_Pair, VB_Chk, Retry=0, RetryMax=10, CmdErrorCnt=0;
	WORD	VB_Block, VB_m = 0, VB_n = 0, VB_Idx = 0, VB_SLC = 0, BlockIndex, MinBlockIndex;	// For 3 SLC VB
	WORD	VB_LFT[48]; 						// Max = 4CE*2CH*2Die*3VB
	UINT	Lun1End, Lun2Start, BlockEnd, ItemCount0;	// For 2 Die
	ULONG	ChipAddress, Address, Address1, MarkBadCnt=0;			// For Mark Bad
	BYTE	Debug_Mode = 0;				// For Debug Testing
	WORD	SLCEnd[4][2][2];			//Save SLC Block

	// ----- Dump Sorting Record Open -----
	BYTE DumpRec = 0;
	if(pTableCfg->Type & DmpForFW_TYPE)		DumpRec = 1;
	FILE * SortingRec;
	// ---------- Set Initial Values ----------
	CE = (BYTE)((pTableCfg->ED3EraseCount & 0x0000FF00)>>8);	// TLC Sorting ECC Limit
	CH = (BYTE)(pTableCfg->ED3EraseCount & 0x000000FF);			// MLC Sorting ECC Limit

	VB_Block = pUFDBlockMap->ChipSelectNum * pUFDBlockMap->ChannelNum * pTableCfg->InternalChip; // =CE*CH*Internal_Die

	// ---------- Get 2Die Block Range ----------
	if((pTableCfg->InternalChip==2) && (pTableCfg->ExtendedBlock!=0)) // With EntendedBlock (Toshiba & Samsung 2Die)
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = 2 * pTableCfg->BasicBlock;
		BlockEnd = 3 *pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
	}
	else if(pTableCfg->InternalChip==2)	// No ExtendedBlock (Micron 2Die)
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = pTableCfg->BasicBlock;
		BlockEnd = 2 *pTableCfg->BasicBlock;
	}
	else
	{
		Lun1End = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
		Lun2Start = 0;
		BlockEnd = pTableCfg->BasicBlock + pTableCfg->ExtendedBlock;
	}

	MinBlockIndex = BlockEnd;
	Plane = pTableCfg->SelectPlane;

   // Sherlock_20130412
	UINT	BlkNum, UsedBlk = 0, LFTNum, BuildCFG = 0;	// SpecialForED3
	BlkNum = (UINT)pTableCfg->InternalChip * (pTableCfg->BasicBlock + pTableCfg->ExtendedBlock);	// All Blocks in 1 Chip
	UsedBlk = (1 + 2 + 2 + pTableCfg->MaxLogBlock + pTableCfg->MaxFreeBlock) * Plane;				// All Used Blocks in 1 Chip
	LFTNum = (BlkNum - UsedBlk) * pUFDBlockMap->ChipSelectNum*pUFDBlockMap->ChannelNum / (1024 * Plane)+1;	// Predict LFT Num + 1

	if(((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) && (Plane == 2))
	{
		ItemCount0 = 111 + 2*LFTNum;
		ItemCount = 104;
	}
	else
	{
		ItemCount0 = 104 + LFTNum;
		ItemCount = 99;
	}

	if(Plane == 2)		VB_Chk = BIT1|BIT0;
	else				VB_Chk = BIT3|BIT2|BIT1|BIT0;

	if(Debug_Mode == 1)
	{
		//ItemCount = 16 ;						// Only Test 16 SLC Block
		pTableCfg->Type &= (~MustMarkBad_TYPE);	// Not Mark Bad in Debugging
	}
	else
		pTableCfg->Type |= MustMarkBad_TYPE;	//Mark Bad Enable

	// ---------- Test SLC Block For Lun 0 ----------
	for(CE=0; CE<pUFDBlockMap->ChipSelectNum; CE++)
	{
		for(CH=0; CH<pUFDBlockMap->ChannelNum; CH++)
		{
			ChipAddress = (CE<<28)|(CH<<24);
			pBlockRecTable->ItemNum = 0;
			for(BlockIndex=0; BlockIndex<Lun1End; BlockIndex+=Plane)
			{
					VB_Pair = 0;
					// ----- Test Plane_0 -----
					Address1 = ChipAddress|(BlockIndex*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT0;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address1, ECCValue);	OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

					// ----- Test Plane_1 -----
					Address = ChipAddress|((BlockIndex+1)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+1, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT1;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue);	OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

				if(Plane == 4)
				{
					// ----- Test Plane_2 -----
					Address = ChipAddress|((BlockIndex+2)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+2, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT2;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue);	OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

					// ----- Test Plane_3 -----
					Address = ChipAddress|((BlockIndex+3)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+3, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT3;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue); OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);

					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

				}

				if(VB_Pair==VB_Chk)
				{
					pBlockRecTable->ItemNum++;
					Address = ChipAddress|(BlockIndex*m_BlockPage);
				}

				// --- Get Good Block Address for 3 SLC VB ---
				if((VB_Pair==VB_Chk) && (VB_n<3))
				{
					VB_Idx = VB_m + VB_n * VB_Block;
					VB_LFT[VB_Idx] = BlockIndex;
					VB_n++;
					VB_SLC++;
				}

				if((CE==0) && (CH==0)) // Sherlock_20130412
				{
					if(pBlockRecTable->ItemNum >= ItemCount0)
						break;
				}
				else
				{
					if(pBlockRecTable->ItemNum >= ItemCount)
						break;
				}

			}
			VB_m++;
			VB_n = 0;

			SLCEnd[CE][CH][0] = BlockIndex;		//Save SLC End

			// ----- Save Minimum SLC End Postition -----
			if(MinBlockIndex > BlockIndex)
				MinBlockIndex = BlockIndex;
		}
	}

	if(pTableCfg->InternalChip == 1)
		goto SkipSearchLun2;

	// ---------- Test SLC Block For Lun 1 ----------
	for(CE=0; CE<pUFDBlockMap->ChipSelectNum; CE++)
	{
		for(CH=0; CH<pUFDBlockMap->ChannelNum; CH++)
		{

			ChipAddress = (CE<<28)|(CH<<24);
			pBlockRecTable->ItemNum = 0;
			for(BlockIndex=Lun2Start; BlockIndex<BlockEnd; BlockIndex+=Plane)
			{

				VB_Pair = 0;
					// ----- Test Plane_0 -----
					Address = ChipAddress|(BlockIndex*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT0;

					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

					// ----- Test Plane_1 -----
					Address = ChipAddress|((BlockIndex+1)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+1, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT1;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue);	OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}
				if(Plane == 4)
				{
					// ----- Test Plane_2 -----
					Address = ChipAddress|((BlockIndex+2)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+2, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT2;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue);	OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}

					// ----- Test Plane_3 -----
					Address = ChipAddress|((BlockIndex+3)*m_BlockPage);
					if(BlockECCTest(CE, CH, 0, BlockIndex+3, pTableCfg, &ECCValue))
					{
						VB_Pair |= BIT3;
						//DbgStr.Format(_T("SLC OK, Path%d, Address = 0x%08X, ECC = 0x%02X"), DiskPathIndex, Address, ECCValue); OutputDebugString(DbgStr);
					}
					else
						MarkBadCnt++;

					if(ECCValue == 0xCC)
					{
						pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
					}

					if(ECCValue == 0xFF)
					{
						if(++CmdErrorCnt > RetryMax)
							goto EndOfTLCSorting;
					}
				}

				if(VB_Pair==VB_Chk)
				{
					pBlockRecTable->ItemNum++;
					Address = ChipAddress|(BlockIndex*m_BlockPage);
				}

				// --- Get Good Block Address for 3 SLC VB ---
				if((VB_Pair==VB_Chk) && (VB_n<3))
				{
					VB_Idx = VB_m + VB_n * VB_Block;
					VB_LFT[VB_Idx] = BlockIndex;
					VB_n++;
					VB_SLC++;
				}

				if(pBlockRecTable->ItemNum >= ItemCount)
					break;

			}
			VB_m++;
			VB_n = 0;
			SLCEnd[CE][CH][1] = BlockIndex;		//Save SLC End

			// ----- Save Minimum SLC End Postition -----
			if(MinBlockIndex > (BlockIndex-Lun2Start))
				MinBlockIndex = (BlockIndex-Lun2Start);
		}
	}

SkipSearchLun2:
	if(VB_SLC<3*VB_Block)
	{
		Status = false;
		goto EndOfTLCSorting;
	}

	// ---------- Save 3 SLC VB Address into Register 0x6900 ----------
	for(VB_Idx = 0; VB_Idx< 3*VB_Block; VB_Idx++)
	{	// VB_Idx as Count
		VB_Chk = HIBYTE(VB_LFT[VB_Idx]);
		Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x6900+2*VB_Idx, 1, &VB_Chk); // Write Reg[6900]
		VB_Chk = LOBYTE(VB_LFT[VB_Idx]);
		Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x6901+2*VB_Idx, 1, &VB_Chk); // Write Reg[6901]
	}
	Status = pmflash->SetThreeSLCVB(MI_SORT_Set3SLCVB);//, CEIndex, CHIndex, BlockAddr, 0, 1, pRxBuf);

	// ---------- Copy SLC To TLC (Write) ----------
	MinBlockIndex = MinBlockIndex + Plane;
	pBlockRecTable->ItemNum = 0;

	for(BlockIndex = MinBlockIndex; BlockIndex < Lun1End; BlockIndex+=Plane)
	{
		Retry = 0;
ReDo_CopySLCtoTLC:
		Status = pmflash->CopySLCtoTLC(MI_SORT_COPYTLC, 0, 0, BlockIndex, 0, Lun2Start);//, CEIndex, CHIndex, BlockAddr, 0, 1, pRxBuf);
		if(!Status)
		{
			usleep(20000);

			Retry++;
			if(Retry < RetryMax)		goto ReDo_CopySLCtoTLC;
			else						goto EndOfTLCSorting;
		}

		if(Debug_Mode == 1)
		{	// TLC Write Stop After 20 Blocks
			if(pBlockRecTable->ItemNum++ > 20)
				break;
		}
	}

	if(Debug_Mode == 1)
		ItemCount = 0x0100;

	// ---------- Read All Block ECC & Erase (By Single Block) ----------
	for(CE=0; CE<pUFDBlockMap->ChipSelectNum; CE++)
	{
		for(CH=0; CH<pUFDBlockMap->ChannelNum; CH++)
		{
			ChipAddress = (CE<<28)|(CH<<24);
			for(BlockIndex=MinBlockIndex; BlockIndex<BlockEnd; BlockIndex++) // Lun 0 SLC Area, Skip Read
			{
				if(Debug_Mode == 1)
				{
					if (BlockIndex>ItemCount)
						break;
				}
				if((pTableCfg->InternalChip==2) && (pTableCfg->ExtendedBlock!=0))
				{
					if((Lun1End <= BlockIndex) && (BlockIndex < Lun2Start)) // Non-Exist Block, Skip Read
						continue;
				}
				if(pTableCfg->InternalChip==2)
				{
					if((Lun2Start<= BlockIndex) && (BlockIndex < (Lun2Start+MinBlockIndex))) // Lun 1 SLC Area, Skip Read
						continue;
				}

				if(BlockIndex < (SLCEnd[CE][CH][0]+Plane))
					continue;
				else if((BlockIndex > Lun2Start) && (BlockIndex < (SLCEnd[CE][CH][1]+Plane)) && (pTableCfg->InternalChip==2))
					continue;

				Address = ChipAddress|(BlockIndex*m_BlockPage);
				if(BlockECCRead(CE, CH, 1, BlockIndex, pTableCfg, &ECCValue))
				{

				}
				else
					MarkBadCnt++;

				if(ECCValue == 0xCC)
				{
					pmroottable->setMapEntryItem(CE, CH, (BlockIndex/8), (BYTE)(BlockIndex%8), 1);
				}

				if(ECCValue == 0xFF)
				{
					if(++CmdErrorCnt > RetryMax)
						goto EndOfTLCSorting;
				}
			}
		}
	}


EndOfTLCSorting:
	pUFDBlockMap->BadBlockCnt[0] = MarkBadCnt; // Sent Total Mark Bad Count To UI
	return Status;
}

UINT CCIS::PreMPTestFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount)
{ // Only Support 2_Plane. Only Use in PreMPTestTool. ReWrite From Find Pair Block From Table
	int i, j, k, l, MarkBadCnt=0;
	BYTE Value, ValueTmp;
	BOOL	Status=false;

	int MaxFindEntry=pTableCfg->MaxPageFind/m_BlockPage/8; //max rage entry to find

	if(MaxFindEntry > (0x10000/m_BlockPage/8) )
		MaxFindEntry = 0x10000/m_BlockPage/8;	// Sherlock_20110921, FW Only Seek 0x10000


	for(i=0; i<pUFDBlockMap->ChipSelectNum; i++)
	{
		for(j=0; j<pUFDBlockMap->ChannelNum; j++)
		{
			pBlockRecTable->ItemNum = 0; // Check Every CH and Every CE
			for(k=0; k<MaxFindEntry; k++)
			{
				Value=pmroottable->GetUFDBlockMapByByte(i, j, k);
				for(l=0;l<8;l+=2)
				{
					ValueTmp=Value>>l;
					if((ValueTmp & 0x03)==0x00)
					{
						// ----- Test Plane_0 -----
						if(BlockStressTest(i, j, k, l, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l;
							pBlockRecTable->ItemNum++;
						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							goto EndOfPreMPTest; // Send Info To UI

						if(pBlockRecTable->ItemNum == ItemCount)
							break; //Quit From "for(l=0;l<8;l+=2)", Not return true;

						// ----- Skip Odd Plane for ED3 Type -----
						if((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) // Sherlock_20110928
							continue;	// For ED3, Can't use Odd Plane For All Table, So Skip Following Part

						// ----- Test Plane_1 -----
						if(BlockStressTest(i, j, k, l+1, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l+1;
							pBlockRecTable->ItemNum++;
						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l+1, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							goto EndOfPreMPTest; // Send Info To UI

						if(pBlockRecTable->ItemNum == ItemCount)
							break; //Quit From "for(l=0;l<8;l+=2)", Not return true;

					}

				}
				if(pBlockRecTable->ItemNum == ItemCount)
					break; // Quit From "for(k=0; k<MaxFindEntry; k++)", Not return true;

			}

		}
	}

EndOfPreMPTest:
	if((i==pUFDBlockMap->ChipSelectNum) && (j==pUFDBlockMap->ChannelNum) && (MarkBadCnt<=10))	return true;
	else // PreMP Test Fail, Send Fail Information To UI
	{
		pTableCfg->MaxFreeBlock = i; // CE
		pTableCfg->MaxLogBlock = j; // CH
	}

	return Status;
}

UINT CCIS::FindQuadriBlockFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount)
{	// Sherlock_20120307, Only Support 4_Plane.
	int		i, j, k, l, MarkBadCnt=0;
	BYTE	Value, ValueTmp;
	BOOL	Status=false;
	int MaxFindEntry=pTableCfg->MaxPageFind/m_BlockPage/8; //max rage entry to find
// Sherlock_20140421, For System Block, It will be rewrite by EncryptionMode
	ULONG	ChipAddr, Address;
	UINT	Original_EraseCnt;
	BYTE	EncryptionCMD = 0;
	if(	((pTableCfg->BaseFType & FT_Vendor) == FT_Samsung) ||
		((pTableCfg->BaseFType & FT_Vendor) == FT_Toshiba) ||
		((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)	)
		EncryptionCMD = BIT7;


	if(MaxFindEntry > (0x10000/m_BlockPage/8) )
		MaxFindEntry = 0x10000/m_BlockPage/8;	// Sherlock_20110921, FW Only Seek 0x10000


	for(i=0; i<=((pTableCfg->MaxChipFind-1)/pUFDBlockMap->ChannelNum); i++)
	{
		for(j=0; j<=((pTableCfg->MaxChipFind-1)%pUFDBlockMap->ChannelNum); j++)
		{
			for(k=0; k<MaxFindEntry; k++)
			{
				Value=pmroottable->GetUFDBlockMapByByte(i, j, k);

				for(l=0;l<8;l+=4)
				{

					ValueTmp=Value>>l;
					if((ValueTmp & 0x0F)==0x00)
					{
						// ----- Test Plane_0 -----

						ChipAddr = (i<<4) | j;
						Address = (ChipAddr << 24) |((k*8 + l) * m_BlockPage);
						Original_EraseCnt = GetAndCheckEraseCount(EncryptionCMD, Address);
						pTableCfg->ED3EraseCount = Original_EraseCnt; // Only For StressTest Use

						if(BlockStressTest(i, j, k, l, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt + 2;
							pBlockRecTable->ItemNum++;

						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							return false;

						if(pBlockRecTable->ItemNum == ItemCount)
							return true;

						// ----- Test Plane_2 -----

						ChipAddr = (i<<4) | j;
						Address = (ChipAddr << 24) |((k*8 + l+2) * m_BlockPage);
						Original_EraseCnt = GetAndCheckEraseCount(EncryptionCMD, Address);
						pTableCfg->ED3EraseCount = Original_EraseCnt; // Only For StressTest Use

						if(BlockStressTest(i, j, k, l+2, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l+2;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt + 2;
							pBlockRecTable->ItemNum++;

						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l+2, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							return false;

						if(pBlockRecTable->ItemNum == ItemCount)
							return true;

						// ----- Skip Odd Plane for ED3 Type -----
						if((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) // Sherlock_20110928
							continue;	// For ED3, Can't use Odd Plane For All Table, So Skip Following Part

						// ----- Test Plane_1 -----

						ChipAddr = (i<<4) | j;
						Address = (ChipAddr << 24) |((k*8 + l+1) * m_BlockPage);
						Original_EraseCnt = GetAndCheckEraseCount(EncryptionCMD, Address);
						pTableCfg->ED3EraseCount = Original_EraseCnt; // Only For StressTest Use

						if(BlockStressTest(i, j, k, l+1, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l+1;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt + 2;
							pBlockRecTable->ItemNum++;
						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l+1, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							return false;

						if(pBlockRecTable->ItemNum == ItemCount)
							return true;

						// ----- Test Plane_3 -----

						ChipAddr = (i<<4) | j;
						Address = (ChipAddr << 24) |((k*8 + l+3) * m_BlockPage);
						Original_EraseCnt = GetAndCheckEraseCount(EncryptionCMD, Address);
						pTableCfg->ED3EraseCount = Original_EraseCnt; // Only For StressTest Use

						if(BlockStressTest(i, j, k, l+3, pTableCfg))
						{
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l+3;
							pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt + 2;
							pBlockRecTable->ItemNum++;
						}
						else
							MarkBadCnt++;

						pmroottable->setMapEntryItem(i, j, k, l+3, 1); //set as bad block beacuse be use as CIS or ISP block
						pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

						if(MarkBadCnt>10)
							return false;

						if(pBlockRecTable->ItemNum == ItemCount)
							return true;

					}

				}

			}

		}
	}

	return Status;
}

UINT CCIS::FindPairBlockFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount)
{	// Only Support 2_Plane.
	int 		i, j, k, l, MarkBadCnt=0;
		BYTE 	Value, ValueTmp;
		BOOL	Status=false, Blk_0=0, Blk_1=0;
		int MaxFindEntry=pTableCfg->MaxPageFind/m_BlockPage/8; //max rage entry to find
	 // Sherlock_20140421, For System Block, It will be rewrite by EncryptionMode
		ULONG	ChipAddr, Address=0, Address_0=0, Address_1=0;
		UINT	Original_EraseCnt=0, Original_EraseCnt_0=0, Original_EraseCnt_1=0;
		BYTE	EncryptionCMD = 0;

		BYTE	*pTxBuf;
		ULONG	TxLen;
		TxLen=((ULONG)pTableCfg->PageSEC)*512;
		pTxBuf=new BYTE[TxLen];
		memset(pTxBuf, 0xFF, TxLen);
		SPARETYPE Spare;
		Spare.SPARE0=0x4D; //'M'
		Spare.SPARE1=0x50; //'P'
		Spare.SPARE2=0x00;
		Spare.SPARE3=0x00;
		Spare.SPARE4=0x99;
		Spare.SPARE5=0x99;

		if(	((pTableCfg->BaseFType & FT_Vendor) == FT_Samsung) ||
			((pTableCfg->BaseFType & FT_Vendor) == FT_Toshiba) ||
			((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)	)
			EncryptionCMD = BIT7;


		if(MaxFindEntry > (0x40000/m_BlockPage/8) )
			MaxFindEntry = 0x40000/m_BlockPage/8;	// Sherlock_20110921, FW Only Seek 0x10000


		for(i=0; i<=((pTableCfg->MaxChipFind-1)/pUFDBlockMap->ChannelNum); i++)
		{
			for(j=0; j<=((pTableCfg->MaxChipFind-1)%pUFDBlockMap->ChannelNum); j++)
			{

				for(k=0; k<MaxFindEntry; k++)
				{
					Value=pmroottable->GetUFDBlockMapByByte(i, j, k);

					for(l=0;l<8;l+=2)
					{

						ValueTmp=Value>>l;
						////////////////////////
						if((ValueTmp & 0x03)==0x00)
						{
							// ----- Test Plane_0 -----

							ChipAddr = (i<<4) | j;
							Address_0 = (ChipAddr << 24) |((k*8 + l) * m_BlockPage);
							Original_EraseCnt_0 = GetAndCheckEraseCount(EncryptionCMD, Address_0);
							pTableCfg->ED3EraseCount = Original_EraseCnt_0; // Only For StressTest Use

							Blk_0 = BlockStressTest(i, j, k, l, pTableCfg);

							// ----- Test Plane_1 -----

							ChipAddr = (i<<4) | j;
							Address_1 = (ChipAddr << 24) | ((k*8 + l+1) * m_BlockPage);
							Original_EraseCnt_1 = GetAndCheckEraseCount(EncryptionCMD, Address_1);
							pTableCfg->ED3EraseCount = Original_EraseCnt_1; // Only For StressTest Use

							Blk_1 = BlockStressTest(i, j, k, l+1, pTableCfg);

							if((Blk_0) && (Blk_1)) //(BlockStressTest(pHandleList, DiskPathIndex, i, j, k, l, pTableCfg))
							{
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l;

								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt_0 + 2;
								pBlockRecTable->ItemNum++;
							}
							else if((Blk_0) && (!Blk_1)) // Special WriteBack_EraseCount
							{
								Spare.SPARE3=(BYTE)((Original_EraseCnt_0 + 2)>>8); // HI
								Spare.SPARE2=(BYTE)(Original_EraseCnt_0 + 2);		// LO
								Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, pTableCfg->PageSEC, pTableCfg->ECCSET, Spare, 0, 0, Address_0, TxLen, pTxBuf);
							}
							else // (!Blk_0)
								MarkBadCnt++;

							pmroottable->setMapEntryItem(i, j, k, l, 1); //set as bad block beacuse be use as CIS or ISP block
							pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;



							if(pBlockRecTable->ItemNum == ItemCount)
							{
								delete pTxBuf;
								return true;
							}

							if((Blk_1) && (Blk_0)) //(BlockStressTest(pHandleList, DiskPathIndex, i, j, k, l+1, pTableCfg))
							{
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CEIndex=i;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].CHIndex=j;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EntryIndex=k;
								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].BlockIndex=l+1;

								pBlockRecTable->pBlockRec[pBlockRecTable->ItemNum].EraseCount=Original_EraseCnt_1 + 2;
								pBlockRecTable->ItemNum++;

							}
							else if((Blk_1) && (!Blk_0)) // Special WriteBack EraseCount
							{
								Spare.SPARE3=(BYTE)((Original_EraseCnt_1+ 2)>>8); // HI
								Spare.SPARE2=(BYTE)(Original_EraseCnt_1 + 2);		// LO
								Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, pTableCfg->PageSEC, pTableCfg->ECCSET, Spare, 0, 0, Address_1, TxLen, pTxBuf);

							}
							else // (!Blk_1)
								MarkBadCnt++;

							pmroottable->setMapEntryItem(i, j, k, l+1, 1); //set as bad block beacuse be use as CIS or ISP block
							pUFDBlockMap->BadBlockCnt[(i*pUFDBlockMap->ChipSelectNum)+j]++;

							if(MarkBadCnt>10)
							{

								delete pTxBuf;
								return false;
							}

							if(pBlockRecTable->ItemNum == ItemCount)
							{
								delete pTxBuf;
								return true;
							}

						}
					}

				}

			}
		}

		delete pTxBuf;
		return Status;
}

UINT CCIS::BlockStressTest(BYTE CEIndex, BYTE CHIndex, UINT EntryIndex, BYTE BlockIndex, LPTableCFG pTableCfg)
{
	BOOL	 Sts;
	ULONG	Address, ChipAddress, TxLen, Offset, Index, MaxTransfer, MaxTxPage=8, PageIndex;
	BYTE	BitErrorCnt = 0, EraseStatus, ValueTmp;
	BYTE	Retry = 0, RetryMax = 10;	// Sherlock_20110502, Add Retry Protection
	BYTE  	*pTxBuf, *pRxBuf;
	SPARETYPE Spare;
	UINT 	Status=1;


	// ----- Mark Bad SLC Page -----
	USHORT	LastPage = pTableCfg->LessPage;
	if((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)		LastPage = LastPage/3;

	if(pTableCfg->PageSEC == 32) // Sherlock_20120207
	{ 	// For 16KPage, PageSEC=32,  Cdb[8] = PageSEC*512*MaxTxPage/512,
		// If MaxTxPage = 8, Cdb[8] = (CHAR)256, Over Flow Error !!
		MaxTxPage = 4;
	}

	Spare.SPARE0=0x4D; //'M'
	Spare.SPARE1=0x50; //'P'
	Spare.SPARE2=0x00;
	Spare.SPARE3=0x00;
	Spare.SPARE4=0x99;
	Spare.SPARE5=0x99;

// Sherlock_20131106, Set EraseCount for RWTest
	Spare.SPARE3=(BYTE)((pTableCfg->ED3EraseCount + 1)>>8);	// HI
	Spare.SPARE2=(BYTE)(pTableCfg->ED3EraseCount + 1);		// LO


	TxLen=((ULONG)m_BlockPage)*((ULONG)pTableCfg->PageSEC)*512;

	MaxTransfer=((ULONG)pTableCfg->PageSEC)*512*MaxTxPage; //transfer size is 8 page
	pTxBuf=new BYTE[TxLen];
	pRxBuf=new BYTE[MaxTransfer];

	for(Index=0; Index<TxLen;Index++)
		pTxBuf[Index]=rand();

	ChipAddress=((ULONG)CEIndex<<4)|CHIndex;
	Address=(ChipAddress<<24)|((EntryIndex*8+BlockIndex)*m_BlockPage);

	// Sherlock_20140815, Set Target Block As MLC & Default_ECC
	pmroottable->setCellMap(Address,pmflash, 1);
	pmroottable->setEccMap(Address, pmflash, 0);
Retry_BST_Erase:
	Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);
	if(!Status)
	{
		usleep(20);	// Sherlock_20121130, Test
		Retry++;
		if(Retry < RetryMax)		goto Retry_BST_Erase;
		else						goto EndBlockStressTest;
	}

	for(Offset=0; Offset<TxLen; Offset+=MaxTransfer)
	{
		PageIndex=(Offset/MaxTransfer)*MaxTxPage;

		if((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Samsung|FT_TLC))
		{	// For Samsung TLC, Flash Only Has 64 WL Page (Aligned/UnAligned, 2Plane/4Pplane)
			if(PageIndex>=((pTableCfg->LessPage/3)-MaxTxPage+1)) // 192/3 -8 + 1 = 57
				goto EndBlockStressTest;
		}

		if((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) // Sherlock_20110928
		{	// For Toshiba TLC, Flash Only Has 86 WL Page (Aligned/UnAligned, 2Plane/4Pplane)
			if(PageIndex>=((pTableCfg->LessPage/3)-MaxTxPage+1))  // 258/3 -8 + 1 = 79
				goto EndBlockStressTest;
		}

		Retry = 0;
Retry_BST_Write:
		Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, pTableCfg->PageSEC, pTableCfg->ECCSET, Spare, 0, 0, Address+PageIndex, MaxTransfer, pTxBuf+Offset);
		if(!Status)
		{
			usleep(20);	// Sherlock_20121130, Test
			Retry++;
			if(Retry < RetryMax)		goto Retry_BST_Write;
			else						goto EndBlockStressTest;
		}

		Retry = 0;
Retry_BST_Read:
		Status=pmflash->BlockAccessRead(MI_BLOCK_READ, pTableCfg->PageSEC, pTableCfg->ECCSET, 0, 0, m_BlockPage, Address+PageIndex, MaxTransfer, pRxBuf);
		if(!Status)
		{
			usleep(20000);	// Sherlock_20121130, Test
			Retry++;
			if(Retry < RetryMax)		goto Retry_BST_Read;
			else						goto EndBlockStressTest;
		}
		for(Index=0;Index< MaxTransfer; Index++)
		{
			if((Index%1024)==0)
				BitErrorCnt=0;

			ValueTmp=pTxBuf[Offset+Index]^pRxBuf[Index];

			if(ValueTmp)
			{
				//DebugStr.Format(_T("1(%d)=%x,%d"), DiskPathIndex, ValueTmp, BitErrorCnt);	OutputDebugString(DebugStr);
				BitErrorCnt+=BitCount(ValueTmp);
				//DebugStr.Format(_T("2(%d)=%d"), DiskPathIndex, BitErrorCnt);					OutputDebugString(DebugStr);
			}

			if(BitErrorCnt > pTableCfg->ErrorBitRate)
			{
				Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus); // Sherlock_20121005, Add Erase Before MarkBad

				if(pTableCfg->Type & MustMarkBad_TYPE)
				{
// Move To Upper					// ----- Mark Bad SLC Page -----
//					USHORT	LastPage = pTableCfg->LessPage;
//					if((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)		LastPage = LastPage/3;

					Status=pmflash->BlockMarkWrite(MI_MARK_BAD, 0, 0, Address, 0, pTableCfg->PageSEC, HIBYTE(LastPage), LOBYTE(LastPage), NULL); //mark bad

				}
				Status=false;
				goto EndBlockStressTest_NoMarkBad;// EndBlockStressTest;
			}
		}
	}

EndBlockStressTest:
	if(Status)
		Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);

EndBlockStressTest_NoMarkBad: // Sherlock_20130301, ErrorBit Over, It Had Already Been Mark Bad

	// ----- Add Debug Message, Sherlock_20121107 -----
	// Move To Upper	BOOL	Sts;
	if(!Status)
	{
		// Check Core Power
		Sts = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, 0x1FF82608, 1, &ValueTmp); // Read Reg[2608]
	}

	delete pRxBuf;
	delete pTxBuf;
	return Status;
}

UINT CCIS::GetAndCheckEraseCount(BYTE EncryptionCmd, ULONG Address)
{	// Sherlock_20131106
	BOOL	Status;
	BYTE	SpareBuf[6] = {0,0,0,0,0,0};
	UINT	EraseCount;
// Sherlock_20140429, Use Sector Mode Read
	UINT	RdSize=1024;
	SPARETYPE Spare;
	BYTE	*RxDataBuf, Register = BIT1|BIT0;
	RxDataBuf=(BYTE *)malloc(sizeof(BYTE)*RdSize);
	Spare.SPARE0=0x00;
	Spare.SPARE1=0x00;
	Spare.SPARE2=0x00;
	Spare.SPARE3=0x00;
	Spare.SPARE4=0x00;
	Spare.SPARE5=0x00;

	Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x1FF8300D, 1, &Register);
	Status = pmflash->BlockAccessRead(	MI_BLOCK_READ,
										(BYTE)(RdSize/512),
										EncryptionCmd, //Encryption_Set + ECC_ON + ECC_Set_40Bit
										0,
										0,
										m_BlockPage,
										Address,
										RdSize,
										RxDataBuf);
	Status = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, 0x300D0010, 6, SpareBuf);



	// ---------- New Get Spare ---------- Sherlock_20140415
	USHORT Shift1 = SpareBuf[1]&0x3F;	// Clear Bit7 and Bit6

	if(((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) && ((EncryptionCmd&BIT7) == BIT7))	// 'CS' && Encryption_ON
	{
// Sherlock_20140429, Use Sector Mode Read
		Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x1FF8300D, 1, &Register);
		Status = pmflash->BlockAccessRead(	MI_BLOCK_READ,
											(BYTE)(RdSize/512),
											2, //Encryption_OFF + ECC_ON + ECC_Set_60Bit
											0,
											0,
											m_BlockPage,
											Address,
											RdSize,
											RxDataBuf);
		Status = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, 0x300D0010, 6, SpareBuf);

		if(Status)
			EraseCount = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
		else
			EraseCount = 0;
	}
	else if(	((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) || 	// 'CS'
			((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x52)) ||	// 'RT' = 52 2A
			((Shift1<<8 | SpareBuf[0]) == (0x48<<7 | 0x43)) ||	// 'CH' = 43 24
			((Shift1<<8 | SpareBuf[0]) == (0x4B<<7 | 0x4D)) ||	// 'MK' = CD 25
			((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x44)) ||	// 'DT' = 44 2A
			((Shift1<<8 | SpareBuf[0]) == (0x55<<7 | 0x44)) ||	// 'DU' = C4 2A
			((SpareBuf[0] == 0x4D) && (SpareBuf[1] == 0x50)) )	// 'MP' This Means "Erased By MPTool"
	{
		EraseCount = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
	}
	else
	{
		EraseCount = 0;
	}

 // Sherlock_20140429, Use Sector Mode Read
	free(RxDataBuf);

	return	EraseCount;
}

// TLC Mode: 0 For MLC & SLC, 1 For TLC.
//		0:	Erase -> Write -> Read ECC -> Check ECC -> Erase(Mark Bad)
//		1:	Check ECC
UINT CCIS::BlockECCTest(BYTE CEIndex, BYTE CHIndex, BYTE TLCMode, WORD BlockAddr, LPTableCFG pTableCfg, BYTE *ECCValue)
{	// Re-Write From Block Stress Test
	BOOL	Sts, MarkBadSts=false;
	ULONG	Address, Address0, ChipAddress, TxLen, Offset, Index, MaxTransfer, MaxTxPage=8, PageIndex, SLCPage;
	BYTE	EraseStatus;//BitErrorCnt = 0, , ValueTmp;
	BYTE	Retry = 0, RetryMax = 10, SortingECC;	// Sherlock_20110502, Add Retry Protection
	BYTE  	*pTxBuf, *pRxBuf;
	SPARETYPE Spare;
	BYTE		Register1, Register2, EncryptionSet;
	ULONG	Reg_Encryption = 0x1FF82809;
	UINT	Status=1;

	if(pTableCfg->PageSEC == 32) // Sherlock_20120207
	{ 	// For 16KPage, PageSEC=32,  Cdb[8] = PageSEC*512*MaxTxPage/512,
		// If MaxTxPage = 8, Cdb[8] = (CHAR)256, Over Flow Error !!
		MaxTxPage = 4;
	}

	Spare.SPARE0=0x53; //'S'	// Sherlock_20120927, Fix eD3 Mark Bad 4Plane
	Spare.SPARE1=0x4F; //'O'
	Spare.SPARE2=0x52; //'R'
	Spare.SPARE3=0x54; //'T'
	Spare.SPARE4=0x49; //'I'
	Spare.SPARE5=0x4E; //'N'

	if((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)
		SLCPage = pTableCfg->LessPage/3;
	else
		SLCPage = m_BlockPage;

	TxLen = SLCPage * pTableCfg->PageSEC * 512;
	MaxTransfer=((ULONG)pTableCfg->PageSEC)*512*MaxTxPage; //transfer size is 4 or 8 page
	pTxBuf=new BYTE[TxLen];
	pRxBuf=new BYTE[1];//MaxTransfer

	pRxBuf[0] = *ECCValue = 0xFF; // Set Max as Default Value

	for(Index=0; Index<TxLen;Index++)
		pTxBuf[Index]=rand();

	ChipAddress=(CEIndex<<4)|CHIndex;
	Address=(ChipAddress<<24)|((ULONG)BlockAddr*m_BlockPage);

	// ----- Set ECC Limit -----
	if(TLCMode == 1)
		SortingECC = (BYTE)((pTableCfg->ED3EraseCount & 0x0000FF00)>>8);	// TLC Sorting ECC Limit
	else
		SortingECC = (BYTE)(pTableCfg->ED3EraseCount & 0x000000FF);		// MLC Sorting ECC Limit

	if(TLCMode == 1) // For TLC, Write & Read is Separated, So We Only Read ECC & Check ECC
		goto ReadBlockECCOnly;

	// ----- 1. Erase Block -----
Retry_BET_E:
	Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);
	if(!Status)
	{
		usleep(20000);	// Sherlock_20121130, Test
		Retry++;
		if(Retry < RetryMax)		goto Retry_BET_E;
		else						goto EndBlockECCTest;
	}

	// ----- 2. Write Block -----

	for(PageIndex=0; PageIndex<SLCPage; PageIndex+=MaxTxPage)
	{
		if((SLCPage-PageIndex) < MaxTxPage)
			MaxTransfer = ((ULONG)pTableCfg->PageSEC)*512*(SLCPage - PageIndex);
		else
			MaxTransfer = ((ULONG)pTableCfg->PageSEC)*512*MaxTxPage;

		Offset = ((ULONG)pTableCfg->PageSEC)*512*PageIndex;

		//DbgStr.Format(_T("**Address=0x%x, PageIndex=%d, MaxTransfer =0x%x,Offset=0x%x"),Address, PageIndex, MaxTransfer,Offset); OutputDebugString(DbgStr);

		Retry = 0;
Retry_BET_W:
		Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, pTableCfg->PageSEC, pTableCfg->ECCSET, Spare, 0, 0, Address+PageIndex, MaxTransfer, pTxBuf+Offset);

		if(!Status)
		{
			usleep(20000);	// Sherlock_20121130, Test
			Retry++;
			if(Retry < RetryMax)		goto Retry_BET_W;
			else						goto EndBlockECCTest;
		}
	}



ReadBlockECCOnly:
	// ----- 3. Read ECC -----
//	WORD BlockAddr = EntryIndex*8+BlockIndex;

	Retry=0;
Retry_BC_ECC:
	Status = pmflash->BlockCheckECC(MI_SORT_READECC, CEIndex, CHIndex, BlockAddr, TLCMode, 1, pRxBuf);
	if(!Status)
	{
		pRxBuf[0] = 0xFF;
		usleep(20000);
		Retry++;
		if(Retry < RetryMax)
			goto Retry_BC_ECC;
		else
			goto EndBlockECCTest;
	}
	*ECCValue = pRxBuf[0];

	// ----- 4. Check ECC -----
	if(pRxBuf[0] > SortingECC) // pTableCfg->ErrorBitRate
	{
		if(TLCMode == 0)
			Sts=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus); // Sherlock_20121005, Add Erase Before MarkBad

		if(pTableCfg->Type & MustMarkBad_TYPE)
		{

			// ----- Mark Bad SLC Page -----
			USHORT	LastPage = pTableCfg->LessPage;
			if((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)		LastPage = LastPage/3;

			Retry = 0;
Retry_MarkBad:
			Sts=pmflash->BlockMarkWrite(MI_MARK_BAD, 0, 0, Address, 0, pTableCfg->PageSEC, HIBYTE(LastPage), LOBYTE(LastPage), NULL); //mark bad

			if(((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) && (BlockAddr%2))
			{	// If it's eD3 Odd Block, Mark Bad Even Block
				Address0 = (CEIndex<<28)|(CHIndex<<24)|((BlockAddr-1)*m_BlockPage); // Get Row Address

				Sts = pmflash->BlockMarkWrite(MI_MARK_BAD, 0, 0, Address0, 0, pTableCfg->PageSEC, HIBYTE(LastPage), LOBYTE(LastPage), NULL); //mark bad
			}

			// Check Spare Area to Make Sure Mark Bad
			Retry = 0;
Retry_SAR1:
			pRxBuf[4] = pRxBuf[5] = 0xFF;
			Sts = pmflash->SpareAccessRead(MI_SPARE_READ, 0, 0, 0, 0, m_BlockPage, Address, 6, pRxBuf);
			if(!Sts)
			{
				usleep(20000);
				Retry++;
				if(Retry < RetryMax)		goto Retry_SAR1;
			}

			// ----- 20130410_New Rule-----
			if((pRxBuf[4]==0x00) && (pRxBuf[5]==0x00)) // Mark Bad OK
			{
				MarkBadSts = true;
			}
			else if((pRxBuf[4]==0xFF) || (pRxBuf[5]==0xFF)) // Mark Bad Error
			{
				MarkBadSts = false;
			}
			else
			{
				Retry = 0;
Retry_Read_Entryption_Reg:
				Register1 = 0xFF;
				Sts = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, Reg_Encryption, 1, &Register1); // Read Reg[2809]
				if((!Sts) || (Register1 == 0xFF))
				{	// Read Error Or Value Did Not Be Change
					Retry++;
					if(Retry<RetryMax) 	goto Retry_Read_Entryption_Reg;
				}

				EncryptionSet = Register1 & 0x01;
				if (EncryptionSet == BIT0)				// Encryption Turn ON
					Register2 = Register1 & (~BIT0); 	// Encryption OFF
				else
					Register2 = Register1 | BIT0;  	// Encryption ON

				Retry = 0;
Retry_Write_Encryption_Reg:
				Sts = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_Encryption, 1, &Register2); // Write Reg[2809]
				if(!Sts)
				{	// Read Error Or Value Did Not Be Change
					Retry++;
					if(Retry<RetryMax) 	goto Retry_Write_Encryption_Reg;
				}

				// Check Spare Area to Make Sure Mark Bad
				Retry = 0;
Retry_SAR2:
				pRxBuf[4] = pRxBuf[5] = 0xFF;
				Sts = pmflash->SpareAccessRead(MI_SPARE_READ, 0, 0, 0, 0, m_BlockPage, Address, 6, pRxBuf);
				if(!Sts)
				{
					usleep(20000);
					Retry++;
					if(Retry < RetryMax)		goto Retry_SAR2;
				}

				if( (pRxBuf[4]==0xFF) || (pRxBuf[5]==0xFF) ) // Mark Bad Error
				{
					MarkBadSts = false;
				}
				else
				{
					MarkBadSts = true;
				}

				Retry = 0;
Retry_Write_Original_Encryption_Value:
				Sts = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_Encryption, 1, &Register1); // Write Reg[2809]
				if(!Sts)
				{
					Retry++;
					if(Retry<RetryMax)	goto Retry_Write_Original_Encryption_Value;
				}

			}

			if(!MarkBadSts)
			{
				usleep(20000);
				Retry++;
				if(Retry < RetryMax)		goto Retry_MarkBad;
				else						*ECCValue = 0xCC; // Flag to Inform Sorting Process
			}
		}
		else
		{
		}
		Status=false;
		goto EndBlockECCTest;
	}

EndBlockECCTest:
	if((Status) && (TLCMode == 0))
		Sts=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);
	delete pRxBuf;
	delete pTxBuf;
	return Status;
}

UINT CCIS::BlockECCRead(BYTE CEIndex, BYTE CHIndex, BYTE TLCMode, WORD BlockAddr, LPTableCFG pTableCfg, BYTE *ECCValue)
{	// Re-Write From Block Stress Test
	BOOL	Sts, MarkBadSts=false;
	ULONG	Address, Address0;
	ULONG	Reg_Encryption = 0x1FF82809;
	BYTE	EraseStatus;//BitErrorCnt = 0, , ValueTmp;
	BYTE	Retry = 0, RetryMax = 10, SortingECC;	// Sherlock_20110502, Add Retry Protection
	BYTE  	*pRxBuf;
	UINT	Status=1;
	BYTE	Register1, Register2, EncryptionSet;

	pRxBuf = new BYTE[6]; // For Spare
	pRxBuf[0] = *ECCValue = 0xFF; // Set Max as Default Value

	Address = (CEIndex<<28)|(CHIndex<<24)|(BlockAddr*m_BlockPage); // Get Row Address

	if(TLCMode == 1)
		SortingECC = (BYTE)((pTableCfg->ED3EraseCount & 0x0000FF00)>>8);	// Set TLC Sorting ECC Limit
	else
		SortingECC = (BYTE)(pTableCfg->ED3EraseCount & 0x000000FF);		// Set MLC Sorting ECC Limit

	// ----- Read ECC -----
	Retry=0;
Retry_BER_ECC:
	Status = pmflash->BlockCheckECC(MI_SORT_READECC, CEIndex, CHIndex, BlockAddr, TLCMode, 1, pRxBuf);
	if(!Status)
	{
		pRxBuf[0] = 0xFF;
		usleep(20000);
		Retry++;

		if(Retry < RetryMax)		goto Retry_BER_ECC;
	}
	*ECCValue = pRxBuf[0];


	// ----- Check ECC -----
	if(pRxBuf[0] > SortingECC) // pTableCfg->ErrorBitRate
	{
		Sts = pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus); // Sherlock_20121005, Add Erase Before MarkBad

		if(pTableCfg->Type & MustMarkBad_TYPE)
		{
			// ----- Mark Bad SLC Page -----
			USHORT	LastPage = pTableCfg->LessPage;
			if((pTableCfg->BaseFType & FT_Cell_Level) == FT_TLC)		LastPage = LastPage/3;

			Retry = 0;
Retry_BER_Mark:	//Retry if Mark Bad Fail
			Sts = pmflash->BlockMarkWrite(MI_MARK_BAD, 0, 0, Address, 0, pTableCfg->PageSEC, HIBYTE(LastPage), LOBYTE(LastPage), NULL); //mark bad

			if(((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) && (BlockAddr%2))
			{	// If it's eD3 Odd Block, Mark Bad Even Block
				Address0 = (CEIndex<<28)|(CHIndex<<24)|((BlockAddr-1)*m_BlockPage); // Get Row Address
				Sts = pmflash->BlockMarkWrite(MI_MARK_BAD, 0, 0, Address0, 0, pTableCfg->PageSEC, HIBYTE(LastPage), LOBYTE(LastPage), NULL); //mark bad
			}

			// Check Spare Area to Make Sure Mark Bad
			Retry = 0;
Retry_Read_Spare1:
			pRxBuf[4] = pRxBuf[5] = 0xFF;
			Sts = pmflash->SpareAccessRead(MI_SPARE_READ, 0, 0, 0, 0, m_BlockPage, Address, 6, pRxBuf);
			if(!Sts)
			{
				usleep(20000);
				Retry++;
				if(Retry < RetryMax)		goto Retry_Read_Spare1;
			}

			// ----- 20130410_New Rule-----
			if((pRxBuf[4]==0x00) && (pRxBuf[5]==0x00)) // Mark Bad OK
			{
				MarkBadSts = true;
			}
			else if((pRxBuf[4]==0xFF) || (pRxBuf[5]==0xFF)) // Mark Bad Error
			{
				MarkBadSts = false;
			}
			else
			{
				Retry = 0;
Retry_Read_Encryption_Register:
				Register1 = 0xFF;
				Sts = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, Reg_Encryption, 1, &Register1); // Read Reg[2809]
				if((!Status) || (Register1 == 0xFF))
				{	// Read Error Or Value Did Not Be Change
					Retry++;
					if(Retry<RetryMax) 	goto Retry_Read_Encryption_Register;
				}

				EncryptionSet = Register1 & 0x01;
				if (EncryptionSet == BIT0)				// Encryption Turn ON
					Register2 = Register1 & (~BIT0); 	// Encryption OFF
				else
					Register2 = Register1 | BIT0;  	// Encryption ON

				Retry = 0;
Retry_Write_Encryption_Register:
				Sts = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_Encryption, 1, &Register2); // Write Reg[2809]
				if(!Status)
				{	// Read Error Or Value Did Not Be Change
					Retry++;
					if(Retry<RetryMax) 	goto Retry_Write_Encryption_Register;
				}

				// Check Spare Area to Make Sure Mark Bad
				Retry = 0;
Retry_Read_Spare2:
				pRxBuf[4] = pRxBuf[5] = 0xFF;
				Sts = pmflash->SpareAccessRead(MI_SPARE_READ, 0, 0, 0, 0, m_BlockPage, Address, 6, pRxBuf);
				if(!Sts)
				{
					usleep(20);
					Retry++;
					if(Retry < RetryMax)		goto Retry_Read_Spare2;
				}

				if( (pRxBuf[4]==0xFF) || (pRxBuf[5]==0xFF) ) // Mark Bad Error
				{
					MarkBadSts = false;
				}
				else
				{
					MarkBadSts = true;
				}

				Retry = 0;
Retry_Write_Original_Encryption:
				Sts = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_Encryption, 1, &Register1); // Write Reg[2809]
				if(!Sts)
				{
					Retry++;
					if(Retry<RetryMax) 	goto Retry_Write_Original_Encryption;
				}
			}

			if(!MarkBadSts)
			{
				usleep(20);
				Retry++;
				if(Retry < RetryMax)		goto Retry_BER_Mark;
				else						*ECCValue = 0xCC; // Flag to Inform Sorting Process
			}
		}
		else
		{
		}
		Status = false; // Always false if Block is bad
		goto EndBlockECCRd;
	}

EndBlockECCRd:
	if((Status) && ((pTableCfg->BaseFType & (FT_Vendor|FT_Cell_Level)) != (FT_Toshiba|FT_TLC)))
	{
		Retry = 0;
ReDo_EraseBlock1:
		Sts=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &EraseStatus);
		if(!Sts)
		{
			usleep(20000);
			Retry++;
			if(Retry < RetryMax)		goto ReDo_EraseBlock1;
		}
	}
	delete pRxBuf;
	return Status;
}
