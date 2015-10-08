/*
 * CCISDLD0.cpp
 *
 *  Created on: Sep 15, 2015
 *      Author: vli
 */

#include "CCISDLD0.h"
#include <stdio.h>
#include <unistd.h>

extern ULONG FileSize(FILE *fp);
CCISDLD0::CCISDLD0(ICISTool * pcistool) {
	// TODO Auto-generated constructor stub
	pmCisTool = pcistool;
}

CCISDLD0::~CCISDLD0() {
	// TODO Auto-generated destructor stub
}

UINT CCISDLD0::Execute(	SettingConfgInfo *pCurSettingConfgInfo,
						CFlash			*pflash,
						CRootTable		*pRootTable,
						eMMC_CIS_INFO 	*pCISInfo,
						UINT 			*eCISADDR,
						UINT			*Original_EraseCnt,
						UINT			Terminate){
	    BYTE	TxDataBuf_Array[270336], RxDataBuf_Array[270336];
		UINT	Status=Success_State, BitErrorCnt=0, ErrorBitLimt;
	//	TotalCIS_INFO	CISSetData;//, CISSetDataTmp;
		eMMC_CIS_INFO	eCISSetData, eCISBackData; //eCISPairMap; Cody_20150216
		BYTE	ECC,  EraseStatus=0, ValueTmp=0, RetryCnt;
		WORD	*pWORD, CheckSum=0;
		SPARETYPE Spare;
		ULONG	Address;
		int		i, j, CISBlockNum, pln,index;
		USHORT	TurboPage[256], TurboPage_NUM, TurboPage_Type;
		BYTE	CheckSumStatus = false;
		MapChipSelect *ptUFDBlockMap;
		Status = pflash->setMultiPageAccress();

		UINT 	OldCISVersionExit = pflash->isOldVersionCISExit();

		if(pCurSettingConfgInfo->PlaneNum == 1)
			CISBlockNum = 2; // if 1 Plane, write 2 CIS Block
		else
			CISBlockNum = 4; // if 2 Plane, Write 4 CIS Block


		BYTE	EncryptionCMD=0, SpareBuf[6]={0,0,0,0,0,0};

		if(((pflash->getBaseFType() & FT_Vendor) == FT_Toshiba) || ((pflash->getBaseFType() & FT_Vendor) == FT_Samsung))
			EncryptionCMD = 1;

		// ----- ISP_Step 1/7: Prepare FW Data -----Must Get This Here for CIS Data

		FILE	*ISPBinFile;
		ULONG	ISPRealByte, ISP0TotalByte, ISP1TotalByte, ISPTotalByte, EachTxSize, Each2PTxSize;
		ULONG	ISPIdx, PageAddr, BufOffset;
		DWORD	dwBytesRead;
		BYTE	*TxDataBuf, *RxDataBuf;


		if(pflash->getPageSEC() == 32)
			EachTxSize = 12*1024;	// 16K_Flash, Each Page Write 12K
		else
			EachTxSize = 6*1024;	//   8K_Flash, Each Page Write 6K

		Each2PTxSize = 2* EachTxSize;	 // For 2_Plane ISP Use

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

		// --- New Update TotalByte Size, Sherlock_20150107---
		if((ISPRealByte%EachTxSize) != 0) // If Not Multiple of 6K, Modify It As 6K*N

			ISP0TotalByte = ((ISPRealByte/EachTxSize)+1) * EachTxSize;

		else
			ISP0TotalByte = ISPRealByte;

		// ---
		if(((ISPRealByte-EachTxSize)%Each2PTxSize) != 0)//FW_1 without DRAM

			ISP1TotalByte = (((ISPRealByte-EachTxSize)/Each2PTxSize)+1) * Each2PTxSize;

		else
			ISP1TotalByte = (ISPRealByte-EachTxSize); //FW_1 without DRAM


		ISPTotalByte = ISP0TotalByte + ISP1TotalByte;
		TxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
		RxDataBuf=(BYTE *)malloc(sizeof(BYTE)*ISPTotalByte);
		memset(TxDataBuf, 0, sizeof(BYTE)*ISPTotalByte);
		fread(TxDataBuf,sizeof(char),ISPRealByte,ISPBinFile);
		fseek(ISPBinFile, EachTxSize,SEEK_SET);//FW_1 without DRAM
		fread(&TxDataBuf[ISP0TotalByte],sizeof(char),ISPRealByte-EachTxSize,ISPBinFile);//FW_1 without DRAM
 // Sherlock_20140814, Put Here For "GOTO"
		BYTE	*RTDataBuf, *LastRTDataBuf;
		BYTE	BackUpRTNum = 0, Num;						// RT Numbers
		ULONG	RTPageIdxOfs, EachRTSize = 14*1024;			// RT Size in One Page is 14K
		RTDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachRTSize*2);	// 1 Set of RT is 2*14K Cody_20150306
		memset(RTDataBuf, 0xFF, sizeof(BYTE)*EachRTSize*2);        //Cody_20150306
		LastRTDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachRTSize*2);	// 1 Set of RT is 2*14K Cody_20150306 for the last RT
		memset(LastRTDataBuf, 0xFF, sizeof(BYTE)*EachRTSize*2);        //Cody_20150306
		RTPageIdxOfs = (sizeof(eMMC_CIS_INFO)/EachTxSize) + (ISP0TotalByte/EachTxSize) +(ISP1TotalByte/Each2PTxSize); // Normally is 3+2n

		// ----- CIS_Step 1/4: Prepare CIS Data -----
		// Set eCIS Basic Data
		// Sherlock_20140819, Modify For eMMC_CFG_A2Scan

		memset(&eCISSetData, 0, sizeof(eMMC_CIS_INFO));
		Status = SetSLCPageCIS( pflash, &TurboPage[0], &TurboPage_NUM, &TurboPage_Type);
		eCISSetData.PAGE_MODE = 1;
/*
		if(pCurSettingConfgInfo->TestProcedureMask & Proc_SLCPageMode) //Cody_20150422
		{
			Status = SetSLCPageCIS( pCurSettingConfgInfo, &TurboPage[0], &TurboPage_NUM, &TurboPage_Type);
			eCISSetData.PAGE_MODE = 1;
		}
		else
		{
			Status = SetTPMTeCIS(pCurSettingConfgInfo, &TurboPage[0], &TurboPage_NUM, &TurboPage_Type);
			eCISSetData.PAGE_MODE = 0;
		}
*/
		eCISSetData.CE_NUM = pflash->getChipSelectNum();
		eCISSetData.CH_NUM = pflash->getChannelNum();
		eCISSetData.FLH_DRV = pCurSettingConfgInfo->FlashDriving;
		eCISSetData.CTL_DRV = pCurSettingConfgInfo->ControllerDriving;
		eCISSetData.CPU_CLK = pCurSettingConfgInfo->CisData.Reserved2[4];
		eCISSetData.Encryption = pCurSettingConfgInfo->CisData.Reserved2[5];
		eCISSetData.PlaneBlock = pflash->getPlaneBlock();
		eCISSetData.FW_PAGE = (BYTE)(ISP0TotalByte/EachTxSize); // This Code Must Be After BIN File Has Read  //Cody_20150416
		eCISSetData.CIS_PAGE = (BYTE)(sizeof(eMMC_CIS_INFO)/EachTxSize);
	//	eCISSetData.FW2_PAGE = (BYTE)(ISP1TotalByte/Each2PTxSize); //Cody_20150417
		eCISSetData.CIS_Version = 0x02; // Sherlock_20140801, For A2CMD PairMaps
		eCISSetData.EACH_PAGE = (BYTE)(EachTxSize/1024);

		if(pCurSettingConfgInfo->LunTypeBitMap & NormalFW)
			eCISSetData.PreMP_MODE = 0;
		else if(pCurSettingConfgInfo->LunTypeBitMap & PreMPFW)
			eCISSetData.PreMP_MODE = 2;
		else if(pCurSettingConfgInfo->LunTypeBitMap & PreMPFWWait1Sec)
			eCISSetData.PreMP_MODE = 1;

		if(pCurSettingConfgInfo->LunTypeBitMap & SortingFW)
			eCISSetData.Sorting_MODE = 1;
		else
			eCISSetData.Sorting_MODE = 0;

		if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableReset)	//No Reset
			eCISSetData.PRE_LOAD_DATA = 0;	// If NO_RESET, 		PreLoad = 0
		else if(pCurSettingConfgInfo->LunTypeBitMap & 0x0F) // Sherlock_20140801, .FormatLunBitMap & 0x0F
			eCISSetData.PRE_LOAD_DATA = 1;	// If RESET & LUN, 		Preload = 1
		else
			eCISSetData.PRE_LOAD_DATA = 0;	// If RESET & NO_LUN, 	PreLoad = 0



	//	eCISSetData.FLH_INFO = (pCisDataEx->FLH_FwScheme.Model6 & 0x0F); // The Bit Definition in DataBase & Register is different
		if(pflash->getBlockPage() == 64)			ValueTmp = 0; //[1:0] = xx00b
		else if(pflash->getBlockPage() == 128)	ValueTmp = 1; //[1:0] = xx01b
		else if(pflash->getBlockPage() == 256)	ValueTmp = 2; //[1:0] = xx10b
		else if(pflash->getBlockPage() == 512)	ValueTmp = 3; //[1:0] = xx11b

		if(pflash->getPageSEC() == 32)			ValueTmp = (ValueTmp & (0xF3)) + 0; // 16K_Page, [3:2] = 00xxb
		else if(pflash->getPageSEC() == 8)		ValueTmp = (ValueTmp & (0xF3)) + 4; //   4K_Page, [3:2] = 01xxb
		else if(pflash->getPageSEC() == 16)		ValueTmp = (ValueTmp & (0xF3)) + 8; //   8K_Page, [3:2] = 10xxb

		eCISSetData.FLH_INFO = ValueTmp;

		for(i=0; i<pflash->getChipSelectNum() ; i++)
		{
			eCISSetData.TotalMlcEraCnt[i] = pRootTable->getRootTable()[i].DWL.dwTotalMlcEraCnt;        //Cody_20150410
			eCISSetData.TotalSlcEraCnt[i] = pRootTable->getRootTable()[i].DWL.dwTotalSlcEraCnt;
		}

		eCISSetData.CIS_RowAddr[0] = eCISADDR[0];
		eCISSetData.CIS_RowAddr[1] = eCISADDR[1];
		eCISSetData.CIS_RowAddr[2] = eCISADDR[2];
		eCISSetData.CIS_RowAddr[3] = eCISADDR[3];

		// Copy Turbo Page
		eCISSetData.TP_NUM = TurboPage_NUM;

		for(i=0; i<(int)eCISSetData.TP_NUM; i++)
			eCISSetData.TurboPage[i] = TurboPage[i];

		// Set CSD & ExtCSD Data, Sherlock_20131122
		eCISSetData.CheckSumLen = (USHORT)offsetof(eMMC_CIS_INFO, Bit_Map[0]);
		memcpy(eCISSetData.CSD, Default_CSD, sizeof(Default_CSD));
		memcpy(eCISSetData.Ext_CSD, Default_ExtCSD, sizeof(Default_ExtCSD));
		// Get ExtCSD Data From File, Sherlock_20140620
		//eMMC_Get_CID_From_File(&eCISSetData);

		if(pCurSettingConfgInfo->TestProcedureMask & Proc_SLCPageMode)
		{
			for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
				Status = pRootTable->setCellMap(pCISInfo->CIS_RowAddr[i],pflash,0);
				Status = pRootTable->setEccMap(pCISInfo->CIS_RowAddr[i],pflash,1);
			}
			for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
				Status = pRootTable->setCellMap(pCISInfo->CIS_RowAddr[i],pflash,0);
				Status = pRootTable->setEccMap(pCISInfo->CIS_RowAddr[i],pflash,1);
			}
		}
		else
		{
			for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
				Status = pRootTable->setCellMap(pCISInfo->CIS_RowAddr[i],pflash,1);
				Status = pRootTable->setEccMap(pCISInfo->CIS_RowAddr[i],pflash,1);
			}

			for(i=0; i<CISBlockNum; i++){ // Sherlock_20140815, Set CIS Block As MLC, 60-bits into PairMaps For FW Read
				Status = pRootTable->setCellMap(pCISInfo->CIS_RowAddr[i],pflash,1);
				Status = pRootTable->setEccMap(pCISInfo->CIS_RowAddr[i],pflash,1);
			}
		}



		ptUFDBlockMap = pRootTable->getUFDBlockMap();
		// Set eCIS BitMap Data
		Status = pmCisTool->setBlockMaptoBitMap(pflash,pRootTable,ptUFDBlockMap,pCISInfo);
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


		//ECC = 2 + 0 + BIT7 + BIT5; // 60-bits ECC + EnableECC_Bit6 + EncryptionOn_Bit7 + SlcMode_Bit5
		ECC = 2 + 0 + BIT7; // 60-bits ECC + EnableECC_Bit6 + EncryptionOn_Bit7
		if(pCurSettingConfgInfo->TestProcedureMask & Proc_SLCPageMode)
			ECC = ECC + BIT5; //SlcMode_Bit5


		Spare.SPARE0=0x43; //'C'
		Spare.SPARE1=0x53; //'S'
		Spare.SPARE2=0x00; //Initial Value
		Spare.SPARE3=0x00; //Initial Value
		Spare.SPARE4=0x99;
		Spare.SPARE5=0x99;

		// ----- Get CIS Data into CIS_SetData From Read CIS (Only_DL_FW Mode) -----
		if((pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)   ||
		   (((pRootTable->getRootTable()[0].dwRTBLVersion & 0xFFFF0000) != 0x52540000)  &&
		   (OldCISVersionExit == Success_State))) //Not Change CIS, Only DL FW, Use OLD CIS Data, Sherlock_20131015 ,Special Function of Keep old CIS data For RE-MP ,Cody_20150217
		{

			for(index=0;index<4;index++)
			{
				for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
				{
					BufOffset = ISPIdx * EachTxSize;
					PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx]; // PageAddr = (ULONG)ISPIdx*2;
					for(RetryCnt=0; RetryCnt<3; RetryCnt++)
					{
						Status=pflash->readBlockData(	(BYTE)(EachTxSize/512),
														ECC,
														Spare,
														eCISADDR[index]+PageAddr,
														EachTxSize,
														(BYTE *)&eCISSetData+BufOffset);
						if(Status)
							break;
					}

					if(!Status)
					{
						Status=EEPROM_UpLoad_Error;
						goto Endof_eCIS_Download_2P_D0;
					}

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
						if(index == 3)
						{
							Status = CIS_CheckSum_Error;
							goto Endof_eCIS_Download_2P_D0;
						}
					}
					else
					{
						CheckSumStatus = true;
					}
				}

				if(CheckSumStatus)
							break;
			}


			for(i=0; i<4; i++) // Sherlock_20131106, For Only_DL_FW Case, We Get CIS Address From CIS Set Data
				eCISADDR[i] = eCISSetData.CIS_RowAddr[i];

	// Sherlock_20131106, Get EraseCount For Only_DL_FW Mode
			BYTE SpareEcc;

			if(pflash->getCisBlkPgeMode())
				SpareEcc = 0xA0;
			else
				SpareEcc = 0x80;

			for(i=0; i<4; i++)
			{
				if(eCISADDR[i] == 0xFFFFFFFF)	continue;	// Protection

				pflash->ReadSpareData(SpareEcc, 0, eCISADDR[i], 6, SpareBuf);
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
			for(Num=0; Num<100; Num++)
			{
				//BufOffset = Num * EachRTSize*2;
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

		// ========== Whole Write CIS Blocks Cycle ==========
		for(i=0; i<CISBlockNum; i=i+2)
		{
			// ===== PART A: Write eCIS Info into Each Blocks =====
			for(pln =0; pln<2; pln++)
			{
				Address = eCISADDR[i+pln]; // Only Block Address Different
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
					goto Endof_eCIS_Download_2P_D0;
				}

				// ----- CIS_Step 2/4: Write eCIS Cycle -----
				for(ISPIdx=0; ISPIdx<(sizeof(eMMC_CIS_INFO)/EachTxSize); ISPIdx++)
				{
					BufOffset = ISPIdx * EachTxSize;
					PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx];
					for(RetryCnt=0; RetryCnt<3; RetryCnt++)
					{
						Status=pflash->writeBlockData( (BYTE)(EachTxSize/512),
														ECC,
														Spare,
														Address+PageAddr,
														EachTxSize,
														(BYTE *)&eCISSetData+BufOffset);
						if(Status)
							break;
					}

					if(!Status)
					{
						Status=EEPROM_DownLoad_Error;
						goto Endof_eCIS_Download_2P_D0;
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

					if(!Status)
					{
						Status=EEPROM_UpLoad_Error;
						goto Endof_eCIS_Download_2P_D0;
					}

				}

				// ----- CIS_Step 4/4: Compare eCIS Info -----
				for(j=0; j<sizeof(eMMC_CIS_INFO); j++)
				{
					if((j%1024)==0)
						BitErrorCnt=0;

					ValueTmp=((BYTE*)&eCISSetData)[j]^((BYTE *)&eCISBackData)[j];

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
						goto Endof_eCIS_Download_2P_D0;
					}
				}
			}
			// ===== PART A: End =====
           memcpy(TxDataBuf_Array,TxDataBuf,sizeof(BYTE)*270336);
			// ===== PART B: Write Firmware Image 0 into Each Blocks =====
			for(pln =0; pln<2; pln++)
			{
				Address = eCISADDR[i+pln]; // Only Block Address Different
				// ----- FW0_Step 1/3: Write FW 0 -----
				for(ISPIdx=0; ISPIdx<(ISP0TotalByte/EachTxSize); ISPIdx++) // FW_0
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

					if(!Status)
					{
						Status=FW_DownLoad_Error;
						goto Endof_eCIS_Download_2P_D0;
					}

				}

				// ----- FW0_Step 2/3: Read FW 0 -----
				for(ISPIdx=0; ISPIdx<(ISP0TotalByte/EachTxSize); ISPIdx++)
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
						goto Endof_eCIS_Download_2P_D0;
					}

				}
				 memcpy(RxDataBuf_Array,RxDataBuf,sizeof(BYTE)*270336);
				// ----- FW0_Step 3/3: Compare FW 0  -----
				for(BufOffset=0; BufOffset<ISP0TotalByte; BufOffset++)
				{
					if((BufOffset%1024)==0)
						BitErrorCnt=0;

					ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

					if(ValueTmp)
						BitErrorCnt+=BitCount(ValueTmp);

					if(BitErrorCnt > ErrorBitLimt)
					{
						Status=FW_Compare_Error;
						goto Endof_eCIS_Download_2P_D0;
					}
				}
			}
			// ===== PART B: End =====

			// ===== PART C: Write Firmware Image 1 with 2 Plane Read/Write =====
			Address = eCISADDR[i];

			// ----- FW1_Step 2/7: Write FW 1 -----
			for(ISPIdx=0; ISPIdx<((ISPTotalByte-ISP0TotalByte)/Each2PTxSize); ISPIdx++)
			{
	//			BufOffset = ISPIdx * Each2PTxSize;
				BufOffset = ISP0TotalByte+ ISPIdx * Each2PTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (ISP0TotalByte/EachTxSize)]; // Skip eCIS & FW_0 Pages
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
					goto Endof_eCIS_Download_2P_D0;
				}

			}

			// ----- FW1_Step 3/7: Read FW 1 -----
			for(ISPIdx=0; ISPIdx<((ISPTotalByte-ISP0TotalByte)/Each2PTxSize); ISPIdx++)
			{
				BufOffset = ISP0TotalByte+ ISPIdx * Each2PTxSize;
				PageAddr = (ULONG)eCISSetData.TurboPage[ISPIdx + (sizeof(eMMC_CIS_INFO)/EachTxSize) + (ISP0TotalByte/EachTxSize)]; // Skip eCIS & FW_0 Pages
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=Status=pflash->MultiPageRead(	(BYTE)(Each2PTxSize/512),
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
					goto Endof_eCIS_Download_2P_D0;
				}

			}

			// ----- FW1_Step 4/7: Compare FW 1 -----
			for(BufOffset=ISP0TotalByte; BufOffset<ISPTotalByte; BufOffset++)
			{
				if((BufOffset%1024)==0)
					BitErrorCnt=0;

				ValueTmp=TxDataBuf[BufOffset]^RxDataBuf[BufOffset];

				if(ValueTmp)
					BitErrorCnt+=BitCount(ValueTmp);

				if(BitErrorCnt > ErrorBitLimt)
				{
					Status=FW_Compare_Error;
					goto Endof_eCIS_Download_2P_D0;
				}
			}

			// ===== PART D: Write Num BackUpRT Info =====
// Sherlock_20140814, Write RT into Each CIS Block
			if(pCurSettingConfgInfo->TestProcedureMask & Proc_DisableCISDL)
			{
				for(Num=0; Num<1; Num++)
				{

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

	Endof_eCIS_Download_2P_D0:
		fclose(ISPBinFile);
		free(TxDataBuf);
		free(RxDataBuf);
		free(RTDataBuf);
		free(LastRTDataBuf);
		return Status;
	}

//-----------------------------------------------------------------------------
//==================================================================
// 	SLC mode Page List
//	Type_1: {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, ...... 252, 254}, Length = BlockPage/2
//	Type_2: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, ...... 126, 127...}, Length = BlockPage/2
//
//==================================================================
// Sherlock_20140819, Modify For eMMC_CFG_A2Scan
UINT CCISDLD0::SetSLCPageCIS(CFlash *pflash, USHORT *TurboPage, USHORT *TurboPage_NUM, USHORT *TurboPage_Type)  //Cody_20150420
{
	USHORT     Index, TPMTLen;
	BYTE	Type = 1; // Default Type
	UINT 	BaseFID[8] = {0};

	pflash->getFlashID(BaseFID);

	if((BaseFID[0]== 0x2C) || (BaseFID[0] == 0x89))
		Type = 2;	// Micron/Intel MLC //For D0 test SLC mode
	else if((BaseFID[0]== 0x98) || (BaseFID[0] == 0xEC) || (BaseFID[0] == 0x45))
		Type = 1; 	// Toshiba/Samsung/SanDisk MLC
	else if(BaseFID[0]== 0xAD)
		Type =  2; //Cody_20150413  Hynix for FW test  For D0 test SLC mode
	else
		Type = 2; 	// Default  Cody_20150413  Hynix for FW test  For D0 test SLC mode


	if(Type == 1) // for Derek test  {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, ...... 252, 254}, Length = 128  Cody_20150410
	{

		TPMTLen = pflash->getBlockPage() / 2;
		TurboPage[0] = 0;
		for(Index=1; Index<TPMTLen; Index++)
		{
			TurboPage[Index] = (USHORT)(2*Index ) ;
		}
	}
	else if(Type == 2) // M73 Flash, Sherlock_20130111  and Hynix Flash Cody_20150413
	{
		TPMTLen = pflash->getBlockPage() / 2; //Cody_20150413
		for(Index=0; Index<TPMTLen; Index++)
		{
			TurboPage[Index] = (USHORT)Index;
		}
	}

	// ----- General Setting for All NAND Flash -----
	for(Index=TPMTLen; Index<256; Index++)				// Set Other Data as 0xFF
		TurboPage[Index] = (USHORT)0xFFFF;

	*TurboPage_NUM = TPMTLen;
	*TurboPage_Type = Type;
	return 1;
}
