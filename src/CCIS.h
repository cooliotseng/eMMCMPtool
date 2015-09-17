/*
 * CCIS.h
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#ifndef CCIS_H_
#define CCIS_H_
#include "ICISDL.h"
#include "CRootTable.h"
#include "CFlash.h"

class CCIS {
private:

	eMMC_CIS_INFO	*pmCISInfo;
	CFlash			*pmflash;
	ICISDL 			*pmCISDL;
	CRootTable 		*pmroottable;
	UINT			eCISADDR[4];
	UINT			Original_EraseCnt[4];
	UINT 			m_BlockPage;

public:
	CCIS();
	CCIS(CRootTable *proottable,ICISDL *pCISDL,SettingConfgInfo *CurSettingConfgInfo,CFlash *pflash,CRootTable *roottable,ULONG ISPTotalByte);
	virtual ~CCIS();
	ICISDL * getCISDL();
	UINT ClearCIS();
	eMMC_CIS_INFO * getCISInfo();
	UINT* getCISBlocksAddr(SettingConfgInfo *CurSettingConfgInfo,CRootTable *roottable);
	UINT getGootlCISABlocksAddr(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pCisIspTB, LPTableCFG pTableCfg);
	UINT DownloadCIS(SettingConfgInfo *CurSettingConfgInfo,CRootTable *pRootTable);
	UINT findAllCISAddr();
	UINT MLCSortingFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount);
	UINT TLCSortingFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount);
	UINT PreMPTestFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount);
	UINT FindQuadriBlockFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount);
	UINT FindPairBlockFromTable(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pBlockRecTable, LPTableCFG pTableCfg, UINT ItemCount);
	UINT BlockStressTest(BYTE CEIndex, BYTE CHIndex, UINT EntryIndex, BYTE BlockIndex, LPTableCFG pTableCfg);
	UINT GetAndCheckEraseCount(BYTE EncryptionCmd, ULONG Address);
	UINT BlockECCTest(BYTE CEIndex, BYTE CHIndex, BYTE TLCMode, WORD BlockAddr, LPTableCFG pTableCfg, BYTE *ECCValue);
	UINT BlockECCRead(BYTE CEIndex, BYTE CHIndex, BYTE TLCMode, WORD BlockAddr, LPTableCFG pTableCfg, BYTE *ECCValue);
};

#endif /* CCIS_H_ */
