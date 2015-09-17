/*
 * CFlash.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: vli
 */

#include "CFlash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#define FID_Chk_Len 		6
#define MaxChipSelect 		4
#define VT3468				0x3468	//Chip is VT3468
CFlash::CFlash() {
	// TODO Auto-generated constructor stub
	pmDriver = NULL;
	pmFlashStructure = NULL;
	//mTurboPageInfo = (TurboPageInfo *)malloc(sizeof(TurboPageInfo));
	//pmUFDBlockMap= NULL;
	moldversionCISflag = -1;
}



CFlash::CFlash(IeMMCDriver *pDriver,FlashStructure *tFlashStructure) {
	// TODO Auto-generated constructor stub
	UINT	Status = 0;
	pmDriver = pDriver;
	pmFlashStructure = new FlashStructure();
	moldversionCISflag = -1;
	memcpy(pmFlashStructure,tFlashStructure,sizeof(FlashStructure));

	//initFlashInfo();
	
	if(pmFlashFwScheme->InterleaveNO== 0)
		pmFlashFwScheme->InterleaveNO = pmFlashStructure->ForceCE;		// 0x122
	
	if(pmFlashFwScheme->VB_Block== 0)
		pmFlashFwScheme->VB_Block= pmFlashStructure->ForceCE * pmFlashStructure->ForceCH;	// 0x130
	
	if(pmFlashFwScheme->Select_VB== 0)
		pmFlashFwScheme->Select_VB= pmFlashStructure->ForceCH;			// 0x131
    
	mEntryItemNum = initEntryItemNum()/8;

	Status=pmDriver->enableMPFunction();
	Status=pmDriver->enableReadSpareArea();
}



CFlash::~CFlash() {
	// TODO Auto-generated destructor stub
}

void CFlash::initFlashInfo(){
	
	BYTE CE=0, CH=1;
	BYTE CE_Num=1;
	INT  Entry=0x200, i, j, PseudoBlock = 0;
	BYTE * buffer=new BYTE[64];
	BYTE * F_ID=new BYTE[8];
	UINT Status=0, PhysicalCapacity=0x200000;
	BYTE CheckChip=true;
	BYTE InternalChip = 1; // 3rd FID bit[1:0], Default is 1
	BYTE	Plane = pmFlashFwScheme->SelectPlane;
    //BaseChipID = VT3468;
	
	Status=pmDriver->ReadFlashID(CE, buffer); // Sherlock_20140430, Patch 3493 ROM Code Bug
	
	if(Status == 0)
	{	
		goto EndCheckFlashInfo;
	}
	
	memcpy(F_ID, buffer, 8); 
	for(j=16;j<64;j+=16)
	{
		if(memcmp(F_ID, buffer+j, FID_Chk_Len)==0)
			CH++;
		else
			break;
	}

	do{
		CE++;    
		if(CE>=MaxChipSelect)   
			break;
		Status=pmDriver->ReadFlashID(CE, buffer); //?CE?INDEX
		if(!Status)  
		{
			continue; 
		}
		for(j=0;j<CH;j++)
		{
			if(memcmp(F_ID, buffer+(j*16), FID_Chk_Len)!=0)
				break;
		}
		if(j==CH)
			CE_Num++;   
	}while(1);  
	CE = CE_Num;
	
	if(pmFlashStructure->ForceCE!=0)
		CE=pmFlashStructure->ForceCE;
	if(pmFlashStructure->ForceCH!=0)
		CH=pmFlashStructure->ForceCH;

	if( (pmFlashStructure->ForceCE==0) && (pmFlashStructure->ForceCH==0))
	{
		//Capacity
		PhysicalCapacity = PhysicalCapacity<<(pmFlashStructure->FlashFwScheme->Model5 &0x0F);
		PhysicalCapacity = PhysicalCapacity * (pmFlashStructure->ChipSelectNum) * (pmFlashStructure->ChannelNum);
		pmFlashStructure->FlashFwScheme->Capacity = PhysicalCapacity;
	}

	mChipSelectNum = CE;
	mChannelNum = CH;
	
	// ----- Get Flash Type -----

EndCheckFlashInfo:
	delete buffer;
	delete F_ID;
}



void CFlash:: setoldversionCISflag() {
	moldversionCISflag = 1;
}

void CFlash:: clearoldversionCISflag() {
	moldversionCISflag = 0;
}


BYTE CFlash:: getCHipVersion() {

	UINT	Status = 0;
	BYTE	Register0, Register1, Register2, Register3;
	Status = readData(0xFFE04A14, 1, &Register0);
	Status = readData(0xFFE04A15, 1, &Register1);
	Status = readData(0xFFE04A16, 1, &Register2);
	Status = readData(0xFFE04A17, 1, &Register3);
	if((Register0 == '0') && (Register1 == '1') && (Register2 == '1') && (Register3 == '6'))
	{
		return	0xA0;
	}
	Status = readData(0xFFE04858, 1, &Register0);
	Status = readData(0xFFE04859, 1, &Register1);
	Status = readData(0xFFE0485A, 1, &Register2);
	Status = readData(0xFFE0485B, 1, &Register3);

	if((Register0 == '0') && (Register1 == '4') && (Register2 == '2') && (Register3 == '8'))
	{
		return	0xB0;
	}

	if(Status)		return	0xC0;
	else			return	0xFF; // Protection

}

void CFlash::getFlashID(UINT* buf) {
	// TODO Auto-generated constructor stub
	memcpy(buf,pmFlashFwScheme->FLH_ID,8*sizeof(BYTE));
}

BYTE CFlash::getChipSelectNum() {
	// TODO Auto-generated constructor stub
	return mChipSelectNum;
}

BYTE CFlash::getChannelNum() {
	// TODO Auto-generated constructor stub
	return mChannelNum;
}

UINT CFlash::getEntryItemNum() {
	// TODO Auto-generated constructor stub
	return mEntryItemNum;
}


BYTE CFlash::getFlashModel(UINT moddelNo) {
	// TODO Auto-generated constructor stub

	switch (moddelNo)
	{
	case 0:
		return pmFlashFwScheme->Model0;
	case 1:
			return pmFlashFwScheme->Model1;
	case 2:
			return pmFlashFwScheme->Model2;
	case 3:
			return pmFlashFwScheme->Model3;
	case 4:
			return pmFlashFwScheme->Model4;
	case 5:
			return pmFlashFwScheme->Model5;
	case 6:
			return pmFlashFwScheme->Model6;
	case 7:
				return pmFlashFwScheme->Model7;
	default:
			return 0;

	}


}

IeMMCDriver* CFlash::getDriver() {
	// TODO Auto-generated constructor stub
	return pmDriver;
}

UINT CFlash::MarkBad(ULONG Address, BYTE PageSEC, USHORT LessPage) {
	// TODO Auto-generated constructor stub
	BYTE HIBYTE_LessPage1,LOBYTE_LessPage0;
	HIBYTE_LessPage1 = HIBYTE(LessPage);
	LOBYTE_LessPage0 = LOBYTE(LessPage);

	pmDriver->MarkBad(0, 0, Address, 0, PageSEC, HIBYTE_LessPage1, LOBYTE_LessPage0, NULL);

	cout << "CFlash::MarkBad" << endl;
	return 0;
}

Flash_FwScheme* CFlash::getFlashFwScheme() {
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme;
}

UINT CFlash::getPageSize() {
	// TODO Auto-generated constructor stub
	return mPageSize;
}

UINT CFlash::getBlockPage() {
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme->BlockPage;
}

UINT CFlash::getLessPage() {
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme->LessPage;
}

UINT CFlash::getLessBlock() {
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme->LessBlock;
}

BYTE CFlash::getPlaneNum() {
	// TODO Auto-generated constructor stub
	return mPlaneNum;
}

UINT CFlash::setFlashSize() {
	// TODO Auto-generated constructor stub
	UINT Status = Fail_State;
	ULONG	Reg_FlashAccess = 0x1FF82600;
	BYTE	Register2, Register3;

	Status = readData( Reg_FlashAccess, 1, &Register2);

	if(!Status)
		return Status;

	if(mBlockPage == 64)
		Register3 = (Register2 & (0xFC)) + 0; //(~(bit_1|bit_0))
	else if(mBlockPage == 128)
		Register3 = (Register2 & (0xFC)) + 1; // Set bit_0
	else if(mBlockPage == 256)
		Register3 = (Register2 & (0xFC)) + 2; // Set bit_1
	else if(mBlockPage == 512)
		Register3 = (Register2 & (0xFC)) + 3; // Set bit_1|bit_0
	else
		Register3 = Register2;				// No Match, No Change

	if(mPageSize == 16*1024)
		Register3 = (Register3 & (0xF3)) + 0; // [3:2] = 00b
	else if(mPageSize == 4*1024)
		Register3 = (Register3 & (0xF3)) + 4; // [3:2] = 01b
	else if(mPageSize == 8*1024)
		Register3 = (Register3 & (0xF3)) + 8; // [3:2] = 10b

	Status = writeData(Reg_FlashAccess, 1, &Register3);
	if(!Status)
		return Status;

	return Status;
}

WORD CFlash::getBaseFType(){
	// TODO Auto-generated constructor stub
	return mBaseFType;
}

USHORT CFlash::getPlaneBlock() {
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme->PlaneBlock;
}

TurboPageInfo* CFlash::getTurboPageInfo() {
	// TODO Auto-generated constructor stub
	return pmFlashStructure->turbopageinfo;
}

UINT CFlash:: DownloadVDRFw(char *FWFileName) {

	UINT	Status = Success_State;
	FILE	*FWBinFile;
	DWORD	dwBytesRead;
	BYTE 	Header[8], *FunctionTblBuf, *VDRCodeBuf;
	ULONG	VDRRealLen, VDRCodeLen=60*1024; // Always 60K
	UINT	FunctionTableCnt, FunctionTblLen[8], FunctionTblAdr[8], FunctionTblOfs[8], idx, Offset;


	if(FWFileName == NULL)
	{
		Status=Open_FW_File_Error;
		return Status;
	}

	FWBinFile = fopen(FWFileName,"r");					// no attr. template

	if(FWBinFile==NULL)
	{
		Status=Open_FW_File_Error;
		return Status;
	}

	// -- Get Function Table Count --
	dwBytesRead = fread(&Header,sizeof(char),4,FWBinFile);
	FunctionTableCnt = ((UINT)Header[3]<<24) | ((UINT)Header[2]<<16) | ((UINT)Header[1]<<8) | Header[0];

	// -- Get Function Table Info --
	Offset = 4;
	for(idx=0; idx<FunctionTableCnt; idx++)
	{
		fseek(FWBinFile, Offset, 0);
		dwBytesRead = fread(&Header,sizeof(char),8,FWBinFile);
		FunctionTblLen[idx] = ((UINT)Header[3]<<24) | ((UINT)Header[2]<<16) | ((UINT)Header[1]<<8) | Header[0];
		FunctionTblAdr[idx] = ((UINT)Header[7]<<24) | ((UINT)Header[6]<<16) | ((UINT)Header[5]<<8) | Header[4];
		FunctionTblOfs[idx] = Offset + 8;					// FT_Data[idx] Start Address
		Offset = FunctionTblOfs[idx] +FunctionTblLen[idx];	// FT_Header[idx+1] Start Address
	}

	// -- VDR Size Protection --

	VDRRealLen= FileSize(FWBinFile); //  Get Real Total VDR Size
	if(VDRRealLen != (Offset + VDRCodeLen))
	{
		Status=Open_FW_File_Error;
		fclose(FWBinFile);
		return Status;
	}

	// -- Get & DL_VDR Code -
	VDRCodeBuf = (BYTE *)malloc(sizeof(BYTE)*VDRCodeLen);
	fseek(FWBinFile, Offset, 0);
	dwBytesRead = fread(VDRCodeBuf,sizeof(char),(UINT)VDRCodeLen,FWBinFile);
	Status = INITISP((ULONG)0x20000000, (USHORT)VDRCodeLen, VDRCodeBuf);
	if(!Status)
		Status=INIT_VDR_FW_Error;


	UINT	RemainLen, TxLen, RegAdrOfs;

	// -- Get & DL_FTBL
	for(idx=0; idx<FunctionTableCnt; idx++)
	{

 		FunctionTblBuf = (BYTE *)malloc(sizeof(BYTE)*FunctionTblLen[idx]);
 		fseek(FWBinFile, FunctionTblOfs[idx], 0);
		dwBytesRead = fread(FunctionTblBuf,sizeof(char),FunctionTblLen[idx],FWBinFile);
		RemainLen = FunctionTblLen[idx];
		RegAdrOfs = 0;
		while(RemainLen != 0) //  For Length > 256 Bytes
		{
			if(RemainLen > 0x100)
				TxLen = 0x100;		// Write Reg Max 0x100 Bytes
			else
				TxLen = RemainLen;

			Status = writeData((ULONG)FunctionTblAdr[idx]+RegAdrOfs, (USHORT)TxLen, FunctionTblBuf+RegAdrOfs); // Write Reg
			RemainLen -= TxLen;
			RegAdrOfs += TxLen;
		}

		if(!Status)
			Status = Register_Write_Error;
		free(FunctionTblBuf);
	}

	free(VDRCodeBuf);
	fclose(FWBinFile);

	return Status;
}




UINT CFlash::resetEcc() {
	// TODO Auto-generated constructor stub
		UINT	TempSts = Fail_State;
		BYTE	Register, Cnt, WaitLimit = 50;
		BYTE 	ECC = pmFlashStructure->FlashFwScheme->Model3 & 0x03;

		// Set ECC 2608
		TempSts = readData(0x1FF82608, 1, &Register);
		Register = (Register & (0xFC)) + ECC; // 00b=24-bit, 01b=40-bit, 10b=60-bit
		TempSts = writeData(0x1FF82608, 1, &Register);

		// 264E|=0x70
		TempSts = readData(0x1FF8264E, 1, &Register);
		Register |= 0x70;
		TempSts = writeData(0x1FF8264E, 1, &Register);

		// 264F|=0x71
		TempSts = readData(0x1FF8264F, 1, &Register);
		Register |= 0x71;
		TempSts = writeData(0x1FF8264F, 1, &Register);

		// Wait 2105=0xF0
		for(Cnt=0; Cnt<WaitLimit; Cnt++)
		{
			TempSts = readData(0x1FF82105, 1, &Register);
			if((Register & 0x0F) == 0x00)
				break;
			usleep(100000);
		}

		if(Cnt == WaitLimit)
			return Fail_State;

		// 264E &=(0x70)
		TempSts = readData(0x1FF8264E, 1, &Register);
		Register &= (~0x70);
		TempSts = writeData(0x1FF8264E, 1, &Register);

		// 264F &=(0x70)
		TempSts = readData(0x1FF8264F, 1, &Register);
		Register &= (~0x70);
		TempSts = writeData(0x1FF8264F, 1, &Register);

		return TempSts;
}

UINT CFlash::setMultiPageAccress() {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;//, TempStatus, WaitCount = 0;;
		BYTE * buffer=new BYTE[64];
		BYTE ReadCMD_TSB_2P[27] = {0x02,0x1B,0x0,0x5,0x32,0x0,0x0,0x0,0x0,0x5,0x30,0x0,0x0,0x0,0x3F,0x0,0x5,0x5,0x2,0xE0,0x0,0x0,0x5,0x5,0x2,0xE0,0x0};
		BYTE WriteCMD_TSB_2P[14] = {0x02,0x1B,0x80,0x5,0x11,0x0,0x0,0x0,0x81,0x5,0x10,0x0,0x0,0x0};
		VendorCMD	VCMD;
		memset(&VCMD, 0x00, sizeof(VendorCMD));

		VCMD.OPCode = 0xEE;
		VCMD.MJCMD = 0x51;
		VCMD.MICMD = 0x82;
		VCMD.BufLen = sizeof(ReadCMD_TSB_2P);

		memcpy(buffer, ReadCMD_TSB_2P, sizeof(ReadCMD_TSB_2P));
		
		if(mPageSize == 16*1024)		
			buffer[0] = 0x00;
		else if(mPageSize == 8*1024)	
			buffer[0] = 0x08;

		if(mBlockPage == 64)			
			buffer[0] |= 0x00;
		else if(mBlockPage == 128)		
			buffer[0] |= 0x01;
		else if(mBlockPage == 256)		
			buffer[0] |= 0x02;
		else if(mBlockPage == 512)		
			buffer[0] |= 0x03;
		
		Status = pmDriver->sendSetCommand(VCMD, buffer);

		VCMD.MICMD = 0x83;
		VCMD.BufLen = sizeof(WriteCMD_TSB_2P);

		memcpy(buffer, WriteCMD_TSB_2P, sizeof(WriteCMD_TSB_2P));

		if(mPageSize == 16*1024)		buffer[0] = 0x00;
		else if(mPageSize == 8*1024)	buffer[0] = 0x08;

		if(mBlockPage == 64)			buffer[0] |= 0x00;
		else if(mBlockPage == 128)		buffer[0] |= 0x01;
		else if(mBlockPage == 256)		buffer[0] |= 0x02;
		else if(mBlockPage == 512)		buffer[0] |= 0x03;

		Status = pmDriver->sendSetCommand(VCMD, buffer);
		delete buffer;
		return Status;
}



UINT CFlash::getFlashType() {
	// TODO Auto-generated constructor stub
	return mBaseFType;
}

UINT CFlash::writeTPMT() {
	// TODO Auto-generated constructor stub
		UINT	Status = Fail_State;
		BYTE	DataH, DataL;
		UINT	Index;

		DataH = HIBYTE(pmFlashStructure->turbopageinfo->TurboPageNUM);
		DataL = LOBYTE(pmFlashStructure->turbopageinfo->TurboPageNUM);
		Status = writeData(0x1FF840A4, 1, &DataL); // Little End
		Status = writeData(0x1FF840A5, 1, &DataH);

		for(Index=0; Index<pmFlashStructure->turbopageinfo->TurboPageNUM; Index++)
		{
			DataH = HIBYTE(pmFlashStructure->turbopageinfo->TurboPage[Index]);
			DataL = LOBYTE(pmFlashStructure->turbopageinfo->TurboPage[Index]);
			Status = writeData(0x1FF846A0+2*Index, 1, &DataL); // Little End
			Status = writeData(0x1FF846A1+2*Index, 1, &DataH);
		}

		return Status;
}



UINT CFlash::writeCellMapFlashType(ULONG TableAddress,BYTE MLCType) {
		UINT 	Status = Fail_State, PU_Idx, Byte_Idx, bit_Idx;
		BYTE	CellMap, ECCMap, ValueTmp,Register;
		ULONG Address;
		Address =TableAddress;

		PU_Idx = (Address & 0xF0000000) >> 28;					// eMMC Only 1 CH
		Address = (Address & 0x00FFFFFF)/mBlockPage/mPlaneNum;	// RAWAdr_BlockAdr_PairAdr
		Byte_Idx = Address / 8;
		bit_Idx	= Address % 8;
		ValueTmp = 0x01<<bit_Idx;

		Register = 0; // Sherlock_20140515 Must Write This Before Read ROOT TABLE
		Status = writeData(0x1FF82004, 1, &Register);
		Register = BIT1|BIT0;
		Status = writeData( 0x1FF8300D, 1, &Register);

		if(Byte_Idx>512)
			return Fail_State;
		Status = readData( 0x1FF85000+PU_Idx*512+Byte_Idx, 1, &CellMap);
		if(MLCType)
			CellMap |= ValueTmp;
		else
			CellMap &= (~ValueTmp);
		Status = writeData(0x1FF85000+PU_Idx*512+Byte_Idx, 1, &CellMap);

		return Status;
}

UINT CFlash::writeEccMapBitLength(ULONG TableAddress,BYTE MaxECCBitLength) {
			UINT 	Status = Fail_State, PU_Idx, Byte_Idx, bit_Idx;
			BYTE	CellMap, ECCMap, ValueTmp,Register;
			ULONG Address;
			Address =TableAddress;

			PU_Idx = (Address & 0xF0000000) >> 28;					// eMMC Only 1 CH
			Address = (Address & 0x00FFFFFF)/mBlockPage/mPlaneNum;	// RAWAdr_BlockAdr_PairAdr
			Byte_Idx = Address / 8;
			bit_Idx	= Address % 8;
			ValueTmp = 0x01<<bit_Idx;

			Register = 0; // Sherlock_20140515 Must Write This Before Read ROOT TABLE
			Status = writeData(0x1FF82004, 1, &Register);
			Register = BIT1|BIT0;
			Status = writeData( 0x1FF8300D, 1, &Register);

			Status = readData(0x1FF85800+PU_Idx*512+Byte_Idx, 1, &ECCMap);

			if(MaxECCBitLength)
				ECCMap |= ValueTmp;
			else
				ECCMap &= (~ValueTmp);
			Status = writeData(0x1FF85800+PU_Idx*512+Byte_Idx, 1, &ECCMap);
}

UINT CFlash::isBlockEmpty(ULONG Address) {
	// TODO Auto-generated constructor stub
		UINT	Status = Fail_State;
		BYTE	EncryptionCMD = BIT1; // Check Block Is Erased, So Always Encryption_OFF
		BYTE	ValueTmp, BitErrorCnt = 0;
		BYTE	SpareBuf[6];
		int 	Index;
		Status = ReadSpareData(EncryptionCMD, 0, Address, 6, SpareBuf);

		for(Index=0; Index<2; Index++)
		{
			ValueTmp = SpareBuf[Index]^0xFF;

			if(ValueTmp)
				BitErrorCnt+=BitCount(ValueTmp);
		}

		// If Bit Error With 0xFF is Less Than 2, This is Erased Block
		if(BitErrorCnt < 2)		Status = Success_State;
		else					Status = Fail_State;

		return Status;
}

UINT CFlash::getEraseCount(ULONG Address, BYTE Encryption, BYTE ECC, USHORT *Value) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
		BYTE	SpareBuf[6] = {0,0,0,0,0,0}; //*pUSHORT2


		UINT	RdSize = 1024;
		SPARETYPE Spare;
		BYTE	*RxDataBuf, Register = BIT0|BIT1;
		RxDataBuf = (BYTE *)malloc(sizeof(BYTE)*RdSize);
		Spare.SPARE0=0x00;
		Spare.SPARE1=0x00;
		Spare.SPARE2=0x00;
		Spare.SPARE3=0x00;
		Spare.SPARE4=0x00;
		Spare.SPARE5=0x00;

		Status = writeData(0x1FF8300D, 1, &Register); // Patch_ROM_BUG
		Status = readBlockData((BYTE)(RdSize/512),
										(Encryption<<7)|ECC, //Encryption_Set + ECC_ON + ECC_Set
										Spare,
										Address,
										RdSize,
										RxDataBuf);

		Status = readData(0x300D0010, 6, SpareBuf); // Read Spare

				// ---------- New Get Spare ---------- Sherlock_20140415
		USHORT Shift1 = SpareBuf[1]&0x3F;	// Clear Bit7 and Bit6

		// Sherlock_20140421, Specail Patch For eCIS Block
		if(((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) && (Encryption == 1))	// 'CS' && Encryption_ON
		{
			//  Use Sector Mode Read
			Status = writeData(0x1FF8300D, 1, &Register); // Patch_ROM_BUG
			Status = readBlockData((BYTE)(RdSize/512),
											  ECC,  //Encryption_OFF + ECC_ON + ECC_Set
											  Spare,
											  Address,
											  RdSize,
											  RxDataBuf);
			Status = readData(0x300D0010, 6, SpareBuf); // Read Spare

			if(Status)
				*Value = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
			else
				*Value = 0;
		}
		else if(	((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) ||	// 'CS'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x52)) ||	// 'RT'
				((Shift1<<8 | SpareBuf[0]) == (0x48<<7 | 0x43)) ||	// 'CH'
				((Shift1<<8 | SpareBuf[0]) == (0x4B<<7 | 0x4D)) ||	// 'MK'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x44)) ||	// 'DT'
				((Shift1<<8 | SpareBuf[0]) == (0x55<<7 | 0x44)) ||	// 'DU'
				((SpareBuf[0] == 0x4D) && (SpareBuf[1] == 0x50)) )	// 'MP' This Means "Erased By MPTool"
		{
			*Value = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
		}
		else
		{
			*Value = 0;
		}

		//  Use Sector Mode Read
		free(RxDataBuf);

		return Status;
}

UINT CFlash::SortingWrite(BYTE PU_Index, USHORT BlockIndex, USHORT EraseCount, USHORT BlockPage) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
		VendorCMD	VCMD;
		memset(&VCMD, 0x00, sizeof(VendorCMD));

		VCMD.OPCode = 0xEE;
		VCMD.MJCMD = 0x51;
		VCMD.Address = (((ULONG)PU_Index) <<24) + BlockIndex;
		VCMD.MICMD = 0x02;
		VCMD.CFG[0] = HIBYTE(EraseCount);
		VCMD.CFG[1] = LOBYTE(EraseCount);
		VCMD.CFG[3] = HIBYTE(BlockPage);
		VCMD.CFG[4] = LOBYTE(BlockPage);

		Status = pmDriver->sendGetCommand(VCMD, NULL); // New CMD: EE, 02,03
		return Status;
}

UINT CFlash::SortingRead(BYTE PU_Index, USHORT BlockIndex, USHORT BlockPage) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
		BYTE	MaxECC = 0xCC;
		//DbgStr.Format(_T("- Sorting_Read_ECC (%d) -"), BlockIndex);	OutputDebugString(DbgStr);

		VendorCMD	VCMD;
		memset(&VCMD, 0x00, sizeof(VendorCMD));

		VCMD.OPCode = 0xEE;
		VCMD.MJCMD = 0x51;
		VCMD.Address = (((ULONG)PU_Index) <<24) + BlockIndex;
		VCMD.MICMD = 0x03;
		VCMD.BufLen = 1; // Unit is Byte
		VCMD.CFG[3] = HIBYTE(BlockPage);
		VCMD.CFG[4] = LOBYTE(BlockPage);

		Status = pmDriver->sendGetCommand(VCMD, &MaxECC); // New CMD: EE, 02,03
		if(!Status)	MaxECC = 0xCC;

		return MaxECC;
}
UINT CFlash::Sorting(bool Terminate,SettingConfgInfo *pCurSettingConfgInfo) {
	// TODO Auto-generated constructor stub
		UINT Status=Success_State;
		BYTE CE, CH, ChipIndex;
		UINT Entry, Address, ChipAddress=0, BlockIndex, PseudoBlockStart, TotalBlkNum=0, BadBlkNum=0;
		BYTE  EraseStatus=0x1, PageSEC, MaxECC;
		USHORT BlockPage, LessPage, ExtendedBlock;
		UINT BasicBlock, TotalBlock;
		BYTE InternalChip;
		BYTE RetryCnt=0, RetryMax=10;

		// 20140819 For Erase Count Use
		BYTE ECC;
		USHORT Value=0, EraseCount=0;
		ECC = pmFlashFwScheme->Model3&0x0F;

		BlockPage	= pmFlashFwScheme->BlockPage;
		PageSEC		= pmFlashFwScheme->PageSEC;
		LessPage		= pmFlashFwScheme->LessPage;
		ExtendedBlock= pmFlashFwScheme->LessBlock % 0x200;

		CE 	= mChipSelectNum;
		CH 	= mChannelNum;
		Entry= mEntryItemNum;
		PseudoBlockStart = Entry*8 -0x200 + ExtendedBlock;  // = Total_Block - 0x200 + LessBlock, LessBlock Must Exist

		// ----- Ashley_20140813 Display Sorting Time -----
		BasicBlock= 0x200<<((pmFlashFwScheme->Model5&0xF0)>>4);	// Basic Block Number
		InternalChip = (0x01 << (pmFlashFwScheme->FLH_ID[2] & 0x03));
		TotalBlock = (BasicBlock+ExtendedBlock)*CE*CH*InternalChip;

		// ----- Get Internal Chip Number & Address -----
		UINT	Lun1End, Lun2Start, Lun2End, Lun3Start; // 2 Internal Chip Use, Lun1Start = 0
		UINT	Lun3End, Lun4Start, Lun4End, Lun5Start; // 4 Internal Chip Use



			if((pmFlashFwScheme->FLH_ID[2] & 0x03) != 0x00) // Multi-Internal Chip Exist
			{
				InternalChip = (0x01 << (pmFlashFwScheme->FLH_ID[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }
				BasicBlock = 0x200<<((pmFlashFwScheme->Model5&0xF0)>>4);;
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
				if(Terminate==true) //terminate the thread and go back to start state
					goto Terminate_Sorting_Process;

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


				Address=ChipAddress|(BlockIndex*BlockPage);

				// 1. 20140820_Get Erase Count

				if(pCurSettingConfgInfo->SortingClearEraseCount == 0)
				{
					EraseCount= 0;
					BYTE Encryption =  pCurSettingConfgInfo->CisData.Reserved2[5];
					Status = getEraseCount(Address,Encryption, ECC, &Value);
					if(Status)
					{
						EraseCount = Value+1;
					}
				}
				else	//Clear Erase Count
					EraseCount= 1;

				// 2. Erase Block
				RetryCnt = 0;
	RetrySortingErase:

				Status = pmDriver->EraseBlock(Address, &EraseStatus);//erase block
				if((!Status) || (EraseStatus & BIT0))
				{
					usleep(20000);

					RetryCnt++;
					if(RetryCnt < RetryMax)
						goto RetrySortingErase;
					else
						goto Terminate_Sorting_Process;
				}

				// 3. Sorting Write
				RetryCnt = 0;
	RetrySortingWrite:
				//Status = eMMC_Sorting_Write(pThreadInfo, ChipIndex, BlockIndex, 0, BlockPage);
				Status = SortingWrite(ChipIndex, BlockIndex, EraseCount, BlockPage);
				if(!Status)
				{
					usleep(20000);
					RetryCnt++;
					if(RetryCnt < RetryMax)
						goto RetrySortingWrite;
					else
						goto Terminate_Sorting_Process;
				}

				// 4. Read ECC
				MaxECC = SortingRead(ChipIndex, BlockIndex, BlockPage);

				// 5. Process Of Sorting Result
				if(MaxECC >pCurSettingConfgInfo->MLCSortingECC) // It is Bad Block
				{
					BadBlkNum++;
					MarkBad(Address, PageSEC,LessPage);//mark bad

				}
			}
		}


		// ----- Ashley_20140813 Calculate Bad Rate ------
		return Success_State;	//return Status;

	Terminate_Sorting_Process:
		Status=Fail_State;
		return Status;
}

BOOL CFlash::WriteBadBlockInfoToFlash(LPSaveBadBlockInfo pBBInfo, UINT Address, BYTE PageIdx, BYTE ECCSet, UINT EraseCount){
	// TODO Auto-generated constructor stub
    BOOL	Status=true;
	BYTE	Redo, ValueTmp, buffer;
	UINT	i, BitErrorCnt;
	UINT	TempAddress, Idx;
	BYTE	*ptr;
	SaveBadBlockInfo	BBInfoTmp;
	UINT m_BlockPage = getBlockPage();
	SPARETYPE Spare;

	Spare.SPARE0=0x4D; //'M'
	Spare.SPARE1=0x50; //'P'
	Spare.SPARE2=0x00;
	Spare.SPARE3=0x00;
	Spare.SPARE4=0x99;
	Spare.SPARE5=0x99;

   // Sherlock_20131106, Set EraseCount for RWTest
	Spare.SPARE3=(BYTE)((EraseCount + 1)>>8);	// HI
	Spare.SPARE2=(BYTE)(EraseCount + 1);			// LO



	// ----- Sherlock_20130313, Swap For BigEnd -----
	for(Idx=0; Idx<pBBInfo->BBICnt; Idx++) // Swap Bad Block Address
	{
		TempAddress = pBBInfo->BBIAddr[Idx];
		ptr=(BYTE *)(&pBBInfo->BBIAddr[Idx]);
		*(ptr+0)=*(((BYTE *)&TempAddress)+3);
		*(ptr+1)=*(((BYTE *)&TempAddress)+2);
		*(ptr+2)=*(((BYTE *)&TempAddress)+1);
		*(ptr+3)=*(((BYTE *)&TempAddress)+0);
	}

	for(Idx=0; Idx<3; Idx++) 				// Swap Bad Block Name
	{
		TempAddress = pBBInfo->BBIName[Idx];
		ptr=(BYTE *)(&pBBInfo->BBIName[Idx]);
		*(ptr+0)=*(((BYTE *)&TempAddress)+3);
		*(ptr+1)=*(((BYTE *)&TempAddress)+2);
		*(ptr+2)=*(((BYTE *)&TempAddress)+1);
		*(ptr+3)=*(((BYTE *)&TempAddress)+0);
	}

		TempAddress = pBBInfo->BBICnt; 	// Swap Bad Block Number
		ptr=(BYTE *)(&pBBInfo->BBICnt);
		*(ptr+0)=*(((BYTE *)&TempAddress)+3);
		*(ptr+1)=*(((BYTE *)&TempAddress)+2);
		*(ptr+2)=*(((BYTE *)&TempAddress)+1);
		*(ptr+3)=*(((BYTE *)&TempAddress)+0);

	if(PageIdx == 0)
				Status=pmDriver->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &buffer);


	memset(&BBInfoTmp, 0, sizeof(SaveBadBlockInfo));

	for(Redo=0; Redo<3; Redo++)
	{
				Status=pmDriver->BlockAccessWrite(MI_BLOCK_WRITE, (sizeof(SaveBadBlockInfo)/512), ECCSet, Spare, 0, 0, Address+PageIdx, sizeof(SaveBadBlockInfo), (BYTE *)pBBInfo);
				Status=pmDriver->BlockAccessRead( MI_BLOCK_READ, (sizeof(SaveBadBlockInfo)/512), ECCSet, 0, 0, m_BlockPage, Address+PageIdx, sizeof(SaveBadBlockInfo), (BYTE *)&BBInfoTmp);
		for(i=0; i<sizeof(TAT_INFO); i++)// compare byte by byte and with BitErrorRate
		{
			if((i%1024)==0)
				BitErrorCnt=0;
			ValueTmp=((BYTE *)pBBInfo)[i]^((BYTE *)&BBInfoTmp)[i];
			if(ValueTmp)
			{
				BitErrorCnt+=BitCount(ValueTmp);
				if(BitErrorCnt>15)
				{
					if(PageIdx != 0x00)
					{	// Sherlock_20130313, Cannot Erase & Retry for These Pages
						return false;
					}

										Status=pmDriver->BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, &buffer);
					Status=false;
					break;
				}
			}
		}

		if(Status)
			break;
	}

	return Status;
}


TableCFG * CFlash::getTableCfgTable(SettingConfgInfo *pCurSettingConfgInfo) {

		TableCFG *pTableCfg = new TableCFG();

		if(pCurSettingConfgInfo->LunTypeBitMap & EraseGoodBlk){//scan block and clear system block(BIT_0), erase good block(BIT_2) and check bad block count(BIT_1)
			pTableCfg->Type|=(BIT0); //Clear Log Block and Free Block(BIT_0) in Build Sys Table
		}else if(pCurSettingConfgInfo->LunTypeBitMap & ClearSysBlock){//scan block and clear system block(BIT_0) and check bad block count(BIT_1)
			pTableCfg->Type|=(BIT0); //Clear Log Block and Free Block(BIT_0) in Build Sys Table
		}else {//only scan block and check bad block count
			pTableCfg->Type&=(~BIT0); //Not Clear Log Block and Free Block in Build Sys Table
		}
		memset(pTableCfg, 0x00, sizeof(TableCFG));
		pTableCfg->MaxChipFind = 1;
		pTableCfg->MaxPageFind = 0x40000; // Sherlock_20141007
		if(mPlaneNum == 1)
			pTableCfg->MaxVaildCISBlockFind = 1; // Get 2 Blocks for eCIS
		else
			pTableCfg->MaxVaildCISBlockFind = 2; // Get 4 Blocks for eCIS
		pTableCfg->MaxTableChipFind = 1;	// Only Search Chip_0
		pTableCfg->MaxLogBlock = 24;		// No Use
		pTableCfg->MaxFreeBlock = 16;	// No Use
		pTableCfg->SelectPlane = pmFlashFwScheme->SelectPlane;
		pTableCfg->InternalChip = (0x01 << (pmFlashFwScheme->FLH_ID[2]&0x03));
		pTableCfg->BasicBlock= 0x200<<((pmFlashFwScheme->Model5&0xF0)>>4);	// Basic Block Number;
		pTableCfg->ExtendedBlock = pmFlashFwScheme->LessBlock;					// Add Extended Block
		pTableCfg->AddressPage = pmFlashFwScheme->BlockPage;
		pTableCfg->LessPage = pmFlashFwScheme->LessPage;
		pTableCfg->PageSEC = pmFlashFwScheme->PageSEC;
		pTableCfg->ErrorBitRate = (pCurSettingConfgInfo->LunTypeBitMap2 & BitErrorRate);
		pTableCfg->ECCSET = pmFlashFwScheme->Model3 & 0x0F;
		pTableCfg->Type|=MustMarkBad_TYPE;//(BIT2); //mark bad when block stress test fail or write system table fail retry 3 times
		pTableCfg->BaseFType = mBaseFType;
		pTableCfg->Mode = 0;
		pTableCfg->ED3EraseCount = 0;
		pTableCfg->RsvInfo = 0;


	if(pCurSettingConfgInfo->ARFlashType != 0x00)
	{
//		TableCfg.MaxPageFind = 0x40000;
		pTableCfg->Mode = ARgrade_Mode;
	}

	if(pCurSettingConfgInfo->LunTypeBitMap & DumpDebugMemory)//dump memory for firmware debug use
		pTableCfg->Type|=(BIT1);
	else
		pTableCfg->Type &=(~BIT1);// 20110705

	if( (pCurSettingConfgInfo->BadColumnSwitch == 1) && // Switch Enable
		((mBaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC)) && // ED3 & Bad_Column
		((pmFlashFwScheme->FLH_ID[5] & 0x07) >= 0x05))
	{

		pTableCfg->Type = pTableCfg->Type & (~(MustMarkBad_TYPE));	// Sherlock_20120927, Not Mark Bad While Bad Column Disabled
	}
		return pTableCfg;
}

ULONG CFlash::getLastBlockAddr() {
	// TODO Auto-generated constructor stub
	BYTE 	InternalChip;
		ULONG	BlockEnd;

		InternalChip = (0x01 << (pmFlashFwScheme->FLH_ID[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }
		BlockEnd = 0x200<<((pmFlashFwScheme->Model5&0xF0)>>4);		// Basic Block Number

		if((InternalChip > 1) && (pmFlashFwScheme->LessBlock != 0x00))		// With InternalChip & LessBlock
			BlockEnd = (BlockEnd * 2 * InternalChip) -BlockEnd + pmFlashFwScheme->LessBlock;
		else if((InternalChip > 1) && (pmFlashFwScheme->LessBlock == 0x00))	// With InternalChip & No LessBlock
			BlockEnd = BlockEnd * InternalChip;
		else																		// No Internal Chip
			BlockEnd = BlockEnd + pmFlashFwScheme->LessBlock;

		return BlockEnd;
}

UINT CFlash::readData(ULONG Address, USHORT BufLen, BYTE *buffer){
	// TODO Auto-generated constructor stub
	return pmDriver->readData(Address,BufLen,buffer);
}

UINT CFlash::writeData(ULONG Address, USHORT BufLen, BYTE *buffer){
	// TODO Auto-generated constructor stub
	return pmDriver->writeData(Address,BufLen,buffer);
}
UINT CFlash::readBlockData(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer){
	// TODO Auto-generated constructor stub
	BOOL	Status = false, IsCisIsp = false;

		if(((Spare.SPARE0 == 0x43) && (Spare.SPARE1 == 0x49) && (Spare.SPARE2 == 0x53)) ||		// Sherlock_20110822, CIS Block
			((Spare.SPARE0 == 0x49) && (Spare.SPARE1 == 0x53) && (Spare.SPARE2 == 0x50)) ||		// Sherlock_20110822, ISP Block
			((Spare.SPARE0 == 0x43) && (Spare.SPARE1 == 0x53) ) 	)							// Sherlock_20130927, 'C' 'S'  for eCIS
			IsCisIsp = true;														// Special Modify for FW V37~V40 Still Use Encryption for ISP Block


		// Sherlock_20111006, , Use Max ECC For System Table
		BYTE	NormalECC, MaxECC, ErrorCnt=0;
		ULONG	Reg_ECCSetting = 0x1FF82608; // Default
		if(IsCisIsp)// && (ChipID == VT3468)) // Only Need To Execute in ChipID = VT3468
		{
			ErrorCnt = 0;
	ReReadECCSetting2:
			Status = AccessMemoryRead(MI_READ_DATA, 0, 0, Reg_ECCSetting, 1, &NormalECC); // Read Reg[2608]
			if(!Status) // Sherlock_20121107
			{
				ErrorCnt++;
				if(ErrorCnt<10)
					goto ReReadECCSetting2;
				else
					goto EndOfReadWriteBlock;
			}

			MaxECC = (NormalECC & 0xFC) | 0x02; // ECC Number = 60-Bits
			Status = AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_ECCSetting, 1, &MaxECC); // Write Reg[2608]

			usleep(2000);	// Sherlock_20121130, Test
		}


		if(IsCisIsp) // If ReadWrite CIS or ISP Block, Must Turn ON ECC Mode, Sherlock_20110721
			ECC = ECC|0x40; // ECC_ON Flag

			//OutputDebugString("ReadBlock()");
			Status=BlockAccessRead(MI_BLOCK_READ, PageSec, ECC, 0, 0, getBlockPage(), Address, BufLen, buffer);

 // Sherlock_20111006, Return Normal ECC Value
		if(IsCisIsp)// && (ChipID == VT3468))
			Status = AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_ECCSetting, 1, &NormalECC); 	// Write Reg[2608]

	EndOfReadWriteBlock:
		return Status;
}
UINT CFlash::writeBlockData(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer){
	BOOL	Status = false, IsCisIsp = false;

				if( 	((Spare.SPARE0 == 0x43) && (Spare.SPARE1 == 0x49) && (Spare.SPARE2 == 0x53)) ||		// Sherlock_20110822, CIS Block
					((Spare.SPARE0 == 0x49) && (Spare.SPARE1 == 0x53) && (Spare.SPARE2 == 0x50)) ||		// Sherlock_20110822, ISP Block
					((Spare.SPARE0 == 0x43) && (Spare.SPARE1 == 0x53) ) 	)							// Sherlock_20130927, 'C' 'S'  for eCIS
					IsCisIsp = true;														// Special Modify for FW V37~V40 Still Use Encryption for ISP Block


				// Sherlock_20111006, , Use Max ECC For System Table
				BYTE	NormalECC, MaxECC, ErrorCnt=0;
				ULONG	Reg_ECCSetting = 0x1FF82608; // Default
				if(IsCisIsp)// && (ChipID == VT3468)) // Only Need To Execute in ChipID = VT3468
				{
					ErrorCnt = 0;
			ReReadECCSetting2:
					Status = AccessMemoryRead(MI_READ_DATA, 0, 0, Reg_ECCSetting, 1, &NormalECC); // Read Reg[2608]
					if(!Status) // Sherlock_20121107
					{
						ErrorCnt++;
						if(ErrorCnt<10) 	goto ReReadECCSetting2;
						else				goto EndOfReadWriteBlock;
					}

					MaxECC = (NormalECC & 0xFC) | 0x02; // ECC Number = 60-Bits
					Status = AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_ECCSetting, 1, &MaxECC); // Write Reg[2608]

					usleep(2000);	// Sherlock_20121130, Test
				}


				if(IsCisIsp) // If ReadWrite CIS or ISP Block, Must Turn ON ECC Mode, Sherlock_20110721
					ECC = ECC|0x40; // ECC_ON Flag


					//OutputDebugString("WriteBlock()");
					Status=BlockAccessWrite(MI_BLOCK_WRITE, PageSec, ECC, Spare, 0, 0, Address, BufLen, buffer);


		 // Sherlock_20111006, Return Normal ECC Value
				if(IsCisIsp)// && (ChipID == VT3468))
					Status = AccessMemoryWrite(MI_WRITE_DATA, 0, 0, Reg_ECCSetting, 1, &NormalECC); 	// Write Reg[2608]

			EndOfReadWriteBlock:

				return Status;
}

UINT CFlash::ReadSpareData(BYTE COLA1, BYTE COLA0, ULONG Address, USHORT BufLen, BYTE *buffer){
	BOOL	Status=true;
	Status=pmDriver->SpareAccessRead(MI_SPARE_READ, COLA1, COLA0, 0, 0, pmFlashStructure->BlockPage, Address, BufLen, buffer);

	return Status;

}
UINT CFlash::WriteSpareData(BYTE COLA1, BYTE COLA0, ULONG Address, USHORT BufLen, BYTE *buffer){
	BOOL	Status=true;
	Status=pmDriver->SpareAccessWrite(MI_SPARE_WRITE, COLA1, COLA0, 0, 0, Address, BufLen, buffer);

	return Status;
}

UINT CFlash::EraseBlock(ULONG Address, BYTE *buffer){

	//	OutputDebugString(" BlockOtherRead()");
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		BOOL status = 0;
		ULONG length = 0;

	//	DWORD dwError;
		BYTE MI_CMD = MI_BLOCK_ERASE;
		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = 0;
		sptwb.spt.TargetId = 0;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = 1;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	      		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_BLOCK_OTHER;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
				sptwb.spt.DataTransferLength;
/*
		status = pCeMMCDeviceIO->SendPackageCmd(DiskPath,
												&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_SCSI,
												ACCESS_CONTROL,
												UNPACK_REG1BYTE); // UNPACK_STATUS); Sherlock_20140812, Result Status From Byte[26]     */
		return status;

}

BYTE CFlash::getPageSEC(){
	// TODO Auto-generated constructor stub
	return pmFlashFwScheme->PageSEC;
}

UINT  CFlash::MultiPageRead(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer)
{
	UINT	Status = Fail_State;//, TempStatus, WaitCount = 0;;

	VendorCMD	VCMD;
	memset(&VCMD, 0x00, sizeof(VendorCMD));

	VCMD.OPCode = 0xEE;
	VCMD.MJCMD = 0x51;
	VCMD.Address = Address;
	VCMD.MICMD = 0x80;
	VCMD.BufLen = (((USHORT)PageSec)<<8) + (BYTE)(BufLen/512);
	VCMD.COLA = (((USHORT)ECC)<<8) + Spare.SPARE0;
	VCMD.CFG[0] = Spare.SPARE1;
	VCMD.CFG[1] = Spare.SPARE2;
	VCMD.CFG[2] = Spare.SPARE3;
	VCMD.CFG[3] = Spare.SPARE4;
	VCMD.CFG[4] = Spare.SPARE5;

	Status = pmDriver->MultiPlaneRead(VCMD.MICMD, HIBYTE(VCMD.BufLen),HIBYTE(VCMD.COLA), 0, 0, getBlockPage(), VCMD.Address,(ULONG)(LOBYTE(VCMD.BufLen))*512, buffer);


	return Status;
}

UINT  CFlash:: MultiPageWrite(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer)
{
	UINT	Status = Fail_State;//, TempStatus, WaitCount = 0;;

	VendorCMD	VCMD;
	memset(&VCMD, 0x00, sizeof(VendorCMD));

	VCMD.OPCode = 0xEE;
	VCMD.MJCMD = 0x51;
	VCMD.Address = Address;
	VCMD.MICMD = 0x81;
	VCMD.BufLen = (((USHORT)PageSec)<<8) + (BYTE)(BufLen/512);
	VCMD.COLA = (((USHORT)ECC)<<8) + Spare.SPARE0;
	VCMD.CFG[0] = Spare.SPARE1;
	VCMD.CFG[1] = Spare.SPARE2;
	VCMD.CFG[2] = Spare.SPARE3;
	VCMD.CFG[3] = Spare.SPARE4;
	VCMD.CFG[4] = Spare.SPARE5;

	Status = pmDriver->MultiPlaneWrite(VCMD.MICMD, HIBYTE(VCMD.BufLen),HIBYTE(VCMD.COLA), Spare, 0, 0,VCMD.Address, (ULONG)(LOBYTE(VCMD.BufLen))*512, buffer);

	return Status;
}

//�n���X�h
UINT CFlash::initEntryItemNum(){

	INT  Entry=0x200, PseudoBlock = 0;
	BYTE InternalChip = 1;

	Entry=Entry<<((pmFlashStructure->FlashFwScheme->Model5 & 0xF0)>>4);
	InternalChip = (0x01 << (pmFlashStructure->FlashFwScheme->FLH_ID[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }

		if((InternalChip > 1) && (pmFlashStructure->FlashFwScheme->LessBlock != 0x00))
		{	// Multi-Internal Chip && Extended Block Exist
			PseudoBlock = Entry - pmFlashStructure->FlashFwScheme->LessBlock; // Only for Debug MSG

			Entry = Entry * 2 *  InternalChip;	// = (Entry + Pseudo_Entry) * InternalChip

		}
		else if((InternalChip > 1) && (pmFlashStructure->FlashFwScheme->LessBlock == 0x00)) // For IM_2Die, Sherlock_20121128
		{
			Entry = Entry *  InternalChip;	// = Entry * InternalChip
		}
		else // Single Internal Chip or InternalChip == 0
		{
			// ----- Make Entry as 0x200*N -----
			if(pmFlashStructure->FlashFwScheme->LessBlock % 0x200)
				PseudoBlock = 0x200 - (pmFlashStructure->FlashFwScheme->LessBlock % 0x200);
			else
				PseudoBlock = 0;

			Entry = Entry + pmFlashStructure->FlashFwScheme->LessBlock + PseudoBlock;
			// Block Map = Basic_Block	+ Extended_Block + Pseudo_Block
		}
		return Entry;
}


UINT CFlash::EraseCISAddress(UINT *eCISADDR){
	UINT 	Status = Success_State;
		UINT	tEraseCnt[4], EachTxSize=1024;
		BYTE	EncryptionCMD=0, SpareBuf[6], i;
		BYTE	*TxDataBuf, EraseStatus, ECC=0;
		moldversionCISflag = 0;
		SPARETYPE	Spare;

		TxDataBuf=(BYTE *)malloc(sizeof(BYTE)*EachTxSize);
		memset(TxDataBuf, 0xFF, sizeof(BYTE)*EachTxSize);
		Spare.SPARE0=0x4D; // 'M'
		Spare.SPARE1=0x50; // 'P'
		Spare.SPARE2=0x00; // Initial Value
		Spare.SPARE3=0x00; // Initial Value
		Spare.SPARE4=0x99; // FW Request
		Spare.SPARE5=0x99; // FW Request

		for(i=0; i<4; i++)
		{
			if((eCISADDR[i] == 0)	||(eCISADDR[i] == 0xFFFFFFFF))	continue; // Skip Wrong Address
			ReadSpareData(EncryptionCMD, 0, eCISADDR[i], 6, SpareBuf);
			USHORT Shift1 = SpareBuf[1]&0x3F;	// Clear Bit7 and Bit6
			if(	((SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)) || 	// 'CS'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x52)) ||	// 'RT'
				((Shift1<<8 | SpareBuf[0]) == (0x48<<7 | 0x43)) ||	// 'CH'
				((Shift1<<8 | SpareBuf[0]) == (0x4B<<7 | 0x4D)) ||	// 'MK'
				((Shift1<<8 | SpareBuf[0]) == (0x54<<7 | 0x44)) ||	// 'DT'
				((Shift1<<8 | SpareBuf[0]) == (0x55<<7 | 0x44)) ||	// 'DU'
				((SpareBuf[0] == 0x4D) && (SpareBuf[1] == 0x50)) )	// 'MP' This Means "Erased By MPTool"
				tEraseCnt[i] = ((UINT)SpareBuf[3]<<8) | SpareBuf[2];	// Real Erase Count
			else
				tEraseCnt[i] = 0;

			Spare.SPARE3 = (BYTE)(((tEraseCnt[i]+1) & 0x0000FF00) >> 8);	// HI
			Spare.SPARE2 = (BYTE)((tEraseCnt[i]+1) & 0x000000FF);		// LO

			Status = pmDriver->EraseBlock(eCISADDR[i], &EraseStatus);;

			Status = writeBlockData((BYTE)(EachTxSize/512),
												ECC,
												Spare,
												eCISADDR[i],
												EachTxSize,
												TxDataBuf);
		}

		free(TxDataBuf);
		return Status;


	return 0;
}

UINT CFlash::ReadTestCmd(VendorCMD VCMD, BYTE *buffer){

	return pmDriver->sendGetCommand(VCMD,buffer);
}

UINT CFlash::WriteTestCmd(VendorCMD VCMD, BYTE *buffer){

	return pmDriver->sendSetCommand(VCMD,buffer);
}

UINT CFlash::isOldVersionCISExit(){
	// If Find 1~4 Address, Return OK
		UINT  TempStatus;
		ULONG	Address;
		BYTE	index, CISIdx=0, SpareBuf[6] = {0,0,0,0,0,0};
		BYTE PlaneNum;
		PlaneNum = 0x01 << ((pmFlashStructure->FlashFwScheme->Model6 & 0x30) >> 4);

        if (moldversionCISflag == -1){
        	moldversionCISflag = 0;
			for(index=0; index<100; index++){ // Search 100 Blocks for CIS Block
				Address = index * pmFlashStructure->FlashFwScheme->BlockPage;
				TempStatus = ReadSpareData(0, 0, Address, 6, SpareBuf);
				if((TempStatus) && (SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53)){
					CISIdx++;
					moldversionCISflag = Success_State; // At Least Find 1 CIS Block
				}

				if (CISIdx >= (PlaneNum*2)){
					break;
				}
			}

        }

        return moldversionCISflag;
}

UINT CFlash::getOldVersionCISAddress(UINT *CISADDR){
	// If Find 1~4 Address, Return OK
		UINT  TempStatus;
		ULONG	Address;
		BYTE	index, CISIdx=0, SpareBuf[6] = {0,0,0,0,0,0};
		BYTE PlaneNum;
		PlaneNum = 0x01 << ((pmFlashStructure->FlashFwScheme->Model6 & 0x30) >> 4);
		UINT 	Status = Fail_State;
		ULONG	EachTxSize = 512;
		BYTE	BufferLen;
		BYTE	ECC ,RetryCnt;
		BYTE	TxDataBuf[13];
		eMMC_CIS_INFO	eCISSetData;
		SPARETYPE Spare;

		Spare.SPARE0=0x43; //'C'
		Spare.SPARE1=0x53; //'S'
		Spare.SPARE2=0x00; //Initial Value
		Spare.SPARE3=0x00; //Initial Value
		Spare.SPARE4=0x99;
		Spare.SPARE5=0x99;

		//if(PlaneNum == 2)
		BufferLen = 13;

		memset(TxDataBuf, 0,BufferLen);
		writeData(0x1FF85000, (USHORT)BufferLen, TxDataBuf);//SLC
		ECC = 2 + BIT5 + BIT7; //60 bit + SLC +  Encryption on
		for(index=0; index<100; index++) // Search 100 Blocks for CIS Block
		{
			Address = index * mBlockPage;
			TempStatus = ReadSpareData(0xA0, 0, Address, 6, SpareBuf);
			if((TempStatus) && (SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53))
			{
				for(RetryCnt=0; RetryCnt<3; RetryCnt++)
				{
					Status=readBlockData(	(BYTE)(EachTxSize/512),
											ECC,
											Spare,
											Address,
											EachTxSize,
											(BYTE *)&eCISSetData);
					if(Status)
						break;
				}

				if(!Status)
					continue;
				if(eCISSetData.PAGE_MODE == 1)
				{
					CISADDR[CISIdx] = Address;
					CISIdx++;
					moldversionCISflag = Success_State; // At Least Find 1 CIS Block
					mCisBlkPgeMode = 1; //SLC mode
				}

			}

			if (CISIdx >= (PlaneNum * 2))
			{
				break;
			}
		}

		if(CISIdx == 0)
		{
			memset(TxDataBuf, 1, BufferLen);
			writeData(0x1FF85000, (USHORT)BufferLen, TxDataBuf);//MLC
			ECC = 2 + BIT7; //60 bit + MLC +  Encryption on
			for(index=0; index<100; index++) // Search 100 Blocks for CIS Block
			{
				Address = index * pmFlashStructure->BlockPage;
				TempStatus = ReadSpareData(0x80, 0, Address, 6, SpareBuf);
				if((TempStatus) && (SpareBuf[0] == 0x43) && (SpareBuf[1] == 0x53))
				{
					for(RetryCnt=0; RetryCnt<3; RetryCnt++)
					{
						Status=readBlockData(	(BYTE)(EachTxSize/512),
												ECC,
												Spare,
												Address,
												EachTxSize,
												(BYTE *)&eCISSetData);
						if(Status)
							break;
					}

					if(!Status)
						continue;
					if(eCISSetData.PAGE_MODE == 0)
					{
						CISADDR[CISIdx] = Address;
						CISIdx++;
						moldversionCISflag = Success_State; // At Least Find 1 CIS Block
						mCisBlkPgeMode = 0; //MLC mode
					}

				}

				if (CISIdx >= (PlaneNum * 2))
				{
					break;
				}
			}

		}
        return moldversionCISflag;
}

BYTE CFlash::getCisBlkPgeMode(void){

	return mCisBlkPgeMode;
}

UINT CFlash::UFDSettingRead(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer){

	return pmDriver->UFDSettingRead(MI_CMD, CFG0, adapter_id, target_id,Address, BufLen, buffer);

}

UINT CFlash::AccessMemoryRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {

	return pmDriver->AccessMemoryRead(MI_CMD, adapter_id, target_id, Address, BufLen, buffer);

}

UINT CFlash::AccessMemoryWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {

	return pmDriver->AccessMemoryWrite(MI_CMD, adapter_id, target_id, Address, BufLen, buffer);

}

UINT CFlash::BlockCheckRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE COLA1, BYTE COLA0, BYTE CFG0, BYTE CFG1, BYTE CFG2, USHORT PlaneBlock, BYTE *buffer) {

	return pmDriver->BlockCheckRead(MI_CMD, adapter_id, target_id, Address, BufLen, COLA1, COLA0, CFG0, CFG1, CFG2, PlaneBlock, buffer);

}

UINT CFlash::BlockOtherRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {

	return pmDriver->BlockOtherRead(MI_CMD, adapter_id, target_id, Address, BufLen, buffer);

}

UINT CFlash::BlockAccessWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer) {

	return pmDriver->BlockAccessWrite(MI_CMD, PageSec,ECC, Spare,adapter_id, target_id,Address,BufLen,buffer);

}

UINT CFlash::BlockAccessRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer) {

	return pmDriver->BlockAccessRead(MI_CMD, PageSec, ECC,  adapter_id, target_id, BlockPage, Address, BufLen, buffer);

}

UINT CFlash::SetInfoWriteCMD(BYTE adapter_id, BYTE target_id, VendorCMD VCMD, BYTE *buffer) {

	return pmDriver->SetInfoWriteCMD(adapter_id, target_id, VCMD, buffer);

}

UINT CFlash::INITISP(ULONG AddrOffset, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	cout << "CeMMCDriver::INITISP" << endl;
	BOOL	Status=true;
	BYTE buffertmp[512];
	// Sherlock_20111110, Add A Special Flag for SendTURdy in Scan_Only Mode.
	BYTE	WaitCnt = 0, ScsiStatus, SendTURdy = 0;
	if(AddrOffset & 0x0001)
	{
		SendTURdy = 1;
		AddrOffset = AddrOffset&(~0x0001);
	}
    memcpy(buffertmp,buffer,512);
	Status=pmDriver->UFDSettingWrite(MI_INIT_ISP, 0, 0, 0, AddrOffset, BufLen, buffer);

// Sherlock_20111110, Add A Special Flag for SendTURdy in Scan_Only Mode.
	if(SendTURdy)
	{
		while(WaitCnt<5) // 5
		{
			usleep(200000);
			if(pmDriver->SendTestUnitReady(0, 0, 0, &ScsiStatus))
			{
				if(!ScsiStatus)	break;
			}
			else
			{
				cout << "TURdy_Fail" << endl;
			}

			WaitCnt++;
		}
	}

	return Status;
}

UINT CFlash::BlockMarkWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer){
	// TODO Auto-generated constructor stub
	return pmDriver->BlockMarkWrite(MI_CMD,adapter_id,target_id,Address,BufLen,LEN0,CFG0,CFG1,buffer);
}
UINT CFlash::CopySLCtoTLC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, WORD LunOffset){
	// TODO Auto-generated constructor stub
	return pmDriver->CopySLCtoTLC(MI_CMD, CE, CH, BlockAddr, Mode, LunOffset);
}
UINT CFlash::MLCVBWrite(BYTE MI_CMD, WORD BlockAddr, WORD LunOffset){
	// TODO Auto-generated constructor stub
	return pmDriver->MLCVBWrite(MI_CMD, BlockAddr,LunOffset);
}
UINT CFlash::FillMainFIFO(BYTE MI_CMD, ULONG BufLen, BYTE *buffer){
	// TODO Auto-generated constructor stub
	return pmDriver->FillMainFIFO(MI_CMD, BufLen,buffer);
}

UINT CFlash::BlockCheckECC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, USHORT BufLen, BYTE *buffer){
	// TODO Auto-generated constructor stub
	return pmDriver->BlockCheckECC(MI_CMD, CE, CH, BlockAddr, Mode, BufLen, buffer);
}

UINT CFlash::SetThreeSLCVB(BYTE MI_CMD){
	// TODO Auto-generated constructor stub
	return pmDriver->SetThreeSLCVB(MI_CMD);
}
UINT CFlash::SpareAccessRead(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, USHORT BufLen, BYTE *buffer){

	return pmDriver->SpareAccessRead(MI_CMD,COLA1,COLA0,adapter_id,target_id,BlockPage,Address,BufLen,buffer);
}

