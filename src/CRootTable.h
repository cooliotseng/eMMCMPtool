/*
 * CRootTable.h
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#ifndef CROOTTABLE_H_
#define CROOTTABLE_H_
#include "common.h"
#include "CFlash.h"

class CRootTable {
private:
	RTblInfo		*pmRTInfo;
	ROOT_VARS 		*pmRootTable;
	MapChipSelect 	*pmUFDBlockMap;// coolio temp
	CFlash 			*pmflash;
	UINT 			mRTInfoINITFlag;

public:
	CRootTable();
	CRootTable(CFlash *flash);
	virtual ~CRootTable();
	UINT setCellMap(ULONG Address, BYTE MLC);
	UINT setEccMap(ULONG Address, BYTE MaxECC);
	UINT setSystemBlock(CFlash *pflash);
	UINT setEccErrBlock(CFlash *pflash);
	RTblInfo * getRTInfoTable();
	ROOT_VARS * getRootTable();
	MapChipSelect * getUFDBlockMap();
	void setUFDBlockMap(MapChipSelect *ufdblockmap);
	UINT getRootTableVersion();
	UINT initRTInfoTable();
	UINT initRootTable();
	void setFlash(CFlash *flash);
	UINT updateEraseCount();
	UINT writeCellMap();
	UINT writeEccMap();
	UINT ScanBlock(SettingConfgInfo *pmCurSettingConfgInfo);
	void setMapEntryItem(BYTE CEIndex, BYTE ChannelIndex,INT EntryItemIndex, BYTE BlockItemIndex, BYTE Value);
	BYTE getMapEntryItem(BYTE CEIndex, BYTE ChannelIndex,INT EntryItemIndex, BYTE BlockItemIndex);
	UINT EraseAllBlock();
	MapChipSelect *AllocateBlockMapMemory();
	BYTE GetUFDBlockMapByByte(BYTE CEIndex, BYTE ChannelIndex, INT EntryItemIndex);
	UINT UpdatePairMapByAddress(ULONG Address, BYTE MLC, BYTE MaxECC);
};

#endif /* CROOTTABLE_H_ */
