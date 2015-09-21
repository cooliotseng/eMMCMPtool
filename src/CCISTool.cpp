/*
 * CCISTool.cpp
 *
 *  Created on: May 14, 2015
 *      Author: via
 */

#include "CCISTool.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
CCISTool::CCISTool() {
	// TODO Auto-generated constructor stub

}

CCISTool::~CCISTool() {
	// TODO Auto-generated destructor stub
}

UINT CCISTool::setBlockMaptoBitMap(CFlash *pflash,CRootTable *roottable,LPMapChipSelect pUFDBlockMap,eMMC_CIS_INFO *pCISInfo) {
	// TODO Auto-generated constructor stub
	UINT	Status=Success_State; //, RegStatus = false;
	BYTE	BlockSts, Value, CE, CH;//, InternalChip;//, EraseStatus
	BYTE 	*TableBuf;
	BYTE	SelectPlane = pflash->getPlaneNum();    //pCisDataEx->FLH_FwScheme.SelectPlane;
	ULONG 	TableLen, ChipIndex=0;
	ULONG	BlockIndex, BlockEnd, BitMapSize;
	ROOT_VARS *RootTable=roottable->getRootTable();
	BlockEnd = pflash->getLastBlockAddr();

	TableLen = (USHORT)(((BlockEnd + 7) / 8) * 2);



	BitMapSize = offsetof(eMMC_CIS_INFO, Cell_Map[0]) - offsetof(eMMC_CIS_INFO, Bit_Map[0]);

	if((ULONG)pflash->getChipSelectNum()*pflash->getChannelNum()*TableLen > BitMapSize) // Protection
	{
		return 0;
	}

	if(SelectPlane == 2) // Sherlock_20141003
	{
		// ---------- Modify For Bit Map ----------
		// Set Bad Block if Another Plane is Bad (For 2 Plane Flash Only)
		BYTE	BlockSts0, BlockSts1; 		// Even & Odd
		ULONG	BlockIndex0, BlockIndex1; 	// Even & Odd
		for(CE=0; CE<pflash->getChipSelectNum(); CE++)
		{
			for(CH=0; CH<pflash->getChannelNum(); CH++)
			{
				for(BlockIndex0=0; BlockIndex0<BlockEnd; BlockIndex0+=2)
				{
					BlockIndex1 = BlockIndex0 + 1;
					BlockSts0 = roottable->getMapEntryItem(CE,CH,(BlockIndex0/8), (BYTE)BlockIndex0%8);
					BlockSts1 = roottable->getMapEntryItem(CE,CH,(BlockIndex1/8), (BYTE)BlockIndex1%8);

					if((BlockSts0 == 0) && (BlockSts1 == 1)) 		// Even is Good Block But Odd is Bad Block
						roottable->setMapEntryItem(CE,CH,(BlockIndex0/8), (BYTE)(BlockIndex0%8), 1);
					else if((BlockSts0 == 1) && (BlockSts1 == 0))	// Even is Bad Block But Odd is Good Block
						roottable->setMapEntryItem(CE,CH,(BlockIndex1/8), (BYTE)(BlockIndex1%8), 1);
				}
			}
		}
	}

	// ---------- Set Bit Map into peCISData ----------
	TableBuf = (BYTE *)malloc(sizeof(BYTE)*TableLen); // Temp BitMap of Each Lun

	for(CE=0; CE<pflash->getChipSelectNum(); CE++)
	{
		for(CH=0; CH<pflash->getChannelNum(); CH++)
		{
			memset(TableBuf, 0x00, sizeof(BYTE)*TableLen); // Clear All

			for(BlockIndex=0; BlockIndex<BlockEnd; BlockIndex++)
			{
				BlockSts = roottable->getMapEntryItem(CE,CH,(BlockIndex/8), (BYTE)(BlockIndex%8));
				if(BlockSts == 0) // Good Block
				{
					Value = 0x03<<(2*(BlockIndex%4)); // Shift 2 Bits
					TableBuf[BlockIndex/4]|=Value;
				}
			}

			memcpy(&pCISInfo->Bit_Map[TableLen*ChipIndex], TableBuf, TableLen);	 // Copy into eCIS.BitMap[]
			ChipIndex++;
		}
	}

	// ---------- Set Cell & ECC Map into peCISData ----------

	// Initial All PairMaps
	TableLen = offsetof(eMMC_CIS_INFO, ECC_Map[0]) - offsetof(eMMC_CIS_INFO, Cell_Map[0]);
	memset(&pCISInfo->Cell_Map[0], 0xFF, sizeof(BYTE)*(TableLen)); // Clear All As MLC


	TableLen = sizeof(eMMC_CIS_INFO) - offsetof(eMMC_CIS_INFO, ECC_Map[0]);
	memset(&pCISInfo->ECC_Map[0], 0x00, TableLen); // Clear All As Default


	for(CE=0; CE<pflash->getChipSelectNum(); CE++)
	{
		if((RootTable[CE].dwRTBLVersion & 0xFFFF0000) == 0x52540000) // RT_TAG Protection
		{
			memcpy(&pCISInfo->Cell_Map[512*CE], &RootTable[CE].chBlkTagBitMapTbl[0], 262);	 	// Copy into eCIS.CellMap[]
			memcpy(&pCISInfo->ECC_Map[512*CE], &RootTable[CE].chBlkECCBitMapTbl[0], 262);	// Copy into eCIS.ECCMap[]
		}
	}
	// ---------- Only Output Message of Cell ECC Map ----------


	free(TableBuf);
		return Status;
}
