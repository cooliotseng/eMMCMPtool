/*
 * IFlashStructure.cpp
 *
 *  Created on: 2015¦~6¤ë17¤é
 *      Author: Coolio
 */

#include "IFlashStructure.h"

IFlashStructure::IFlashStructure() {
	// TODO Auto-generated constructor stub

}

IFlashStructure::~IFlashStructure() {
	// TODO Auto-generated destructor stub
}

INT IFlashStructure::getEntryItemNum(Flash_FwScheme *pFlashFwScheme) {
	// TODO Auto-generated destructor stub
	INT  Entry=0x200, j, PseudoBlock = 0;
	BYTE InternalChip = 1;
	Entry=Entry<<((pFlashFwScheme->Model5 & 0xF0)>>4);

		// Sherlock_20121128
	InternalChip = (0x01 << (pFlashFwScheme->FLH_ID[2] & 0x03));	// InternalChip { 00b=1, 01b=2, 10b=4, 11b=8 }

	if((InternalChip > 1) && (pFlashFwScheme->LessBlock != 0x00))
	{	// Multi-Internal Chip && Extended Block Exist
		Entry = Entry * 2 *  InternalChip;	// = (Entry + Pseudo_Entry) * InternalChip
	}
	else if((InternalChip > 1) && (pFlashFwScheme->LessBlock == 0x00)) // For IM_2Die, Sherlock_20121128
	{	// Multi-Internal Chip && Extended Block Not Exist
		Entry = Entry *  InternalChip;	// = (Entry + Pseudo_Entry) * InternalChip
	}
	else // Single Internal Chip or InternalChip == 0
	{
		// ----- Make Entry as 0x200*N -----
		if(pFlashFwScheme->LessBlock % 0x200)
			PseudoBlock = 0x200 - (pFlashFwScheme->LessBlock % 0x200);
		else
			PseudoBlock = 0;

		Entry = Entry + pFlashFwScheme->LessBlock + PseudoBlock;
	}
	return Entry/8;
}

UINT IFlashStructure::getPageSize(Flash_FwScheme *pFlashFwScheme) {
	// TODO Auto-generated destructor stub
	UINT PageSize;
	if((pFlashFwScheme->Model6 & 0x03) == 0x00) // 16K Page
			PageSize = 16*1024;
		else if((pFlashFwScheme->Model6 & 0x03) == 0x01) // 4K Page
			PageSize = 4*1024;
		else if((pFlashFwScheme->Model6 & 0x03) == 0x02) // 8K Page
			PageSize = 8*1024;
		else // Default As 16K Page
			PageSize = 16*1024;
	return PageSize;
}
