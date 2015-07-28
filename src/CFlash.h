/*
 * CFlash.h
 *
 *  Created on: Apr 17, 2015
 *      Author: vli
 */

#ifndef CFLASH_H_
#define CFLASH_H_
#include "common.h"
#include "CeMMCDriver.h"

extern ULONG FileSize(FILE *fp);
extern UINT BitCount(UINT Value);

#define mBlockPage pmFlashStructure->BlockPage
#define mPageSize pmFlashStructure->PageSize
#define mPlaneNum pmFlashStructure->PlaneNum
#define mChipSelectNum pmFlashStructure->ChipSelectNum
#define mChannelNum pmFlashStructure->ChannelNum
#define mEntryItemNum pmFlashStructure->EntryItemNum
#define mBaseFType pmFlashStructure->BaseFType
#define pmFlashFwScheme pmFlashStructure->FlashFwScheme


class CFlash {
private:
	IeMMCDriver *pmDriver;
	FlashStructure *pmFlashStructure;
	UINT initEntryItemNum();
    INT moldversionCISflag;


public:
	CFlash();
	CFlash(IeMMCDriver *pDriver,FlashStructure *FlashStructure);
	virtual ~CFlash();
	IeMMCDriver* getDriver();
	Flash_FwScheme* getFlashFwScheme();
	TableCFG *getTableCfgTable(SettingConfgInfo *pCurSettingConfgInfo);
	UINT getPageSize();
	UINT getBlockPage();
	UINT getLessPage();
	UINT getLessBlock();
	BYTE getPlaneNum();
	WORD getBaseFType();
	BYTE getCHipVersion();
	void getFlashID(UINT* buf);
	UINT setFlashSize();
	BYTE getChipSelectNum();
	BYTE getChannelNum();
	UINT getEntryItemNum();
	UINT DownloadVDRFw(char *FWFileName);
	BYTE getFlashModel(UINT moddelNo);
	UINT MarkBad(ULONG Address, BYTE PageSEC,USHORT LessPage);
	UINT INITISP(ULONG AddrOffset, USHORT BufLen, BYTE *buffer);
	UINT resetEcc();
	UINT setMultiPageAccress();
	UINT getFlashType();
	UINT writeTPMT();
	UINT writeCellMapFlashType(ULONG TableAddress,BYTE MLCType);//0:MLC 1:SLC
	UINT writeEccMapBitLength(ULONG TableAddress,BYTE MaxECCBitLength);
	UINT isBlockEmpty(ULONG Address);
	UINT Sorting(bool Terminate,SettingConfgInfo *pCurSettingConfgInfo);
	UINT getEraseCount(ULONG Address, BYTE Encryption, BYTE ECC, USHORT *Value);
	UINT SortingWrite(BYTE PU_Index, USHORT BlockIndex, USHORT EraseCount, USHORT BlockPage);
	UINT SortingRead(BYTE PU_Index, USHORT BlockIndex, USHORT BlockPage);
	UINT readData(ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT writeData(ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT readBlockData(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT writeBlockData(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT ReadSpareData(BYTE COLA1, BYTE COLA2, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT ReadTestCmd(VendorCMD VCMD, BYTE *buffer);
	UINT WriteTestCmd(VendorCMD VCMD, BYTE *buffer);
	UINT WriteSpareData(BYTE COLA1, BYTE COLA2, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT MultiPageRead(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT MultiPageWrite(BYTE PageSec, BYTE ECC, SPARETYPE Spare, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT EraseBlock(ULONG Address, BYTE *buffer);
	USHORT getPlaneBlock();
	BYTE getPageSEC();
	ULONG getLastBlockAddr();
	UINT EraseCISAddress(UINT *eCISADDR);
	TurboPageInfo* getTurboPageInfo();
	UINT isOldVersionCISExit();
	UINT getOldVersionCISAddress(UINT *CISADDR);
	void setoldversionCISflag();
	void clearoldversionCISflag();
	BOOL WriteBadBlockInfoToFlash(LPSaveBadBlockInfo pBBInfo, UINT Address, BYTE PageIdx, BYTE ECCSet, UINT EraseCount); // Sherlock_20120130

	UINT UFDSettingRead(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT AccessMemoryRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT AccessMemoryWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT BlockCheckRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE COLA1, BYTE COLA0, BYTE CFG0, BYTE CFG1, BYTE CFG2, USHORT PlaneBlock, BYTE *buffer);
	UINT BlockOtherRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT BlockAccessWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT BlockAccessRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT SetInfoWriteCMD(BYTE adapter_id, BYTE target_id, VendorCMD VCMD, BYTE *buffer);
};
#endif /* CFLASH_H_ */
