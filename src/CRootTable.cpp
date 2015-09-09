/*
 * CRootTable.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CRootTable.h"


CRootTable::CRootTable() {
	mRTInfoINITFlag = Fail_State;
	pmRTInfo = new RTblInfo;
	pmRootTable =new ROOT_VARS[4];
	pmflash = NULL;
	pmUFDBlockMap = new MapChipSelect();

}
CRootTable::CRootTable(CFlash *flash) {
	// TODO Auto-generated constructor stub
	mRTInfoINITFlag = Fail_State;
	pmflash= flash;

	pmUFDBlockMap = AllocateBlockMapMemory();
	pmRTInfo = new RTblInfo();
	pmRootTable =new ROOT_VARS[4];

	memset(pmRTInfo, 0xFF, sizeof(RTblInfo));
	pmRTInfo->Exist = (BYTE)false;

	pmUFDBlockMap->ChipSelectNum = pmflash->getChipSelectNum();
	pmUFDBlockMap->EntryItemNum = pmflash->getEntryItemNum();
	pmUFDBlockMap->ChannelNum = pmflash->getChannelNum();

	if(pmflash->isOldVersionCISExit()){
		mRTInfoINITFlag = initRTInfoTable();
		if(mRTInfoINITFlag == Success_State){
			setCellMap(pmRTInfo->RAWAddr0,pmflash,pmRTInfo->CellType);
			setEccMap(pmRTInfo->RAWAddr0,pmflash,pmRTInfo->CellType);
			initRootTable();
		}
	}

}

CRootTable::~CRootTable() {
	// TODO Auto-generated destructor stub
}

void CRootTable::setFlash(CFlash *flash) {

	pmflash=flash;
}

UINT CRootTable::setCellMap(ULONG Address, CFlash *pmflash, BYTE MLC) {
	// TODO Auto-generated constructor stub
		UINT	PU_Idx, Byte_Idx, bit_Idx;
		BYTE	ValueTmp;

		PU_Idx = (Address & 0xF0000000) >> 28;					// eMMC Only 1 CH
		Address = (Address & 0x00FFFFFF)/pmflash->getBlockPage()/pmflash->getPlaneNum();	// RAWAdr_BlockAdr_PairAdr
		Byte_Idx = Address / 8;
		bit_Idx	= Address % 8;
		ValueTmp = 0x01<<bit_Idx;

		if(Byte_Idx>262)	return Fail_State;

		if(MLC)
			pmRootTable[PU_Idx].chBlkTagBitMapTbl[Byte_Idx] |= ValueTmp;	// Set Cell_Map[] as MLC
		else
			pmRootTable[PU_Idx].chBlkTagBitMapTbl[Byte_Idx] &= (~ValueTmp);	// Set Cell_Map[] as SLC

		return Success_State;
}

UINT CRootTable::setEccMap(ULONG Address, CFlash *pmflash, BYTE MaxECC) {
	// TODO Auto-generated constructor stub
		UINT	PU_Idx, Byte_Idx, bit_Idx;
		BYTE	ValueTmp;

		PU_Idx = (Address & 0xF0000000) >> 28;					// eMMC Only 1 CH
		Address = (Address & 0x00FFFFFF)/pmflash->getBlockPage()/pmflash->getPlaneNum();	// RAWAdr_BlockAdr_PairAdr
		Byte_Idx = Address / 8;
		bit_Idx	= Address % 8;
		ValueTmp = 0x01<<bit_Idx;

		if(Byte_Idx>262)	return Fail_State;

		if(MaxECC)
			pmRootTable[PU_Idx].chBlkECCBitMapTbl[Byte_Idx] |= ValueTmp;	// Set ECC_Map[] as 60-bits
		else
			pmRootTable[PU_Idx].chBlkECCBitMapTbl[Byte_Idx] &= (~ValueTmp);	// Set ECC_Map[] as Default


		return Success_State;
}


UINT CRootTable::setSystemBlock(CFlash *pflash) {
	// TODO Auto-generated constructor stub
	UINT	Status = Success_State, Idx;

		for(Idx=0; Idx<3072; Idx++)
		{
			if(pmUFDBlockMap->SysBlkAdr[Idx] == 0)
				break;
			Status = setCellMap(pmUFDBlockMap->SysBlkAdr[Idx],pflash,1);
			Status = setEccMap(pmUFDBlockMap->SysBlkAdr[Idx], pflash,0);
		}

		return Status;
}

UINT CRootTable::setEccErrBlock(CFlash *pflash) {
	// TODO Auto-generated constructor stub
	UINT	Status = Success_State, Idx;

		for(Idx=0; Idx<512; Idx++)
		{
			if(pmUFDBlockMap->EccErrBlkAdr[Idx] == 0)
				break;
			Status = setCellMap(pmUFDBlockMap->EccErrBlkAdr[Idx],pflash,1);
			Status = setEccMap(pmUFDBlockMap->EccErrBlkAdr[Idx], pflash,0);
		}

		return Status;
}


ROOT_VARS * CRootTable::getRootTable() {
	// TODO Auto-generated constructor stub

		return pmRootTable;
}

RTblInfo * CRootTable::getRTInfoTable() {
	// TODO Auto-generated constructor stub

		return pmRTInfo;
}

UINT CRootTable::getRootTableVersion() {
	// TODO Auto-generated constructor stub
		return  pmRootTable->dwRTBLVersion;
}

MapChipSelect * CRootTable::getUFDBlockMap() {
	// TODO Auto-generated constructor stub

		return pmUFDBlockMap;
}

void  CRootTable::setUFDBlockMap(MapChipSelect *ufdblockmap) {
	// TODO Auto-generated constructor stub
	memcpy(pmUFDBlockMap,ufdblockmap,sizeof(MapChipSelect));
}

UINT CRootTable::initRTInfoTable() {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
	USHORT	BlockAddr, PageAddr;
	BYTE	buffer[7]={0xFF};
	UINT   FLH_ID[8];
	VendorCMD	VCMD;
	BYTE tPlaneNum;
	UINT tBlockPage;
	tPlaneNum = pmflash->getPlaneNum();
	tBlockPage = pmflash->getBlockPage();

	pmflash->getFlashID(FLH_ID);

	memset(&VCMD, 0x00, sizeof(VendorCMD));
	memset(pmRTInfo, 0xFF, sizeof(RTblInfo));	// Set Defaut RTInfo is 0xFF


	VCMD.OPCode = 0xEE;
	VCMD.MJCMD = 0x51;
	VCMD.Address = 0;
	VCMD.MICMD = 0x01;
	VCMD.BufLen = 7; // Unit is Byte

	Status = pmflash->ReadTestCmd(VCMD, (BYTE *)buffer);

	if( ((buffer[0]==0xFF)&&(buffer[1]==0xFF)) ||((buffer[2]==0xFF)&&(buffer[3]==0xFF)) )// FW Not Find Root_Table
	{
			return Fail_State;
	}

	pmRTInfo->PairAddr = (((USHORT)buffer[1])<<8) + buffer[0];	// Little End, FW Returned is "Pair_Address", Not Real Block_Address
	BlockAddr = pmRTInfo->PairAddr * tPlaneNum;
	PageAddr = (((USHORT)buffer[3])<<8) + buffer[2];	// Little End, FW Returned is "Real_Page_Index"
	pmRTInfo->CellType = buffer[4];	// For RT Protection, This Must Be 0/1
	pmRTInfo->ECCType = buffer[5];	// For RT Protection, This Must Be 0/1
	pmRTInfo->PU_Index = buffer[6];// For RT Protection, This Must Be 0~7

	pmRTInfo->RAWAddr0 = (((ULONG)pmRTInfo->PU_Index) << 28) + (ULONG)BlockAddr * tBlockPage + PageAddr;
	pmRTInfo->RAWAddr1 = (((ULONG)pmRTInfo->PU_Index) << 28) + (ULONG)(BlockAddr+1) * tBlockPage + PageAddr; // For 2 Plane Only

	if(tPlaneNum == 1)
	{
		if(FLH_ID[0] == 0x98)
			pmRTInfo->RAWAddr1 = pmRTInfo->RAWAddr0 + 2;
		else
			pmRTInfo->RAWAddr1 = pmRTInfo->RAWAddr0 + 2;

	}

	if(pmRTInfo->PU_Index > 8)
		Status = Fail_State;

	return Status;
}

UINT CRootTable::initRootTable() {
	// TODO Auto-generated constructor stub
	ULONG RAWAddr0 = pmRTInfo->RAWAddr0;
	ULONG RAWAddr1 = pmRTInfo->RAWAddr1;
	UINT	Status = Fail_State;
	BYTE	ECC;	// = bit_7 + bit_0; // Encryption_On(Bit7) + ECC_On(Bit6) + 40bit_ECC(Bit1 & Bit0)
	SPARETYPE Spare;
	BYTE	Register = 0;


	pmflash->writeData(0x1FF82004, 1, &Register);
	Register = BIT1|BIT0;
	pmflash->writeData( 0x1FF8300D, 1, &Register);

	Status =  pmflash->writeCellMapFlashType(RAWAddr0,pmRTInfo->CellType);//0:MLC 1:SLC

	Status =  pmflash->writeEccMapBitLength(RAWAddr0,pmRTInfo->ECCType);


	Spare.SPARE0 = 0x4D;
	Spare.SPARE1 = 0x50;
	Spare.SPARE2 = 0x00;
	Spare.SPARE3 = 0x00;
	Spare.SPARE4 = 0x99;
	Spare.SPARE5 = 0x99;
	ECC = BIT7 + (pmflash->getFlashModel(3)  & 0x0F); // Encryption_On(Bit7) + ECC_On(Bit6) + 40bit_ECC(Bit1 & Bit0)

	if((pmflash->getFlashModel(6)  & 0x03) == 0x02) // 8K Page, Protection For Read 14K RT
	{
		return Fail_State;
	}

	// ----- Get Root_Table[0] & [1] -----
	Status=pmflash->readBlockData(	(BYTE)(sizeof(ROOT_VARS)*2/512),
									ECC,
									Spare,
									RAWAddr0, //PU + (ULONG)pRTInfo->BlockAddr * BlockPage + pRTInfo->PageAddr,
									sizeof(ROOT_VARS)*2,
									(BYTE *)&pmRootTable[0]);



	// ----- Get Root_Table[2] & [3] -----
	Status=pmflash->readBlockData(	(BYTE)(sizeof(ROOT_VARS)*2/512),
									ECC,
									Spare,
									RAWAddr1, //PU + (ULONG)(pRTInfo->BlockAddr+1) * BlockPage + pRTInfo->PageAddr,
									sizeof(ROOT_VARS)*2,
									(BYTE *)&pmRootTable[2]);

	// ----- Protection Of RT -----
	if((pmRootTable[0].dwRTBLVersion & 0xFFFF0000) != 0x52540000) // RT_TAG Protection
		return Fail_State;

	return Status;
}

UINT CRootTable::updateEraseCount() {
	// TODO Auto-generated constructor stub
	// Read 4-RT From RTAddress & Check
		UINT	Status = Fail_State;
		BYTE	ECC;// = bit_7 + bit_0; // Encryption_On(Bit7) + ECC_On(Bit6) + 40bit_ECC(Bit1 & Bit0)
		BYTE	TxDataBuf[1024];
		UINT   FLH_ID[8];
		UINT	PU_Idx, chMaxCachBufNo = 0, Cache_Idx, TempAddr, OriginNRT, FB_Idx, QDep_Idx, FB_PU, FB_Pair;
		USHORT	BlockPage = pmflash->getBlockPage();
		BYTE tPlaneNum=pmflash->getPlaneNum();
		USHORT tPlaneBlock = pmflash->getPlaneBlock();
		BYTE tChipSelectNum = pmflash->getChipSelectNum();
		pmflash->getFlashID(FLH_ID);
		TurboPageInfo *tturbopageinfo = pmflash->getTurboPageInfo();


		if(mRTInfoINITFlag == Fail_State){

			return Fail_State;
		}

		if((pmRTInfo->CellType>1) || (pmRTInfo->ECCType>1))
		{
			return Fail_State;
		}

		SPARETYPE Spare;
		Spare.SPARE0 = 0x4D;
		Spare.SPARE1 = 0x50;
		Spare.SPARE2 = 0x00;
		Spare.SPARE3 = 0x00;
		Spare.SPARE4 = 0x99;
		Spare.SPARE5 = 0x99;
		memset(TxDataBuf, 0xFF, sizeof(BYTE)*1024);

		ECC = BIT7 + (pmflash->getFlashModel(3)& 0x0F); // Encryption_On(Bit7) + ECC_On(Bit6) + 40bit_ECC(Bit1 & Bit0)
		if(FLH_ID[0] == 0x89)		chMaxCachBufNo = 8;
		else if(FLH_ID[0] == 0x2C)	chMaxCachBufNo = 8;
		else if(FLH_ID[0] == 0x98)	chMaxCachBufNo = 32;
		else if(FLH_ID[0] == 0xEC)	chMaxCachBufNo = 32;

		// ----- Protection Of RT -----

		if((pmRootTable[0].dwRTBLVersion & 0xFFFF0000) != 0x52540000) // RT_TAG Protection
			goto END_OF_SET_RT_EC;

		pmRTInfo->Exist = (BYTE)true;	// Here The RT is Exist

		for(PU_Idx=0; PU_Idx<tChipSelectNum; PU_Idx++)
		{
			if((pmRootTable[PU_Idx].dwRTBLVersion & 0xFFFF0000) != 0x52540000) 	continue; // RT_TAG Protection

			// ----- Write Cache Blocks -----
			for(Cache_Idx=0; Cache_Idx<chMaxCachBufNo; Cache_Idx++)
			{
				Spare.SPARE3 = HIBYTE(pmRootTable[PU_Idx].CBufT[Cache_Idx].wEraCnt);	// HI
				Spare.SPARE2 = LOBYTE(pmRootTable[PU_Idx].CBufT[Cache_Idx].wEraCnt);	// LO
				TempAddr = ((UINT)PU_Idx<<28) + (UINT)(pmRootTable[PU_Idx].CBufT[Cache_Idx].wBlkAdr)*tPlaneNum*BlockPage; // Pair_Addr To RowAddr

				if(pmRootTable[PU_Idx].CBufT[Cache_Idx].wBlkAdr > tPlaneBlock)
				{

					continue; // Protection Before Check
				}

				// Only Message


				if(pmflash->isBlockEmpty(TempAddr) == Fail_State)	continue; // FW Had Write This Block, MPTool No Need To Write Again.


				Status = pmflash->writeBlockData((BYTE)(1024/512),
						ECC,
						Spare,
						(ULONG)TempAddr,
						1024,
						(BYTE *)TxDataBuf);


				if(tPlaneNum == 2) // Sherlock_20141003
				{
					Status = pmflash->writeBlockData((BYTE)(1024/512),
							ECC,
							Spare,
							(ULONG)TempAddr+BlockPage,
							1024,
							(BYTE *)TxDataBuf);
				}

				Status =  pmflash->writeCellMapFlashType(TempAddr,1);//0:MLC 1:SLC
				Status =  pmflash->writeEccMapBitLength(TempAddr,0);
			}

			// ----- Write FB_Blocks -----
			for(FB_Idx=0; FB_Idx<3; FB_Idx++)
			{
				for(QDep_Idx=0; QDep_Idx<QDEPTH_GBLK; QDep_Idx++)
				{

					if(pmRootTable[PU_Idx].FB[FB_Idx].wBlk[QDep_Idx] == 0xFFFF)
						continue;

					Spare.SPARE3 = HIBYTE(pmRootTable[PU_Idx].FB[FB_Idx].wEra[QDep_Idx]);	// HI
					Spare.SPARE2 = LOBYTE(pmRootTable[PU_Idx].FB[FB_Idx].wEra[QDep_Idx]);	// LO
					FB_PU = (UINT)(pmRootTable[PU_Idx].FB[FB_Idx].wBlk[QDep_Idx] >> 12);
					FB_Pair = (UINT)(pmRootTable[PU_Idx].FB[FB_Idx].wBlk[QDep_Idx]  & 0x0FFF);

					TempAddr = (FB_PU<<28) + FB_Pair*tPlaneNum*BlockPage; // Pair_Addr To RowAddr


					if((FB_PU>0x3)||(FB_Pair > tPlaneBlock))
					{
						continue; // Protection Before Check
					}

					if(pmflash->isBlockEmpty(TempAddr) == Fail_State)	continue; // FW Had Write This Block, MPTool No Need To Write Again.

					Status = pmflash->writeBlockData((BYTE)(1024/512),
														ECC,
														Spare,
														(ULONG)TempAddr,
														1024,
														(BYTE *)TxDataBuf);


					if(tPlaneNum == 2) // Sherlock_20141003
					{
						Status = pmflash->writeBlockData((BYTE)(1024/512),
															ECC,
															Spare,
															(ULONG)TempAddr+BlockPage,
															1024,
															(BYTE *)TxDataBuf);
					}
					Status =  pmflash->writeCellMapFlashType(TempAddr,1);//0:MLC 1:SLC
					Status =  pmflash->writeEccMapBitLength(TempAddr,0);
				}
			}

		}


		// ----- Write Next Root_Table -----
		// Only Check PU_0
		// dwNextRTblWPgePtr = BlockPair_Adr * TurboPage_Num + TurboPage_Index

		PU_Idx = 0;
		Spare.SPARE3 = HIBYTE(pmRootTable[PU_Idx].wNextRTblEraCnt);	// HI
		Spare.SPARE2 = LOBYTE(pmRootTable[PU_Idx].wNextRTblEraCnt);	// LO
		OriginNRT = ((UINT)PU_Idx<<28) + (UINT)(pmRootTable[PU_Idx].dwNextRTblWPgePtr)*tPlaneNum*(BlockPage/tturbopageinfo->TurboPageNUM); // Pair Addr To Row Addr
		TempAddr = 	(OriginNRT / BlockPage) * BlockPage;


		if((TempAddr&0x00FFFFFF) > (UINT)tPlaneBlock*tPlaneNum*BlockPage)
			goto END_OF_SET_RT_EC; // Protection Before Check


		if(pmflash->isBlockEmpty(TempAddr) == Fail_State)
			goto END_OF_SET_RT_EC; // FW Had Write This Block, MPTool No Need To Write Again.



	//	Status = pVLIBaseAPICtrl2->BlockErase(HandleList, DiskPathIndex, TempAddr, &EraseStatus); No Need To Erase Again
		Status = pmflash->writeBlockData((BYTE)(1024/512),
											ECC,
											Spare,
											(ULONG)TempAddr,
											1024,
											(BYTE *)TxDataBuf);
		if(tPlaneNum == 2) // Sherlock_20141003
		{
			Status = pmflash->writeBlockData((BYTE)(1024/512),
												ECC,
												Spare,
												(ULONG)TempAddr+BlockPage,
												1024,
												(BYTE *)TxDataBuf);
		}

		Status =  pmflash->writeCellMapFlashType(TempAddr,1);//0:MLC 1:SLC
		Status =  pmflash->writeEccMapBitLength(TempAddr,0);

	END_OF_SET_RT_EC:

		return Status;
}

UINT CRootTable::writeCellMap() {
	// TODO Auto-generated constructor stub
		UINT Status = Fail_State;
		ULONG PU_Idx, TableLen = 512, Offset;//, BlockEnd
		BYTE tChipSelectNum = pmflash->getChipSelectNum();
		//----- Protection of PairMap -----
		for(PU_Idx=0; PU_Idx<tChipSelectNum; PU_Idx++)
		{
			if((pmRootTable[PU_Idx].dwRTBLVersion & 0xFFFF0000) != 0x52540000) // RT_TAG Protection
			{
				memset(&pmRootTable[PU_Idx].chBlkTagBitMapTbl[0], 0xFF, 262);	// Clear As MLC
			}
		}

		Offset = 0;
		for(PU_Idx=0; PU_Idx<tChipSelectNum; PU_Idx++)
		{	// 5000~5800: 4 CellMap For 4 PUs
			Status = pmflash->writeData(0x1FF85000+Offset, (USHORT)256, &pmRootTable[PU_Idx].chBlkTagBitMapTbl[0]);; // Max_Len = 256
			Status = pmflash->writeData( 0x1FF85000+Offset+256, (USHORT)262-256, &pmRootTable[PU_Idx].chBlkTagBitMapTbl[256]);
			Offset += TableLen;
		}

		return Status;
}

UINT CRootTable::writeEccMap() {
	// TODO Auto-generated constructor stub
	UINT Status = Fail_State;
	ULONG PU_Idx, TableLen = 512, Offset;//, BlockEnd
	BYTE tChipSelectNum = pmflash->getChipSelectNum();
	//----- Protection of PairMap -----
	for(PU_Idx=0; PU_Idx<tChipSelectNum; PU_Idx++)
	{
		if((pmRootTable[PU_Idx].dwRTBLVersion & 0xFFFF0000) != 0x52540000) // RT_TAG Protection
		{
			memset(&pmRootTable[PU_Idx].chBlkECCBitMapTbl[0], 0x00, 262);	// Clear As Default ECC
		}
	}
	Offset= 0;
	for(PU_Idx=0; PU_Idx<tChipSelectNum; PU_Idx++)
	{	// 5800~6000: 4 ECCMap For 4 PUs
		Status = pmflash->writeData(0x1FF85800+Offset, (USHORT)256, &pmRootTable[PU_Idx].chBlkECCBitMapTbl[0]);
		Status = pmflash->writeData(0x1FF85800+Offset+256, (USHORT)262-256, &pmRootTable[PU_Idx].chBlkECCBitMapTbl[256]);
		Offset += TableLen;
	}
	return Status;
}

UINT CRootTable::ScanBlock(SettingConfgInfo *pmCurSettingConfgInfo) {
	// TODO Auto-generated constructor stub
	BOOL	Status1 = true,EccErrSts = true;
	int 		ChipIndex, ErrorCnt=0, ScanChipNo;
	UINT  	EntryIndex, EntryItemNo, Address=0, Count=0, ChipAddress, bufIndex, BlkAddr, ExtendedBlock, PlaneBlock;
	BYTE 	ChipSelectNo, ChannelNo, Chk1, Chk2, Register2, Register3, DumpRec=0, ForcedBad=0, EncryptionFlash; //Register1, EncryptionSet
	BYTE	EncryptionCMD_Scan=0, EncryptionCMD_EraseCnt=0, ECCSet=0; // Sherlock_20131025, 0x80=Encryption_ON, 0x00=Encryption_OFF
	BYTE 	buffer1[MaxChekBlockNo], buffer2[MaxChekBlockNo], buffer3[MaxChekBlockNo], buffer4[MaxChekBlockNo], buffertmp;
	ULONG	Reg_FlashAccess = 0x1FF82600; //Reg_Encryption = 0x1FF82809
	ULONG	Index,TxLen;
	BYTE	InternalChip = 1,EccErrEccSet = 0;
	UINT	BasicBlock;
	UINT	Lun1End, Lun2Start, Lun2End, Lun3Start; // 2 Internal Chip Use, Lun1Start = 0
	UINT	Lun3End, Lun4Start, Lun4End, Lun5Start; // 4 Internal Chip Use
	UINT	EraseCount=0, SysBlkCnt=0,EccErrBlkCnt = 0;
	UINT 	m_BlockPage;
	UINT	m_PageSize;
	UINT	LastPage,Status = 1;
	BYTE	BitErrorCnt = 0,ValueTmp = 0,PageSec ;
    BYTE 	*pTxBuf;
	BYTE 	*pRxBuf;
	UINT	ErrorBitRate = 24;//coolio temporary setting 20150909

	BYTE	Config = 0x23;
	Config = Config | (Scan_ClearSysBlk | Scan_EraseEncrypt);
	//if(pThreadInfo->instp->m_CurMPToolMode & SaveBBInfoToFlash)
	//	Config |= Scan_SaveBadBlkInfo; //bit_5; // Save Bad Block Info To Flash
	if(pmCurSettingConfgInfo->ARFlashType == 0x00)
		Config |= Scan_CheckBadCnt;
	if(pmCurSettingConfgInfo->LunTypeBitMap & EraseGoodBlk)
		Config |= Scan_EraseGoodBlk;
	if(pmCurSettingConfgInfo->LunTypeBitMap & DumpDebugMemory)
		Config |= Scan_DumpMessage; // 0x80;	// Sherlock_20110614, Dump BadBlockRec.txt Enable
  //coolio test
	if(!(pmCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)  && !(((pmRootTable[0].dwRTBLVersion & 0xFFFF0000) != 0x52540000)  &&
		       (pmflash->isOldVersionCISExit() == Success_State))) //Sherlock_20131015, Get New CIS Address ,Special Function of Keep old CIS data For RE-MP ,Cody_20150217
	{
			Config&=(0xF3);//(~(Sc
	}



	ExtendedBlock = (UINT)pmCurSettingConfgInfo->CisDataEx.FLH_FwScheme.LessBlock;	// Extract Extended Block Info. From UI

	ChipSelectNo	= pmUFDBlockMap->ChipSelectNum;
	ChannelNo	= pmUFDBlockMap->ChannelNum;
	EntryItemNo	= pmUFDBlockMap->EntryItemNum;
	m_BlockPage	= pmflash->getBlockPage();
	m_PageSize	= pmflash->getPageSize();
	PageSec = m_PageSize/512;
	LastPage = m_BlockPage;
	TxLen = m_PageSize;
	pTxBuf = new BYTE[TxLen];
	pRxBuf = new BYTE[TxLen];

	// ----- Sherlock_20121112, Only Scan 1 Chip -----
	if(Config & Scan_ScanOneChip)
		ScanChipNo = 1;
	else
		ScanChipNo = ChipSelectNo*ChannelNo;

	//  ----- Sherlock_20110928, Get Chip ID, Start -----
	BYTE	Code200A = 0x36, Code200B = 0x38;
	// Sherlock_20120130
	BOOL	GetBBInfoAddr = false; // Get a Block to Save Bad Block Info
	UINT	SaveBBInfoAddr = 0x00;//, TempAddress, Idx;
	SaveBadBlockInfo	BadBlockInfo;
	BYTE	PageIdx=0;//, *ptr;

	memset(&BadBlockInfo, 0xFF, sizeof(SaveBadBlockInfo));

	BadBlockInfo.BBIName[0] = 0x42616442;	// = 'BadB'
	BadBlockInfo.BBIName[1] = 0x6C6F636B;	// = 'lock'
	BadBlockInfo.BBIName[2] = 0x436E743D;	// = 'Cnt='
	BadBlockInfo.BBICnt = 0x00;


	// ----- Sherlock_20111007, Get_Flash_ID, Start -----
	BYTE * IDBuffer = new BYTE[64];
	WORD	BaseFType = Normal_M_2P; // Default is Normal
	USHORT	SLC_BlockPage;

	// ----- Read Flash ID -----
	Status=pmflash->UFDSettingRead(MI_READ_FLASH_ID, 0, 0, 0, 0, 64, IDBuffer);
//	ModifyFlashID(IDBuffer);
	BaseFType = pmflash->getBaseFType();

	if((BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) // Toshiba ED3
		SLC_BlockPage = 86;
	else if((BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Samsung|FT_TLC)) // Samsung TLC
		SLC_BlockPage = 64;
	else
		SLC_BlockPage = m_BlockPage; // Default is Normal Flash


	// ----- Get Flash Type For Erase Encryption Good Block -----
	if((BaseFType & FT_Vendor) == FT_Toshiba)
		EncryptionFlash = 1;
	else if((BaseFType & FT_Vendor) == FT_Samsung)
		EncryptionFlash = 1;
	else
		EncryptionFlash = 0;

	// ----- ECC Setting For Block Write -----
	if((BaseFType & (FT_Cell_Level)) == (FT_TLC))
		ECCSet = BIT7 + BIT1; // TLC + 60-bit ECC
	else if((IDBuffer[0] == 0x98) ||(IDBuffer[0] == 0xEC) || (IDBuffer[0] == 0x45))
		ECCSet = BIT7 + BIT0; // Toshiba or Samsung or SanDisk 40-bit ECC
	else
		ECCSet = BIT0;

	// ----- Encryption Command Set For Erase Count -----
	if((BaseFType & (FT_Cell_Level)) == (FT_TLC))
		EncryptionCMD_EraseCnt = BIT7 + 2; // TLC + 60bit_ECC
	else if((IDBuffer[0] == 0x98) ||(IDBuffer[0] == 0xEC) || (IDBuffer[0] == 0x45))
		EncryptionCMD_EraseCnt = BIT7 + 1; // Toshiba or Samsung or SanDisk + 40bit_ECC
	else
		EncryptionCMD_EraseCnt = 0 + 1; // Others + 40bit_ECC

	// ----- Get Internal Chip Number & Address -----

		if((IDBuffer[2] & 0x03) != 0x00) // Multi-Internal Chip Exist
		{
			InternalChip = (0x01 << (IDBuffer[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }

			if((IDBuffer[0]==0x98) || (IDBuffer[0]==0xEC) || (IDBuffer[0]==0x45))
				BasicBlock = pmUFDBlockMap->EntryItemNum * 8 / 2 / InternalChip;
			else if((IDBuffer[0]==0x2C) || (IDBuffer[0]==0x89))
				BasicBlock = pmUFDBlockMap->EntryItemNum * 8 / InternalChip;

			Lun1End = BasicBlock+ ExtendedBlock - 1;
			Lun2Start = 2*BasicBlock;
			Lun2End = 3*BasicBlock+ ExtendedBlock - 1;
			Lun3Start = 4*BasicBlock;
			Lun3End = 5*BasicBlock+ ExtendedBlock - 1;
			Lun4Start = 6*BasicBlock;
			Lun4End = 7*BasicBlock+ ExtendedBlock - 1;
			Lun5Start = 8*BasicBlock;
			PlaneBlock = 3*BasicBlock+ ExtendedBlock;
		}
		else // Sherlock_20140724, For InternalChip is 0, Make Entry as 0x200*N
		{
			BasicBlock = pmUFDBlockMap->EntryItemNum * 8;
			if(ExtendedBlock)
				PlaneBlock = BasicBlock + ExtendedBlock - 0x200;
			else
				PlaneBlock = BasicBlock;
		}
	delete IDBuffer;
	// ----- Get_Flash_ID, End -----

 // After Reset, Chip Use Default Setting(128-Page), So Must Set This Again.
	ErrorCnt = 0;
ReReadFlashAccess:
	Status = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, Reg_FlashAccess, 1, &Register2); // Read Reg[2300]
	if(!Status)	// Sherlock_20121107
	{
		ErrorCnt++;
		if(ErrorCnt<10)
			goto ReReadFlashAccess;
		else
			goto EndScanFlashBlock;
	}
	if(m_BlockPage == 64)
		Register3 = (Register2 & (0xFC)) + 0; //(~(bit_1|bit_0))
	else if(m_BlockPage == 128)
		Register3 = (Register2 & (0xFC)) + 1; // Set bit_0
	else if(m_BlockPage == 256)
		Register3 = (Register2 & (0xFC)) + 2; // Set bit_1
	else if(m_BlockPage == 512)
		Register3 = (Register2 & (0xFC)) + 3; // Set bit_1|bit_0
	else
		Register3 = Register2;				// No Match, No Change

	if(m_PageSize == 16*1024)
		Register3 = (Register3 & (0xF3)) + 0; // [3:2] = 00b
	else if(m_PageSize == 4*1024)
		Register3 = (Register3 & (0xF3)) + 4; // [3:2] = 01b
	else if(m_PageSize == 8*1024)
		Register3 = (Register3 & (0xF3)) + 8; // [3:2] = 10b

	ErrorCnt = 0;
SetPageNumber: // ---------- Set Page Number ----------
	Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_FlashAccess, 1, &Register3); // Write Reg[2300]
	if(!Status)
	{

		ErrorCnt++;
		if(ErrorCnt<10)
			goto SetPageNumber;
		else
			goto EndScanFlashBlock;
	}


	//if(Config & Scan_DumpMessage)
	//	DumpRec = 1; // Sherlock_20110614, Dump BadBlockRec.txt Enable

	FILE * BadBlockRec;

	for(ChipIndex=0; ChipIndex<ScanChipNo; ChipIndex++)
	{
		Count=0;
		pmUFDBlockMap->BadBlockCnt[ChipIndex]=0;
		ChipAddress=(((ChipIndex/ChannelNo)<<4)|(ChipIndex%ChannelNo))<<24;
		for(EntryIndex=0;EntryIndex<EntryItemNo;EntryIndex+=(MaxChekBlockNo/8)) //64=0x200/8
		{

			// ----- Skip Scan Not Exist Block Of Samsung -----
			BlkAddr = 8*EntryIndex;
			if((InternalChip == 2) && (ExtendedBlock != 0x00)) // Sherlock_20120425, Only Support 2 Internal Chip
			{ // These Blocks Are Not Exist, But FW May Misjudge As Good Block
				if(((Lun1End<BlkAddr)&&(BlkAddr<Lun2Start)) || ((Lun2End<BlkAddr)&&(BlkAddr<Lun3Start)))
				{
					goto JudgeBlock;
				}
			}
			else if((InternalChip == 4) && (ExtendedBlock != 0x00)) // Sherlock_20120425, Only Support 4 Internal Chip
			{ // These Blocks Are Not Exist, But FW May Misjudge As Good Block
				if(	((Lun1End<BlkAddr)&&(BlkAddr<Lun2Start)) || ((Lun2End<BlkAddr)&&(BlkAddr<Lun3Start)) ||
					((Lun3End<BlkAddr)&&(BlkAddr<Lun4Start)) || ((Lun4End<BlkAddr)&&(BlkAddr<Lun5Start))	)
				{
					goto JudgeBlock;
				}
			}

			Address=(Count*(m_BlockPage*MaxChekBlockNo))|(ChipAddress);

			if(Config & Scan_PreMPTestMode)
			{
				Config = Config & (~Scan_EraseEncrypt); // Clear BIT_3: NOT Erase Encryption Good Block
			}

			EncryptionCMD_Scan &= (~BIT7); // Sherlock_20131025, Clear Encryption_CMD
			EncryptionCMD_Scan = EncryptionCMD_Scan+1;

			ErrorCnt = 0;
	ReSendCmdAgain00_Off: // ---------- Scan_1, Page 0 With Encryption OFF ----------
			Status = pmflash->BlockCheckRead(MI_CHK_BAD, 0, 0, Address, MaxChekBlockNo, HIBYTE(m_PageSize), LOBYTE(m_PageSize), HIBYTE(m_BlockPage), LOBYTE(m_BlockPage), EncryptionCMD_Scan, PlaneBlock, buffer1);
			if(!Status)
			{
				ErrorCnt++;
				if(ErrorCnt<10)	goto ReSendCmdAgain00_Off;
				else				goto EndScanFlashBlock;
			}

			ErrorCnt = 0;
	ReSendCmdAgainFF_Off: // ---------- Scan_2, Page FF With Encryption OFF ----------
			Status = pmflash->BlockCheckRead(MI_CHK_BAD, 0, 0, Address+(SLC_BlockPage-1), MaxChekBlockNo, HIBYTE(m_PageSize), LOBYTE(m_PageSize), HIBYTE(m_BlockPage), LOBYTE(m_BlockPage), EncryptionCMD_Scan, PlaneBlock, buffer2);	// ED3 SLC Issue
			if(!Status)
			{
				ErrorCnt++;
				if(ErrorCnt<10)	goto ReSendCmdAgainFF_Off;
				else				goto EndScanFlashBlock;
			}

			EncryptionCMD_Scan |= BIT7; // Sherlock_20131025, Set Encryption_CMD
			EncryptionCMD_Scan = EncryptionCMD_Scan +1;
//Only_Scan_Mode: // Scan Without Change Encryption Mode

			ErrorCnt = 0;
	ReSendCmdAgain00_On: // ---------- Scan_3, Page 0 With Encryption ON ----------
			Status = pmflash->BlockCheckRead(MI_CHK_BAD, 0, 0, Address, MaxChekBlockNo, HIBYTE(m_PageSize), LOBYTE(m_PageSize), HIBYTE(m_BlockPage), LOBYTE(m_BlockPage), EncryptionCMD_Scan, PlaneBlock, buffer3);
			if(!Status)
			{
				ErrorCnt++;
				if(ErrorCnt<10) 	goto ReSendCmdAgain00_On;
				else				goto EndScanFlashBlock;
			}

			ErrorCnt = 0;
	ReSendCmdAgainFF_On: // ---------- Scan_4, Page FF With Encryption ON ----------
		Status = pmflash->BlockCheckRead(MI_CHK_BAD, 0, 0, Address+(SLC_BlockPage-1), MaxChekBlockNo, HIBYTE(m_PageSize), LOBYTE(m_PageSize), HIBYTE(m_BlockPage), LOBYTE(m_BlockPage), EncryptionCMD_Scan, PlaneBlock, buffer4);	// ED3 SLC Issue
			if(!Status)
			{
				ErrorCnt++;
				if(ErrorCnt<10) 	goto ReSendCmdAgainFF_On;
				else				goto EndScanFlashBlock;
			}

JudgeBlock:
			// ---------- Judge Block Start ----------
			for(bufIndex=0;bufIndex<MaxChekBlockNo;bufIndex++)
			{

				// Judge Encryption OFF Part
				if( (buffer1[bufIndex]==0x01) || (buffer2[bufIndex]==0x01) )
					Chk1 = 1; // Bad
				else if(buffer1[bufIndex]==0x02)
					Chk1 = 2; // System
				else if(buffer1[bufIndex]==0x03)
					Chk1 = 3; //Cody_20150224 ECC unrecoverable error
				else
					Chk1 = 0; // Good

				// Judge Encryption ON Part
				if( (buffer3[bufIndex]==0x01) || (buffer4[bufIndex]==0x01) )
					Chk2 = 1; // Bad
				else if(buffer3[bufIndex]==0x02)
					Chk2 = 2; // System
				else if(buffer3[bufIndex]==0x03)
					Chk2 = 3; //Cody_20150224 ECC unrecoverable error
				else
					Chk2 = 0; // Good

				ForcedBad = 0; // Initialize This Forced BadBlock Flag
				BlkAddr = 512*Count + bufIndex;
				if((InternalChip == 2)  && (ExtendedBlock != 0x00))// Sherlock_20120425, Only Support 2 Internal Chip
				{ 	// These Blocks Are Not Exist, But FW May Misjudge As Good Block
					if(((Lun1End<BlkAddr)&&(BlkAddr<Lun2Start)) || ((Lun2End<BlkAddr)&&(BlkAddr<Lun3Start)))
					{
						Chk1 = 1; // Force It To Be Bad Block
						Chk2 = 1;
						ForcedBad = 1; // Forced BadBlock By Samsung Spec, Not Really BadBlock
					}
				}
				else if((InternalChip == 4)  && (ExtendedBlock != 0x00))// Sherlock_20120425, Only Support 4 Internal Chip
				{ 	// These Blocks Are Not Exist, But FW May Misjudge As Good Block
					if(	((Lun1End<BlkAddr)&&(BlkAddr<Lun2Start)) || ((Lun2End<BlkAddr)&&(BlkAddr<Lun3Start)) ||
						((Lun3End<BlkAddr)&&(BlkAddr<Lun4Start)) || ((Lun4End<BlkAddr)&&(BlkAddr<Lun5Start))	)
					{
						Chk1 = 1; // Force It To Be Bad Block
						Chk2 = 1;
						ForcedBad = 1; // Forced BadBlock By Samsung Spec, Not Really BadBlock
					}
				}

				// Sherlock_20120321
				if(ExtendedBlock) // Extended Block Is Exist
				{
					if(EntryIndex >= (EntryItemNo-(MaxChekBlockNo/8))) // Last Scan Cycle
					{
						if(bufIndex >= (ExtendedBlock%0x200)) // Block After Extended Block, Should Be Not Exist Block
						{	// These Blocks Are Not Exist, But FW May Misjudge As Good Block
							Chk1 = 1; 	// Force It To Be Bad Block
							Chk2 = 1;
							ForcedBad = 1;
						}
					}
				}


				if( (Chk1 == 2) ||(Chk2 == 2) ) //System Block
				{
					if(Config & Scan_ClearSysBlk)	// BIT_0: Clear System Block
					{
						ErrorCnt=0;
ReSendEraseSysBlock:

						UINT	TempEraseCount = 0;
						SPARETYPE Spare;
						Spare.SPARE0=0x4D; //'M'
						Spare.SPARE1=0x50; //'P'
						Spare.SPARE2=0x00;
						Spare.SPARE3=0x00;
						Spare.SPARE4=0x99;
						Spare.SPARE5=0x99;
						Status = pmflash->getEraseCount(Address+(bufIndex*m_BlockPage),EncryptionCMD_EraseCnt>>7,2,&TempEraseCount);	// Sherlock_20140421, Always EncriptionCMD for FW System Block
						Spare.SPARE3=(BYTE)((TempEraseCount + 1)>>8);	// HI
						Spare.SPARE2=(BYTE)(TempEraseCount + 1);		// LO

						Status = UpdatePairMapByAddress(Address+(bufIndex*m_BlockPage), m_BlockPage, pmflash->getPlaneNum(), 1, 0);

						Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp);
						// ----- Sherlock_20140114, If DataLen < PageSector, Use DataLen/512 into cdb[7] -----
						BadBlockInfo.BBIName[0] = 0xFFFFFFFF;	// =Clear
						BadBlockInfo.BBIName[1] = 0xFFFFFFFF;	// =Clear
						BadBlockInfo.BBIName[2] = 0xFFFFFFFF;	// =Clear
						Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, (sizeof(SaveBadBlockInfo)/512), ECCSet, Spare, 0, 0, Address+(bufIndex*m_BlockPage), sizeof(SaveBadBlockInfo), (BYTE *)&BadBlockInfo);
						BadBlockInfo.BBIName[0] = 0x42616442;	// = 'BadB'
						BadBlockInfo.BBIName[1] = 0x6C6F636B;	// = 'lock'
						BadBlockInfo.BBIName[2] = 0x436E743D;	// = 'Cnt='



						// Sherlock_20140730
						if(SysBlkCnt < 3072) // Protection Cody_20150326 add sysblk buffer
						{
							pmUFDBlockMap->SysBlkAdr[SysBlkCnt] = Address+(bufIndex*m_BlockPage);
							SysBlkCnt++;
						}


						if(!Status)
						{
							ErrorCnt++;
							if(ErrorCnt<10)	goto ReSendEraseSysBlock;
							else				goto EndScanFlashBlock;
						}
					}

					setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 0);

				}
				else if( (Chk1 == 0) ||(Chk2 == 0) )  //Good Block
				{
					if(Config & Scan_EraseGoodBlk)	// BIT_2: Erase Good Block
					{
						ErrorCnt=0;

						Status = UpdatePairMapByAddress(Address+(bufIndex*m_BlockPage), m_BlockPage, pmflash->getPlaneNum(), 1, 0);
ReSendEraseGoodBlock:
						Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp);
						if(!Status)
						{

							ErrorCnt++;
							if(ErrorCnt<10)
								goto ReSendEraseGoodBlock;
							else
								goto EndScanFlashBlock;
						}
					}
 // Sherlock_20110810, Add Erase Good Block of Another Encryption Mode
					else if(Config & Scan_EraseEncrypt)	// BIT_3: Erase Encryption Good Block
					{
						if(((EncryptionFlash == 1) && (Chk1 == 0) && (Chk2 == 1)) || ((EncryptionFlash == 0) && (Chk1 == 1) && (Chk2 == 0)))
						{ // IF (FlashNeedEncryption==true,OFF==Good,ON==Bad)  or (FlashNeedEncryption==false,OFF==Bad,ON==Good)

							ErrorCnt=0;
ReSendEraseEncryptionGoodBlock:
							Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp);
							if(!Status)
							{
								ErrorCnt++;
								if(ErrorCnt<10) 	goto ReSendEraseEncryptionGoodBlock;
								else				goto EndScanFlashBlock;
							}
						}
					}


					setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 0);

// Sherlock_20120130
					if((Config&0x60)==Scan_SaveBadBlkInfo) // bit_5==1 && bit_6==0, Write Bad Block into Flash Enable
					{
						if(!GetBBInfoAddr) // Only Execute 1 time To Get A Bad Info. Block
						{
							GetBBInfoAddr = true;
							SaveBBInfoAddr = Address+(bufIndex*m_BlockPage);
							setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 1); // Set As Bad
							Status=pmflash->getEraseCount(SaveBBInfoAddr,EncryptionCMD_EraseCnt>>7,2,&EraseCount);

						}
					}

				}else if((Chk1 == 3) &&(Chk2 == 3)){//Cody_20150224 ECC unrecoverable error
					ErrorCnt = 0;
					EccErrSts = true;
					if((IDBuffer[0] == 0xAD))// && (ChipVer == 0xD0))
						EccErrEccSet = 0; //EccErrEccSet -> Encryption off, ECC 56bits
					else
						EccErrEccSet = 1; //EccErrEccSet -> Encryption off, ECC 40 bits

					UINT	EccErrEraseCount = 0;

					SPARETYPE EccErrSpare;
					EccErrSpare.SPARE0=0x4D; //'M'
					EccErrSpare.SPARE1=0x50; //'P'
					EccErrSpare.SPARE2=0x00;
					EccErrSpare.SPARE3=0x00;
					EccErrSpare.SPARE4=0x99;
					EccErrSpare.SPARE5=0x99;

					EccErrSpare.SPARE3=(BYTE)((EccErrEraseCount + 1)>>8);	// HI
					EccErrSpare.SPARE2=(BYTE)(EccErrEraseCount + 1);		// LO

					Status = UpdatePairMapByAddress(Address+(bufIndex*m_BlockPage), m_BlockPage, pmflash->getPlaneNum(), 1, 0);

Retry_BST_Erase:
					Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp);
					if(!Status)
					{
						usleep(20000);	// Sherlock_20121130, Test
						ErrorCnt++;
						if(ErrorCnt <10)		goto Retry_BST_Erase;
						else						goto EndScanFlashBlock;
					}

					ErrorCnt = 0;
Retry_BST_Write:
					Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, PageSec, EccErrEccSet, EccErrSpare, 0, 0, Address+(bufIndex*m_BlockPage), TxLen, pTxBuf);
					if(!Status)
					{
						usleep(20000);	// Sherlock_20121130, Test
						ErrorCnt++;
						if(ErrorCnt < 10)		goto Retry_BST_Write;
						else						goto EndScanFlashBlock;
					}

					ErrorCnt = 0;
Retry_BST_Read:
					Status=pmflash->BlockAccessRead(MI_BLOCK_READ,PageSec, EccErrEccSet, 0, 0, m_BlockPage, Address+(bufIndex*m_BlockPage), TxLen, pRxBuf);
					if(!Status)
					{
						usleep(20000);	// Sherlock_20121130, Test
						ErrorCnt++;
						if(ErrorCnt < 10)
							goto Retry_BST_Read;
						else
							goto EndScanFlashBlock;
					}

					for(Index=0;Index< TxLen; Index++)
					{
						if((Index%1024)==0)
						BitErrorCnt=0;

						ValueTmp=pTxBuf[Index]^pRxBuf[Index];

						if(ValueTmp)
						{
							//DebugStr.Format(_T("1(%d)=%x,%d"), DiskPathIndex, ValueTmp, BitErrorCnt);	OutputDebugString(DebugStr);
							BitErrorCnt+=BitCount(ValueTmp);
							//DebugStr.Format(_T("2(%d)=%d"), DiskPathIndex, BitErrorCnt);					OutputDebugString(DebugStr);
						}

						if(BitErrorCnt > ErrorBitRate)
						{
							Status=pmflash->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp); // Sherlock_20121005, Add Erase Before MarkBad
							//MemoryDumpToFile(pTxBuf+Offset, MaxTransfer, pRxBuf, MaxTransfer, DbgStr);

							Status=pmflash->MarkBad(Address+(bufIndex*m_BlockPage),PageSec,LastPage); //mark bad
							setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 1);
							EccErrSts = false;
							break;
						}
					}


					if(EccErrSts)
					{
						if((IDBuffer[0] == 0xAD))// && (ChipVer == 0xD0))
							EccErrEccSet = BIT7+0; //EccErrEccSet -> Encryption on, ECC 56bits
						else
							EccErrEccSet = BIT7+1; //EccErrEccSet -> Encryption on, ECC 40 bits

						EccErrSpare.SPARE3=(BYTE)((EccErrEraseCount + 2)>>8);	// HI
						EccErrSpare.SPARE2=(BYTE)(EccErrEraseCount + 2);		// LO

						ErrorCnt =0;
ReSendEraseEccErrBlock:

						Status=pmflash->BlockOtherRead( MI_BLOCK_ERASE, 0, 0, Address+(bufIndex*m_BlockPage), 1, &buffertmp);

						if(!Status)
						{
							ErrorCnt++;
							if(ErrorCnt<10)	goto ReSendEraseEccErrBlock;
							else				goto EndScanFlashBlock;
						}

						ErrorCnt =0;
						BadBlockInfo.BBIName[0] = 0xFFFFFFFF;	// =Clear
						BadBlockInfo.BBIName[1] = 0xFFFFFFFF;	// =Clear
						BadBlockInfo.BBIName[2] = 0xFFFFFFFF;	// =Clear
ReSendWriteEccErrBlock:

						Status=pmflash->BlockAccessWrite(MI_BLOCK_WRITE, (sizeof(SaveBadBlockInfo)/512), EccErrEccSet, EccErrSpare, 0, 0, Address+(bufIndex*m_BlockPage), sizeof(SaveBadBlockInfo), (BYTE *)&BadBlockInfo);

						if(!Status)
						{
							ErrorCnt++;
							if(ErrorCnt<10)	goto ReSendWriteEccErrBlock;
							else				goto EndScanFlashBlock;
						}

						BadBlockInfo.BBIName[0] = 0x42616442;	// = 'BadB'
						BadBlockInfo.BBIName[1] = 0x6C6F636B;	// = 'lock'
						BadBlockInfo.BBIName[2] = 0x436E743D;	// = 'Cnt='

						if( EccErrBlkCnt < 512) // Protection
						{
							pmUFDBlockMap->EccErrBlkAdr[EccErrBlkCnt] = Address+(bufIndex*m_BlockPage);
							 EccErrBlkCnt++;
						}

						setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 0);
					}

				}else {// Bad Block

					setMapEntryItem((ChipIndex/ChannelNo), (ChipIndex%ChannelNo), (EntryIndex+(bufIndex/8)), (bufIndex%8), 1);

					if(ForcedBad == 0) // Count, If It's Not Forced_Bad_Block (Real Exist Block)
						pmUFDBlockMap->BadBlockCnt[ChipIndex]++;
					// Sherlock_20120130
					if(((Config&0x60)==Scan_SaveBadBlkInfo) && (ForcedBad == 0))// bit_5==1 && bit_6==0 && Not Forced_Bad_Block, Write Bad Block into Flash Enable(Real Exist Block)
					{
						if(BadBlockInfo.BBICnt < 1020)// Write Max 1020 Info (1 Info = 4 Bytes)
						{
							BadBlockInfo.BBIAddr[BadBlockInfo.BBICnt] = Address+(bufIndex*m_BlockPage);
							BadBlockInfo.BBICnt++;

						}

						// ----- Sherlock_20130313, Multi-Save BBI Page -----
						if(BadBlockInfo.BBICnt == 1020)
						{
							Status1 = pmflash->WriteBadBlockInfoToFlash(&BadBlockInfo, SaveBBInfoAddr, PageIdx, ECCSet, EraseCount);
							PageIdx++;

							// Clean All Info to 0xFF
							memset(&BadBlockInfo, 0xFF, sizeof(SaveBadBlockInfo));

							BadBlockInfo.BBIName[0] = 0x42616442;	// = 'BadB'
							BadBlockInfo.BBIName[1] = 0x6C6F636B;	// = 'lock'
							BadBlockInfo.BBIName[2] = 0x436E743D;	// = 'Cnt='
							BadBlockInfo.BBICnt = 0x00;
						}

					}

				}

			}
			// ----- Judge Block End -----

			Count++;
		}

	}

EndScanFlashBlock:	//Sherlock_20110504, Add Exit

// Sherlock_20120130
	if((Config&0x60)==Scan_SaveBadBlkInfo) // bit_5==1 && bit_6==0, Write Bad Block into Flash Enable
	{
		if(Status) // Only Write if Scan Success
			Status1 = pmflash->WriteBadBlockInfoToFlash(&BadBlockInfo, SaveBBInfoAddr, PageIdx, ECCSet, EraseCount);
	}
//////////////////////////
	/*
	Status = pmflash->ScanBlock(pmUFDBlockMap,Config);

	setUFDBlockMap(pmUFDBlockMap);
*/
	return Status;
}


BYTE CRootTable::getMapEntryItem(INT EntryItemIndex, BYTE BlockItemIndex) {
	// TODO Auto-generated constructor stub
	BYTE CEIndex = pmflash->getChipSelectNum();
	BYTE ChannelIndex = pmflash->getChannelNum();
	switch(BlockItemIndex)
		{
		case 0:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block0;
		case 1:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block1;
		case 2:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block2;
		case 3:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block3;
		case 4:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block4;
		case 5:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block5;
		case 6:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block6;
		case 7:
			return ((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block7;
		}
		return 0xFF;
}

void CRootTable::setMapEntryItem(BYTE CEIndex, BYTE ChannelIndex,INT EntryItemIndex, BYTE BlockItemIndex, BYTE Value) {
	// TODO Auto-generated constructor stub
	switch(BlockItemIndex)
		{
		case 0:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block0=Value;
			break;
		case 1:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block1=Value;
			break;
		case 2:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block2=Value;
			break;
		case 3:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block3=Value;
			break;
		case 4:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block4=Value;
			break;
		case 5:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block5=Value;
			break;
		case 6:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block6=Value;
			break;
		case 7:
			((LPMapEntryItem)pmUFDBlockMap->CEItem[CEIndex]->ChannelItem[ChannelIndex])[EntryItemIndex].MapToBit.Block7=Value;
			break;
		}

}

UINT CRootTable:: EraseAllBlock(){

	UINT Status= Success_State;
	UINT BaseFID[8];
	BYTE CE, CH, ChipIndex;
	UINT Entry, Address, ChipAddress=0, BlockIndex, PseudoBlockStart;
	BYTE  EraseStatus, PageSEC, RetryCnt = 0;
	USHORT BlockPage, LessPage, ExtendedBlock;

	pmflash->clearoldversionCISflag();
	pmflash->getFlashID(BaseFID);

	BlockPage	= pmflash->getBlockPage();
	PageSEC		= pmflash->getPageSEC();
	LessPage	= pmflash->getLessPage();

	ExtendedBlock= pmflash->getLessBlock() % 0x200;

	CE 	= pmflash->getChipSelectNum();
	CH 	= pmflash->getChannelNum();
	Entry= pmflash->getEntryItemNum();

	PseudoBlockStart = Entry*8 -0x200 + ExtendedBlock;  // = Total_Block - 0x200 + LessBlock, LessBlock Must Exist

	// ----- Get Internal Chip Number & Address -----
	BYTE	InternalChip = 1;
	UINT	BasicBlock;
	UINT	Lun1End, Lun2Start, Lun2End, Lun3Start; // 2 Internal Chip Use, Lun1Start = 0
	UINT	Lun3End, Lun4Start, Lun4End, Lun5Start; // 4 Internal Chip Use

	if((BaseFID[2] & 0x03) != 0x00) // Multi-Internal Chip Exist
		{
			InternalChip = (0x01 << (BaseFID[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }
			BasicBlock = 0x200<<((pmflash->getFlashModel(5)&0xF0)>>4);;
			Lun1End = BasicBlock+ ExtendedBlock - 1;
			Lun2Start = 2*BasicBlock;
			Lun2End = 3*BasicBlock+ ExtendedBlock - 1;
			Lun3Start = 4*BasicBlock;
			Lun3End = 5*BasicBlock+ ExtendedBlock - 1;
			Lun4Start = 6*BasicBlock;
			Lun4End = 7*BasicBlock+ ExtendedBlock - 1;
			Lun5Start = 8*BasicBlock;
		}

	for(ChipIndex=0; ChipIndex<(CE*CH); ChipIndex++)
	{
		ChipAddress=(((ChipIndex/CH)<<4)|(ChipIndex%CH))<<24;
		for(BlockIndex=0; BlockIndex<(Entry*8); BlockIndex++)
		{

			//if(isTerminate==true) //terminate the thread and go back to start state
			//	goto Terminate_Erase_Handle;
			// ----- Skip Dummy Block -----
			if((InternalChip == 2) && (ExtendedBlock != 0x00))// Sherlock_20120425, Only Support 2 Internal Chip
			{
				if(((Lun1End<BlockIndex)&&(BlockIndex<Lun2Start)) || ((Lun2End<BlockIndex)&&(BlockIndex<Lun3Start)))
						continue;
			}
			else if((InternalChip == 4) && (ExtendedBlock != 0x00)) // Sherlock_20120425, Only Support 4 Internal Chip
			{
				if(	((Lun1End<BlockIndex)&&(BlockIndex<Lun2Start)) || ((Lun2End<BlockIndex)&&(BlockIndex<Lun3Start)) ||
					((Lun3End<BlockIndex)&&(BlockIndex<Lun4Start)) || ((Lun4End<BlockIndex)&&(BlockIndex<Lun5Start))	)
					continue;
			}

			if((ExtendedBlock != 0x00) && (InternalChip == 1))	// Extended_Block_Exist && Not Internal_Chip_Flash
			{
					if(BlockIndex >= PseudoBlockStart) // It Is Pseudo Block
					 continue;
			}

			RetryCnt = 0;
			Address=ChipAddress|(BlockIndex*BlockPage);
	ReTryBlockErase:

			Status= pmflash->EraseBlock(Address, &EraseStatus);//erase block
			if((!Status) || (EraseStatus & BIT0))
			{
				if(++RetryCnt < 3)
				{
						usleep(10000);
					goto ReTryBlockErase;
				}
				pmflash->MarkBad(Address, PageSEC,LessPage);//mark bad
				setMapEntryItem((ChipIndex/CH), (ChipIndex%CH), (BlockIndex/8), (BYTE)(BlockIndex%8),1);

			}

		}
	}


	return Success_State;	//return Status;
//Terminate_Erase_Handle:
//	return ReStart_State;
}
MapChipSelect * CRootTable::AllocateBlockMapMemory(){
	UINT j,k;
	MapChipSelect *pUFDItem;
	BYTE ChipSelectNum = pmflash->getChipSelectNum();
	BYTE ChannelNum =  pmflash ->getChannelNum();
	UINT EntryItemNum = pmflash->getEntryItemNum();
	pUFDItem=new MapChipSelect();
	pUFDItem->ChipSelectNum=ChipSelectNum;
	pUFDItem->ChannelNum=ChannelNum;
	pUFDItem->EntryItemNum=EntryItemNum;
	 // Sherlock_20140730, Initialize
    memset(&pUFDItem->SysBlkAdr[0],0,sizeof(UINT)*3076);

	for(j=0;j<ChipSelectNum;j++)
	{
		pUFDItem->CEItem[j]=new MapChannel;
		for(k=0;k<ChannelNum;k++)
		{
			pUFDItem->BadBlockCnt[j*ChannelNum+k]=0;
			pUFDItem->CEItem[j]->ChannelItem[k]=(LPMapEntryItem)new MapEntryItem[EntryItemNum];
			memset(pUFDItem->CEItem[j]->ChannelItem[k], 0, EntryItemNum);

		}
	}

	return pUFDItem;

}


UINT CRootTable::UpdatePairMapByAddress(ULONG Address, USHORT BlockPage, BYTE PlaneNum, BYTE MLC, BYTE MaxECC)
{
	UINT 	PU_Idx, Byte_Idx, bit_Idx;
	BYTE	CellMap, ECCMap, ValueTmp;
	BOOL 	Status = false;

	PU_Idx = (Address & 0xF0000000) >> 28;					// eMMC Only 1 CH
	Address = (Address & 0x00FFFFFF)/BlockPage/PlaneNum;	// RAWAdr_BlockAdr_PairAdr
	Byte_Idx = Address / 8;
	bit_Idx	= Address % 8;
	ValueTmp = 0x01<<bit_Idx;

	if(Byte_Idx>512)
		return false;


	Status = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, 0x1FF85000+PU_Idx*512+Byte_Idx, 1, &CellMap);

	if(MLC)
		CellMap |= ValueTmp;
	else
		CellMap &= (~ValueTmp);

	Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x1FF85000+PU_Idx*512+Byte_Idx, 1, &CellMap);

	Status = pmflash->AccessMemoryRead(MI_READ_DATA, 0, 0, 0x1FF85800+PU_Idx*512+Byte_Idx, 1, &ECCMap);

	if(MaxECC)
		ECCMap |= ValueTmp;
	else
		ECCMap &= (~ValueTmp);

	Status = pmflash->AccessMemoryWrite(MI_WRITE_DATA, 0, 0, 0x1FF85800+PU_Idx*512+Byte_Idx, 1, &ECCMap);

	return Status;
}
