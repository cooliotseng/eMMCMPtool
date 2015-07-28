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
	UINT			eCISADDR[4];
	UINT			Original_EraseCnt[4];
	UINT 			findAllCISAddr();
public:
	CCIS();
	CCIS(ICISDL *pCISDL,SettingConfgInfo *CurSettingConfgInfo,CFlash *pflash,CRootTable *roottable,ULONG ISPTotalByte);
	virtual ~CCIS();
	ICISDL * getCISDL();
	UINT ClearCIS();
	eMMC_CIS_INFO * getCISInfo();
	UINT* getCISBlocksAddr(SettingConfgInfo *CurSettingConfgInfo,CRootTable *roottable);
	UINT getGootlCISABlocksAddr(LPMapChipSelect pUFDBlockMap, LPBlockRecTable pCisIspTB, LPTableCFG pTableCfg);
	UINT DownloadCIS(SettingConfgInfo *CurSettingConfgInfo,CRootTable *pRootTable);
};

#endif /* CCIS_H_ */
