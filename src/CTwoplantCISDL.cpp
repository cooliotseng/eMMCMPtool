/*
 * CTwoplantCISDL.cpp
 *
 *  Created on: Apr 29, 2015
 *      Author: vli
 */

#include "CTwoplantCISDL.h"

extern ULONG FileSize(FILE *fp);
CTwoplantCISDL::CTwoplantCISDL(ICISTool *pCisTool) {
	// TODO Auto-generated constructor stub
	pmCisTool = pCisTool;

}

CTwoplantCISDL::~CTwoplantCISDL() {
	// TODO Auto-generated destructor stub
}

UINT CTwoplantCISDL::Execute(SettingConfgInfo *pCurSettingConfgInfo,
		CFlash			*pflash,
		CRootTable		*pRootTable,
		eMMC_CIS_INFO *pCISInfo,
		UINT 			*eCISADDR,
		UINT			*Original_EraseCnt,
		UINT 			OldCISVersionExit,
		UINT			Terminate){
	UINT	Status=Success_State, BitErrorCnt=0, ErrorBitLimt;
	eMMC_CIS_INFO	eCISSetData, eCISBackData; //eCISPairMap; Cody_20150216
	BYTE	ECC,  EraseStatus=0, ValueTmp=0, RetryCnt;
	WORD	*pWORD, CheckSum=0;
	SPARETYPE Spare;
	ULONG	Address;
	int		i, j, CISBlockNum, pln;
	USHORT	TurboPage[256], TurboPage_NUM;

	FILE	*BLBinFile,*ISPBinFile;


	if(pflash->getPlaneNum() == 1)
		CISBlockNum = 2; // if 1 Plane, write 2 CIS Block
	else
		CISBlockNum = 4; // if 2 Plane, Write 4 CIS Block


	BYTE	SpareBuf[6]={0,0,0,0,0,0};


	// ----- ISP_Step 1/7: Prepare FW Data -----Must Get This Here for CIS Data

	ULONG	ISPRealByte, ISPTotalByte, EachTxSize, Each2PTxSize, ISPSizeWithoutBL; //Cody_20150305  Add ISPSizeWithoutBL
	ULONG	ISPIdx, PageAddr, BufOffset, BLBinRealSize, BootLoaderSize;
	DWORD	dwBytesRead;
	BYTE	*TxDataBuf, *RxDataBuf;

	if(pflash->getPageSEC() == 32)
		EachTxSize = 12*1024;	// 16K_Flash, Each Page Write 12K
	else
		EachTxSize = 6*1024;	//   8K_Flash, Each Page Write 6K

	Each2PTxSize = 2* EachTxSize;	 // For 2_Plane ISP Use
	BootLoaderSize = 3 * EachTxSize; //For New Bin, First 36K is BootLoader

	if(sizeof(eMMC_CIS_INFO)%EachTxSize){ // Protection
		Status=Illegal_CIS_Define;
		return Status;
	}

	// ----- Get BootLoader Bin File Start -----
	BLBinFile = fopen((char *)pCurSettingConfgInfo->BLBinFileName.c_str(),"r");

	if(BLBinFile==NULL){
		Status=Open_FW_File_Error;
		return Status;
	}

	BLBinRealSize	 = FileSize(BLBinFile); //	Get Real BootLoader Size
	if(BLBinRealSize == 0){
			Status=Open_FW_File_Error;
			return Status;
	}

	if(BLBinRealSize != BootLoaderSize){
		Status=Open_FW_File_Error;
		fclose(BLBinFile);
		return Status;
	}
	// ----- Get BootLoader Bin File End -----
	ISPBinFile = fopen((char *)pCurSettingConfgInfo->FWFileName.c_str(),"r");
			// no attr. template
	if(ISPBinFile==NULL){
		Status=Open_FW_File_Error;
		return Status;
	}
	ISPRealByte = FileSize(ISPBinFile); //	Get Real ISP Size

	if(ISPRealByte == 0){
		Status=Open_FW_File_Error;
		return Status;
	}


	// --- New Update TotalByte Size, Sherlock_20150107---
	if((ISPRealByte%Each2PTxSize) != 0){
		ISPTotalByte = ((ISPRealByte/Each2PTxSize)+1) * Each2PTxSize;
	}
	else{
		ISPTotalByte = ISPRealByte;
	}

	ISPSizeWithoutBL = ISPTotalByte;  //Cody_20150305 Be used in RTPageIdxOfs
	ISPTotalByte += BootLoaderSize;		// Add BootLoader Parts in TotalSize

	TxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
	RxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
	memset(TxDataBuf, 0, sizeof(BYTE)*ISPTotalByte);
	dwBytesRead=fread(TxDataBuf,sizeof(char),BootLoaderSize,BLBinFile);
	dwBytesRead=fread(TxDataBuf[BootLoaderSize],sizeof(char),ISPRealByte,ISPBinFile);

// Sherlock_20140814, Put Here For "GOTO"
	BYTE	*RTDataBuf, *LastRTDataBuf;
	BYTE	BackUpRTNum = 0, Num;						// RT Numbers
	ULONG	RTPageIdxOfs, EachRTSize = 14*1024;			// RT Size in One Page is 14K
	RTDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachRTSize*2);	// 1 Set of RT is 2*14K Cody_20150306
	memset(RTDataBuf, 0xFF, sizeof(BYTE)*EachRTSize*2);        //Cody_20150306
	LastRTDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachRTSize*2);	// 1 Set of RT is 2*14K Cody_20150306 for the last RT
	memset(LastRTDataBuf, 0xFF, sizeof(BYTE)*EachRTSize*2);        //Cody_20150306
	RTPageIdxOfs = (sizeof(eMMC_CIS_INFO)/EachTxSize) + (BootLoaderSize/EachTxSize)+2*(ISPSizeWithoutBL/Each2PTxSize); // Normally is 15 //Cody_20150305  RTPageIdxOfs
	// ----- CIS_Step 1/4: Prepare CIS Data -----
	// Set eCIS Basic Data
// Sherlock_20140819, Modify For eMMC_CFG_A2Scan
	memset(&eCISSetData, 0, sizeof(eMMC_CIS_INFO));
	eCISSetData.CE_NUM = pflash->getChipSelectNum();
	eCISSetData.CH_NUM = pflash->getChannelNum();
	eCISSetData.FLH_DRV = pCurSettingConfgInfo->FlashDriving;
	eCISSetData.CTL_DRV = pCurSettingConfgInfo->ControllerDriving;
	eCISSetData.CPU_CLK = pCurSettingConfgInfo->CisData.Reserved2[4];
	eCISSetData.Encryption = pCurSettingConfgInfo->CisData.Reserved2[5];
	eCISSetData.PlaneBlock = pflash->getPlaneBlock();
	eCISSetData.FW_PAGE = (BYTE)(ISPTotalByte/EachTxSize); // This Code Must Be After BIN File Has Read
	eCISSetData.CIS_PAGE = (BYTE)(sizeof(eMMC_CIS_INFO)/EachTxSize);
	eCISSetData.CIS_Version = 0x02; // Sherlock_20140801, For A2CMD PairMaps
	eCISSetData.EACH_PAGE = (BYTE)(EachTxSize/1024);

	if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableReset)	//No Reset
		eCISSetData.PRE_LOAD_DATA = 0;	// If NO_RESET, 		PreLoad = 0
	else if(pCurSettingConfgInfo->LunTypeBitMap & 0x0F) // Sherlock_20140801, .FormatLunBitMap & 0x0F
		eCISSetData.PRE_LOAD_DATA = 1;	// If RESET & LUN, 		Preload = 1
	else
		eCISSetData.PRE_LOAD_DATA = 0;	// If RESET & NO_LUN, 	PreLoad = 0


//	eCISSetData.FLH_INFO = (pCisDataEx->FLH_FwScheme.Model6 & 0x0F); // The Bit Definition in DataBase & Register is different
	if(pflash->getBlockPage() == 64)
		ValueTmp = 0; //[1:0] = xx00b
	else if(pflash->getBlockPage() == 128)
		ValueTmp = 1; //[1:0] = xx01b
	else if(pflash->getBlockPage() == 256)
		ValueTmp = 2; //[1:0] = xx10b
	else if(pflash->getBlockPage() == 512)
		ValueTmp = 3; //[1:0] = xx11b

	if(pflash->getPageSEC() == 32)
		ValueTmp = (ValueTmp & (0xF3)) + 0; // 16K_Page, [3:2] = 00xxb
	else if(pflash->getPageSEC() == 8)
		ValueTmp = (ValueTmp & (0xF3)) + 4; //   4K_Page, [3:2] = 01xxb
	else if(pflash->getPageSEC() == 16)
		ValueTmp = (ValueTmp & (0xF3)) + 8; //   8K_Page, [3:2] = 10xxb

	eCISSetData.FLH_INFO = ValueTmp;

	eCISSetData.CIS_RowAddr[0] = eCISADDR[0];
	eCISSetData.CIS_RowAddr[1] = eCISADDR[1];
	eCISSetData.CIS_RowAddr[2] = eCISADDR[2];
	eCISSetData.CIS_RowAddr[3] = eCISADDR[3];
//	eCISSetData.CID[0] = ??

	// Copy Turbo Page
	eCISSetData.TP_NUM = TurboPage_NUM;

	for(i=0; i<(int)eCISSetData.TP_NUM; i++)
		eCISSetData.TurboPage[i] = TurboPage[i];

	// Set CSD & ExtCSD Data, Sherlock_20131122
	eCISSetData.CheckSumLen = (USHORT)offsetof(eMMC_CIS_INFO, Bit_Map[0]);
	memcpy(eCISSetData.CSD, Default_CSD, sizeof(Default_CSD));
	memcpy(eCISSetData.Ext_CSD, Default_ExtCSD, sizeof(Default_ExtCSD));
	// Get ExtCSD Data From File, Sherlock_20140620


	for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits For Current Read
		Status = pflash->writeCellMapFlashType(eCISADDR[i],1);
		Status = pflash->writeEccMapBitLength(eCISADDR[i],1);
	}
	for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
		Status = pRootTable->setCellMap(eCISADDR[i],pflash,1);
		Status = pRootTable->setEccMap(eCISADDR[i],pflash,1);
	}
	// Set eCIS BitMap Data

	pmCisTool->setBlockMaptoBitMap(pflash,pRootTable,pRootTable->getUFDBlockMap(),pCISInfo);
	// calc the checksum
	pWORD = (WORD *) &eCISSetData;
	for(i=1; i < (int)offsetof(eMMC_CIS_INFO, Bit_Map[0])/2; i++){
		CheckSum+=pWORD[i];
	}
	CheckSum=0x10000-CheckSum; //get CheckSum inverse
	eCISSetData.CheckSum = (USHORT)CheckSum;

	// ----- Prepare Other Data For Writing -----
	ErrorBitLimt = (pCurSettingConfgInfo->LunTypeBitMap2 & BitErrorRate);
	ECC = 2 + 0 + 0; // 60-bits ECC + EnableECC_Bit6 + EncryptionOff_Bit7

	Spare.SPARE0=0x43; //'C'
	Spare.SPARE1=0x53; //'S'
	Spare.SPARE2=0x00; //Initial Value
	Spare.SPARE3=0x00; //Initial Value
	Spare.SPARE4=0x99;
	Spare.SPARE5=0x99;

	// ----- Get CIS Data into CIS_SetData From Read CIS (Only_DL_FW Mode) -----
	if((pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)   ||
	   (((pRootTable[0].getRootTableVersion() & 0xFFFF0000) != 0x52540000)  &&
	   (OldCISVersionExit == Success_State))){ //Not Change CIS, Only DL FW, Use OLD CIS Data, Sherlock_20131015 ,Special Function of Keep old CIS data For RE-MP ,Cody_20150217

		for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++){
			BufOffset = ISPIdx * EachTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx]; // PageAddr = (ULONG)ISPIdx*2;
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)	{
				Status=pflash->readBlockData(  (BYTE)(EachTxSize/512),
												ECC,
												Spare,
												eCISADDR[0]+PageAddr,
												EachTxSize,
												(BYTE *)&eCISSetData+BufOffset);
				if(Status)
					break;
			}
			if(!Status){
				Status=EEPROM_UpLoad_Error;
				goto Endof_eCIS_Download_2P;
			}
			// calc the checksum
			CheckSum=0;
			pWORD = (WORD *) &eCISSetData;
			for(i=1; i < (int)offsetof(eMMC_CIS_INFO, Bit_Map[0])/2; i++){
				CheckSum+=pWORD[i];
			}
			CheckSum=0x10000-CheckSum; //get CheckSum inverse
			if(eCISSetData.CheckSum != (USHORT)CheckSum){
				Status = CIS_CheckSum_Error;
				goto Endof_eCIS_Download_2P;
			}
		}

		for(i=0; i<4; i++) // Sherlock_20131106, For Only_DL_FW Case, We Get CIS Address From CIS Set Data
			eCISADDR[i] = eCISSetData.CIS_RowAddr[i];



// Sherlock_20131106, Get EraseCount For Only_DL_FW Mode
		for(i=0; i<4; i++)
		{
			if(eCISADDR[i] == 0xFFFFFFFF)
				continue;	// Protection

			pflash->ReadSpareData(0, 0, eCISADDR[i], 6, SpareBuf);
			// For eCIS. EncryptionCMD is 0

			// ---------- New Get Spare ---------- Sherlock_20140415
			USHORT Shift1 = SpareBuf[1]&0x3F;	// Clear Bit7 and Bit6
			if( ((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) || // 'CS'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x52)) ||	// 'RT'
				((Shift1<<8 | SpareBuf[0]) == (0x48<<7 | 0x43)) ||	// 'CH'
				((Shift1<<8 | SpareBuf[0]) == (0x4B<<7 | 0x4D)) ||	// 'MK'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x44)) ||	// 'DT'
				((Shift1<<8 | SpareBuf[0]) == (0x55<<7 | 0x44)) ||	// 'DU'
				((SpareBuf[0] == 0x4D) && (SpareBuf[1] == 0x50)) )	// 'MP' This Means "Erased By MPTool"
				Original_EraseCnt[i] = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
			else
				Original_EraseCnt[i] = 0;

		}

	}

// Sherlock_20140814, Read RT from First CIS Block
	if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)
	{
		for(Num=0; Num<100; Num++){
			PageAddr = (ULONG)eCISSetData.TurboPage[RTPageIdxOfs+Num]; // PageAddr = (ULONG)RTPageIdxOfs*2+Num*2;
			Status=pflash->MultiPageRead(	(BYTE)(EachRTSize*2/512),
											ECC, // Encryption OFF
											Spare,
											eCISADDR[0]+PageAddr,
											EachRTSize*2,
											(BYTE *)RTDataBuf);


				j = (EachRTSize/2);	// First RT(7K) Position

				if( (RTDataBuf[j-1]!=0x52) ||(RTDataBuf[j-2]!=0x54) ) // Check 1st RT for 'R' & 'T'
				{
					break;
				}
				memcpy(LastRTDataBuf,RTDataBuf,sizeof(BYTE)*EachRTSize*2);


		}
		BackUpRTNum = Num;
	}

	// ----- Dump eCIS Important Data -----
	// ========== Whole Write CIS Blocks Cycle ==========
	for(i=0; i<CISBlockNum; i=i+2){
		// ===== PART A: Write eCIS Info into Each Blocks =====
		for(pln =0; pln<2; pln++){
			Address = eCISADDR[i+pln]; // Only Block Address Different
// Sherlock_20131106, Set New EraseCount
			Spare.SPARE3 = (BYTE)(((Original_EraseCnt[i]+1) & 0x0000FF00) >> 8);	// HI
			Spare.SPARE2 = (BYTE)((Original_EraseCnt[i]+1) & 0x000000FF);		// LO


			if(Address == 0xFFFFFFFF)	continue;	// Protection

			// ----- Erase Block -----
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
				Status= pflash->EraseBlock(Address, &EraseStatus);
				if(Status)
					break;
			}

			if(!Status)
			{
				Status = Block_Erase_Error;
				goto Endof_eCIS_Download_2P;
			}

			// ----- CIS_Step 2/4: Write eCIS Cycle -----
			for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
			{
				BufOffset = ISPIdx * EachTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx];
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=pflash->writeBlockData((BYTE)(EachTxSize/512),
													ECC,
													Spare,
													Address+PageAddr,
													EachTxSize,
													(BYTE *)&eCISSetData+BufOffset);
					if(Status)
						break;
				}

//				Sleep(20); // 20121114, For Multi-Device MP

				if(!Status)
				{
					Status=EEPROM_DownLoad_Error;
					goto Endof_eCIS_Download_2P;
				}

			}

			// ----- CIS_Step 3/4: Read eCIS Cycle -----
			memset(&eCISBackData, 0, sizeof(eMMC_CIS_INFO));
			for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
			{
				BufOffset = ISPIdx * EachTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx];
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=pflash->readBlockData(	(BYTE)(EachTxSize/512),
													ECC,
													Spare,
													Address+PageAddr,
													EachTxSize,
													(BYTE *)&eCISBackData+BufOffset);
					if(Status)
						break;
				}

//				Sleep(20); // 20121114, For Multi-Device MP

				if(!Status)
				{
					Status=EEPROM_UpLoad_Error;
					goto Endof_eCIS_Download_2P;
				}

			}

			// ----- CIS_Step 4/4: Compare eCIS Info -----
			for(j=0; j<sizeof(eMMC_CIS_INFO); j++)
			{
				if((j%1024)==0)
					BitErrorCnt=0;

				ValueTmp=((BYTE *)&eCISSetData)[j]^((BYTE *)&eCISBackData)[j];

				if(ValueTmp)
					BitErrorCnt+=BitCount(ValueTmp);

				if(BitErrorCnt > ErrorBitLimt)
				{

					// ----- Debug Message, Sherlock_20121130 -----
					int	Start, Cnt;

					if(j > 15)
						Start = j - 15;
					else
						Start = 0;



					Status=EEPROM_Compare_Error;
					goto Endof_eCIS_Download_2P;
				}
			}
		}
		// ===== PART A: End =====

		// ===== PART B: Write BootLoaders into Each Blocks =====
		for(pln =0; pln<2; pln++)
		{
			Address = eCISADDR[i+pln]; // Only Block Address Different

			// ----- BLD_Step 1/3: Write BLD -----
			for(ISPIdx=0; ISPIdx<(BootLoaderSize/EachTxSize); ISPIdx++) // BootLoader Length is 3 * 12K
			{
				BufOffset = ISPIdx * EachTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize)]; // Skip eCIS Pages
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=pflash->writeBlockData(	(BYTE)(EachTxSize/512),
													ECC,
													Spare,
													Address+PageAddr,
													EachTxSize,
													TxDataBuf+BufOffset);
					if(Status)
						break;
				}

//				Sleep(20); // 20121114, For Multi-Device MP

				if(!Status)
				{
					Status=FW_DownLoad_Error;
					goto Endof_eCIS_Download_2P;
				}

			}

			// ----- BLD_Step 2/3: Read BLD -----
			for(ISPIdx=0; ISPIdx<(BootLoaderSize/EachTxSize); ISPIdx++)
			{
				BufOffset = ISPIdx * EachTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize)]; // Skip eCIS Pages
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=pflash->readBlockData(	(BYTE)(EachTxSize/512),
													ECC,
													Spare,
													Address+PageAddr,
													EachTxSize,
													RxDataBuf+BufOffset);
					if(Status)
						break;
				}

				if(!Status)
				{
					Status=FW_UpLoad_Error;
					goto Endof_eCIS_Download_2P;
				}

			}

			// ----- BLD_Step 3/3: Compare BLD  -----
			for(BufOffset=0; BufOffset<BootLoaderSize; BufOffset++)
			{
				if((BufOffset%1024)==0)
					BitErrorCnt=0;

				ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

				if(ValueTmp)
					BitErrorCnt+=BitCount(ValueTmp);

				if(BitErrorCnt > ErrorBitLimt)
				{


					Status=FW_Compare_Error;
					goto Endof_eCIS_Download_2P;
				}
			}
		}
		// ===== PART B: End =====

		// ===== PART C: Write 2 FW Into Planes =====
		Address = eCISADDR[i];

		// ----- ISP_StICISDL *pmCISDL;ep 2/7: Write FW_1 -----
		for(ISPIdx=0; ISPIdx<((ISPTotalByte-BootLoaderSize)/Each2PTxSize); ISPIdx++)
		{
//			BufOffset = ISPIdx * Each2PTxSize;
			BufOffset = BootLoaderSize + ISPIdx * Each2PTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (BootLoaderSize/EachTxSize)]; // Skip eCIS & BLD Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
//				Status=pVLIBaseAPICtrl2->ReadWriteBlock(HandleList,
				Status=pflash->MultiPageWrite(	(BYTE)(Each2PTxSize/512),
												ECC,
												Spare,
												Address+PageAddr,
												Each2PTxSize,
												TxDataBuf+BufOffset);
				if(Status)
					break;
			}

			if(!Status)
			{
				Status=FW_DownLoad_Error;
				goto Endof_eCIS_Download_2P;
			}

		}

		// ----- ISP_Step 3/7: Read FW_1 -----
		for(ISPIdx=0; ISPIdx<((ISPTotalByte-BootLoaderSize)/Each2PTxSize); ISPIdx++)
		{
//			BufOffset = ISPIdx * Each2PTxSize;
			BufOffset = BootLoaderSize + ISPIdx * Each2PTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (BootLoaderSize/EachTxSize)]; // Skip eCIS & BLD Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
//				Status=pVLIBaseAPICtrl2->ReadWriteBlock(HandleList,
				Status=pflash->MultiPageRead(	(BYTE)(Each2PTxSize/512),
												ECC,
												Spare,
												Address+PageAddr,
												Each2PTxSize,
												RxDataBuf+BufOffset);
				if(Status)
					break;
			}

			if(!Status)
			{
				Status=FW_UpLoad_Error;
				goto Endof_eCIS_Download_2P;
			}

		}

		// ----- ISP_Step 4/7: Compare FW 1 -----
		for(BufOffset=BootLoaderSize; BufOffset<ISPTotalByte; BufOffset++)
		{
			if((BufOffset%1024)==0)
				BitErrorCnt=0;

			ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

			if(ValueTmp)
				BitErrorCnt+=BitCount(ValueTmp);

			if(BitErrorCnt > ErrorBitLimt)
			{
				Status=FW_Compare_Error;
				goto Endof_eCIS_Download_2P;
			}
		}

		// ----- ISP_Step 5/7: Write FW_2 -----
		for(ISPIdx=0; ISPIdx<((ISPTotalByte-BootLoaderSize)/Each2PTxSize); ISPIdx++)
		{
//			BufOffset = ISPIdx * Each2PTxSize;
			BufOffset = BootLoaderSize + ISPIdx * Each2PTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (BootLoaderSize/EachTxSize) + ((ISPTotalByte-BootLoaderSize)/Each2PTxSize)]; // Skip eCIS & BLD & FW1 Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
//				Status=pVLIBaseAPICtrl2->ReadWriteBlock(HandleList,
				Status=pflash->MultiPageWrite(	(BYTE)(Each2PTxSize/512),
												ECC,
												Spare,
												Address+PageAddr,
												Each2PTxSize,
												TxDataBuf+BufOffset);
				if(Status)
					break;
			}


			if(!Status)
			{
				Status=FW_DownLoad_Error;
				goto Endof_eCIS_Download_2P;
			}

		}

		// ----- ISP_Step 6/7: Read FW_2 -----
		for(ISPIdx=0; ISPIdx<((ISPTotalByte-BootLoaderSize)/Each2PTxSize); ISPIdx++)
		{
//			BufOffset = ISPIdx * Each2PTxSize;
			BufOffset = BootLoaderSize + ISPIdx * Each2PTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (BootLoaderSize/EachTxSize) + ((ISPTotalByte-BootLoaderSize)/Each2PTxSize)]; // Skip eCIS & BLD & FW1 Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
//				Status=pVLIBaseAPICtrl2->ReadWriteBlock(HandleList,
				Status=pflash->MultiPageRead(	(BYTE)(Each2PTxSize/512),
												ECC,
												Spare,
												Address+PageAddr,
												Each2PTxSize,
												RxDataBuf+BufOffset);

				if(Status)
					break;
			}


			if(!Status)
			{
				Status=FW_UpLoad_Error;
				goto Endof_eCIS_Download_2P;
			}

		}

		// ----- ISP_Step 7/7: Compare FW_2 -----
		for(BufOffset=BootLoaderSize; BufOffset<ISPTotalByte; BufOffset++)
		{
			if((BufOffset%1024)==0)
				BitErrorCnt=0;

			ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

			if(ValueTmp)
				BitErrorCnt+=BitCount(ValueTmp);

			if(BitErrorCnt > ErrorBitLimt)
			{

				Status=FW_Compare_Error;
				goto Endof_eCIS_Download_2P;
			}
		}

		// ===== PART D: Write Num BackUpRT Info =====
// Sherlock_20140814, Write RT into Each CIS Block
		if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)
		{
			for(Num=0; Num<1; Num++)
			{
				//BufOffset = Num * EachRTSize*2;
				PageAddr = (ULONG)eCISSetData.TurboPage[RTPageIdxOfs]; // PageAddr = (ULONG)RTPageIdxOfs*2+Num*2;
				Status=pflash->MultiPageWrite(	(BYTE)(EachRTSize*2/512),
												ECC, // Encryption Off
												Spare,
												Address+PageAddr,
												EachRTSize*2,
												(BYTE *)LastRTDataBuf);
				if(!Status) // Write Fail
					break;
			}
		}


	}

Endof_eCIS_Download_2P:
	fclose(BLBinFile);
	fclose(ISPBinFile);
	free(TxDataBuf);
	free(RxDataBuf);
	free(RTDataBuf);
	free(LastRTDataBuf);
	return Status;
}

UINT CTwoplantCISDL::MultiPageCMDSet(CFlash *flash){

	UINT	Status = Fail_State;//, TempStatus, WaitCount = 0;;
		BYTE * buffer=new BYTE[64];
		UINT BlockPage = flash->getBlockPage();
		UINT PageSize = flash->getPageSize();
		VendorCMD	VCMD;
		memset(&VCMD, 0x00, sizeof(VendorCMD));

		VCMD.OPCode = 0xEE;
		VCMD.MJCMD = 0x51;
		VCMD.MICMD = 0x82;
		VCMD.BufLen = sizeof(ReadCMD_TSB_2P);

		memcpy(buffer, ReadCMD_TSB_2P, sizeof(ReadCMD_TSB_2P));
		if(PageSize == 16*1024)		buffer[0] = 0x00;
		else if(PageSize == 8*1024)	buffer[0] = 0x08;

		if(BlockPage == 64)			buffer[0] |= 0x00;
		else if(BlockPage == 128)		buffer[0] |= 0x01;
		else if(BlockPage == 256)		buffer[0] |= 0x02;
		else if(BlockPage == 512)		buffer[0] |= 0x03;

		Status = flash->SetInfoWriteCMD(0, 0, VCMD, buffer);

		VCMD.MICMD = 0x83;
		VCMD.BufLen = sizeof(WriteCMD_TSB_2P);

		memcpy(buffer, WriteCMD_TSB_2P, sizeof(WriteCMD_TSB_2P));

		if(PageSize == 16*1024)		buffer[0] = 0x00;
		else if(PageSize == 8*1024)	buffer[0] = 0x08;

		if(BlockPage == 64)			buffer[0] |= 0x00;
		else if(BlockPage == 128)		buffer[0] |= 0x01;
		else if(BlockPage == 256)		buffer[0] |= 0x02;
		else if(BlockPage == 512)		buffer[0] |= 0x03;

		Status = flash->SetInfoWriteCMD(0, 0, VCMD, buffer);
		delete buffer;
		return Status;


}
