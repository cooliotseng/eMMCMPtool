/*
 * CCISDL.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#include "CCISDL.h"
#include <unistd.h>

extern ULONG FileSize(FILE *fp);
CCISDL::CCISDL(ICISTool * pcistool) {
	// TODO Auto-generated constructor stub
	pmCisTool = pcistool;
}

CCISDL::~CCISDL() {
	// TODO Auto-generated destructor stub
}

UINT CCISDL::Execute(	SettingConfgInfo *pCurSettingConfgInfo,
						CFlash			*pflash,
						CRootTable		*pRootTable,
						eMMC_CIS_INFO *pCISInfo,
						UINT 			*eCISADDR,
						UINT			*Original_EraseCnt,
						UINT 			OldCISVersionExit,
						UINT			Terminate){

	UINT	Status=Success_State, BitErrorCnt=0, ErrorBitLimt;
	eMMC_CIS_INFO	eCISSetData, eCISBackData; //eCISPairMap; Cody_20150216
	BYTE	ECC, EraseStatus=0, ValueTmp=0, RetryCnt;
	WORD	*pWORD, CheckSum=0;
	SPARETYPE Spare;
	ULONG	Address;
	UINT    i,j,CISBlockNum;
	FILE	*ISPBinFile;
	ULONG	ISPRealByte, ISPTotalByte, EachTxSize;
	ULONG	ISPIdx, PageAddr, BufOffset;
	BYTE	*TxDataBuf, *RxDataBuf;
	BYTE	*RTDataBuf;
	BYTE	BackUpRTNum = 0, Num;						// RT Numbers
	ULONG	RTPageIdxOfs, EachRTSize = 14*1024;			// RT Size in One Page is 14K

	if(pflash->getPlaneNum() == 1)
		CISBlockNum = 2; // if 1 Plane, write 2 CIS Block
	else
		CISBlockNum = 4; // if 2 Plane, Write 4 CIS Block



	BYTE	SpareBuf[6]={0,0,0,0,0,0};

	// ----- ISP_Step 1/7: Prepare ISP Data -----Must Get This Here for CIS Data


	if(pflash->getPageSEC() == 32)
		EachTxSize = 12*1024;	// 16K_Flash, Each Page Write 12K
	else
		EachTxSize = 6*1024;	//   8K_Flash, Each Page Write 6K

	if(sizeof(eMMC_CIS_INFO)%EachTxSize) // Protection
	{
		Status=Illegal_CIS_Define;
		return Status;
	}

	if(pCurSettingConfgInfo->FWFileName.length() == 0)
	{
		Status=Open_FW_File_Error;
		return Status;
	}

	ISPBinFile = fopen((char *)pCurSettingConfgInfo->FWFileName.c_str(),"r");					// no attr. template

	if(ISPBinFile==NULL)
	{
		Status=Open_FW_File_Error;
		return Status;
	}


	ISPRealByte = FileSize(ISPBinFile); //	Get Real ISP Size
	if((ISPRealByte%EachTxSize) != 0) // If Not Multiple of 6K, Modify It As 6K*N
	{
		ISPTotalByte = ((ISPRealByte/EachTxSize)+1) * EachTxSize;

	}
	else
		ISPTotalByte = ISPRealByte;

	TxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
	RxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
	memset(TxDataBuf, 0, sizeof(BYTE)*ISPTotalByte);
	fread(&TxDataBuf,sizeof(char),ISPRealByte,ISPBinFile);
	RTDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachRTSize*10);	// 1 Set of RT is 2*14K, Backup 5 Set of RT
	memset(RTDataBuf, 0xFF, sizeof(BYTE)*EachRTSize*10);
	RTPageIdxOfs = (sizeof(eMMC_CIS_INFO)/EachTxSize) + (ISPTotalByte/EachTxSize); // Normally is 15


// ----- CIS_Step 1/4: Prepare CIS Data -----
// Set eCIS Basic Data
// Sherlock_20140819, Modify For eMMC_CFG_A2Scan

// Get ExtCSD Data From File, Sherlock_20140620
//eMMC_Get_CID_From_File(pThreadInfo, &eCISSetData);


	for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits For Current Read
		Status = pflash->writeCellMapFlashType(pCISInfo->CIS_RowAddr[i],1);
		Status = pflash->writeEccMapBitLength(pCISInfo->CIS_RowAddr[i],1);
	}
	for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
		Status = pRootTable->setCellMap(pCISInfo->CIS_RowAddr[i],pflash,1);
		Status = pRootTable->setEccMap(pCISInfo->CIS_RowAddr[i],pflash,1);
	}
	// Set eCIS BitMap Data
	pmCisTool->setBlockMaptoBitMap(pflash,pRootTable,pRootTable->getUFDBlockMap(),pCISInfo);

	// calc the checksum
	pWORD = (WORD *) &eCISSetData;
	for(i=1; i < (int)offsetof(eMMC_CIS_INFO, Bit_Map[0])/2; i++)
	{
		CheckSum+=pWORD[i];
	}
	CheckSum=0x10000-CheckSum; //get CheckSum inverse
	eCISSetData.CheckSum = (USHORT)CheckSum;

	// ----- Prepare Other Data For Writing -----
	ErrorBitLimt = (pCurSettingConfgInfo->LunTypeBitMap2 & BitErrorRate);
	ECC = pflash->getFlashModel(3) & 0x0F;
	Spare.SPARE0=0x43; //'C'
	Spare.SPARE1=0x53; //'S'
	Spare.SPARE2=0x00; //Initial Value
	Spare.SPARE3=0x00; //Initial Value
	Spare.SPARE4=0x99;
	Spare.SPARE5=0x99;

	// ----- Get CIS Data into CIS_SetData From Read CIS (Only_DL_FW Mode) -----
	if((pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)   ||
	   (((pRootTable[0].getRootTableVersion() & 0xFFFF0000) != 0x52540000)  &&
	   (OldCISVersionExit == Success_State))) //Not Change CIS, Only DL FW, Use OLD CIS Data, Sherlock_20131015  ,Special Function of Keep old CIS data For RE-MP ,Cody_20150217
	{

		for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
		{
			BufOffset = ISPIdx * EachTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx];
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
				Status=pflash->readBlockData((BYTE)(EachTxSize/512),
											ECC,
											Spare,
											eCISADDR[0]+PageAddr,
											EachTxSize,
											(BYTE *)&eCISSetData+BufOffset);
				if(Status)
					break;
			}
			;
			if(!Status)
			{
				Status=EEPROM_UpLoad_Error;
				goto Endof_eCIS_Download;
			}

//			OutputDataBuffer(sizeof(eMMC_CIS_INFO),(LPBYTE)&eCISSetData);

			// calc the checksum
			CheckSum=0;
			pWORD = (WORD *) &eCISSetData;
			for(i=1; i < (int)offsetof(eMMC_CIS_INFO, Bit_Map[0])/2; i++)
			{
				CheckSum+=pWORD[i];
			}
			CheckSum=0x10000-CheckSum; //get CheckSum inverse
			if(eCISSetData.CheckSum != (USHORT)CheckSum)
			{
				Status = CIS_CheckSum_Error;
				goto Endof_eCIS_Download;
			}
		}

		for(i=0; i<4; i++) // Sherlock_20131106, For Only_DL_FW Case, We Get CIS Address From CIS Set Data
			eCISADDR[i] = eCISSetData.CIS_RowAddr[i];

 // Sherlock_20131106, Get EraseCount For Only_DL_FW Mode
		for(i=0; i<4; i++)
		{
			if(eCISADDR[i] == 0xFFFFFFFF)	continue;	// Protection

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
		for(Num=0; Num<10; Num++)
		{
			BufOffset = Num * EachRTSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[RTPageIdxOfs+Num];
			Status=pflash->readBlockData((BYTE)(EachRTSize/512),
										ECC+BIT7, // Encryption ON
										Spare,
										eCISADDR[0]+PageAddr,
										EachRTSize,
										(BYTE *)RTDataBuf+BufOffset);

			if((Num%2) == 0)	// Check While Even Index, One Set Of RT is 28K
			{
				j = BufOffset+(EachRTSize/2);	// First RT(7K) Position
				if( (RTDataBuf[j-1]!=0x52) ||(RTDataBuf[j-2]!=0x54) ) // Check 1st RT for 'R' & 'T'
				{
					break;
				}
			}
		}
		BackUpRTNum = Num;
	}




	// ----- Dump eCIS Important Data -----

	// ---------- Whole Write CIS Block Cycle ----------
	for(i=0; i<CISBlockNum; i++)
	{
		Address = eCISADDR[i];
// Sherlock_20131106, Set New EraseCount
		Spare.SPARE3 = (BYTE)(((Original_EraseCnt[i]+1) & 0x0000FF00) >> 8);	// HI
		Spare.SPARE2 = (BYTE)((Original_EraseCnt[i]+1) & 0x000000FF);		// LO


		if(Address == 0xFFFFFFFF)	continue;	// Protection

		// ----- Erase Block -----
		for(RetryCnt=0; RetryCnt<3; RetryCnt++)
		{
			Status=pflash->EraseBlock(Address, &EraseStatus);
			if(Status)
				break;
		}

		if(!Status)
		{
			Status = Block_Erase_Error;
			goto Endof_eCIS_Download;
		}

		// ----- CIS_Step 2/4: Write eCIS Cycle-----
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

			usleep(20000); // 20121114, For Multi-Device MP

			if(!Status)
			{
				Status=EEPROM_DownLoad_Error;
				goto Endof_eCIS_Download;
			}

		}

		// ----- CIS_Step 3/4: Read eCIS Cycle-----
		memset(&eCISBackData, 0, sizeof(eMMC_CIS_INFO));
		for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
		{
			BufOffset = ISPIdx * EachTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx];
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
				Status=pflash->readBlockData((BYTE)(EachTxSize/512),
											 	ECC,
												Spare,
												Address+PageAddr,
												EachTxSize,
												(BYTE *)&eCISBackData+BufOffset);
				if(Status)
					break;
			}

			usleep(20000); // 20121114, For Multi-Device MP

			if(!Status)
			{
				Status=EEPROM_UpLoad_Error;
				goto Endof_eCIS_Download;
			}

		}


		if(Terminate==true) //terminate the thread and go back to start state
			goto Terminate_eCIS_DownLoad_Handle;

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
				Status=EEPROM_Compare_Error;
				goto Endof_eCIS_Download;
			}
		}

		// ----- ISP_Step 2/7: Write ISP 1 -----
		for(ISPIdx=0; ISPIdx<(ISPTotalByte/EachTxSize); ISPIdx++)
		{
			BufOffset = ISPIdx * EachTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize)]; // Skip eCIS Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
				Status=pflash->writeBlockData((BYTE)(EachTxSize/512),
												ECC,
												Spare,
												Address+PageAddr,
												EachTxSize,
												TxDataBuf+BufOffset);
				if(Status)
					break;
			}
			usleep(20000); // 20121114, For Multi-Device MP

			if(!Status)
			{
				Status=FW_DownLoad_Error;
				goto Endof_eCIS_Download;
			}

		}

 // Sherlock_20140814, Write RT into Each CIS Block
		if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)
		{
			for(Num=0; Num<BackUpRTNum; Num++)
			{
				BufOffset = Num * EachRTSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[RTPageIdxOfs+Num];
				Status=pflash->writeBlockData((BYTE)(EachRTSize/512),
												ECC+BIT7, // Encryption ON
												Spare,
												Address+PageAddr,
												EachRTSize,
														(BYTE *)RTDataBuf+BufOffset);
				if(!Status) // Write Fail
					break;
			}
		}


		// ----- ISP_Step 3/7: Read ISP 1 -----
		for(ISPIdx=0; ISPIdx<(ISPTotalByte/EachTxSize); ISPIdx++)
		{
			BufOffset = ISPIdx * EachTxSize;
			PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize)]; // Skip eCIS Pages
			for(RetryCnt=0; RetryCnt<3; RetryCnt++)
			{
				Status=pflash->readBlockData((BYTE)(EachTxSize/512),
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
				goto Endof_eCIS_Download;
			}

		}

		// ----- ISP_Step 4/7: Compare ISP 1 -----
		for(BufOffset=0; BufOffset<ISPTotalByte; BufOffset++)
		{
			if((BufOffset%1024)==0)
				BitErrorCnt=0;

			ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

			if(ValueTmp)
				BitErrorCnt+=BitCount(ValueTmp);

			if(BitErrorCnt > ErrorBitLimt)
			{

				Status=FW_Compare_Error;
				goto Endof_eCIS_Download;
			}
		}
	}

Endof_eCIS_Download:
	fclose(ISPBinFile);
	free(TxDataBuf);
	free(RxDataBuf);
	free(RTDataBuf);
	return Status;

Terminate_eCIS_DownLoad_Handle:
	fclose(ISPBinFile);
	free(TxDataBuf);
	free(RxDataBuf);
	free(RTDataBuf);
	Status=ReStart_State;
	return Status;
}

