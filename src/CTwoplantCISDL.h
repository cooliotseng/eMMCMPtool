/*
 * CTwoplantCISDL.h
 *
 *  Created on: Apr 29, 2015
 *      Author: vli
 */

#ifndef CTWOPLANTCISDL_H_
#define CTWOPLANTCISDL_H_

#include "ICISDL.h"
#include "ICISTool.h"


class CTwoplantCISDL: public ICISDL {
private:
	UINT MultiPageCMDSet(CFlash *flash);
public:
	CTwoplantCISDL(ICISTool *pCisTool);
	virtual ~CTwoplantCISDL();
	UINT Execute(SettingConfgInfo *pCurSettingConfgInfo,
			CFlash			*pflash,
			CRootTable		*pRootTable,
			eMMC_CIS_INFO *pCISInfo,
			UINT 			*eCISADDR,
			UINT			*Original_EraseCnt,
			UINT 			OldCISVersionExit,
			UINT			Terminate);

};

#endif /* CTWOPLANTCISDL_H_ */
