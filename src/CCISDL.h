/*
 * CCISDL.h
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#ifndef CCISDL_H_
#define CCISDL_H_
#include "common.h"
#include "ICISDL.h"
#include "ICISTool.h"

class CCISDL: public ICISDL {
private:

public:
	CCISDL(ICISTool *pCisTool);
	virtual ~CCISDL();
	UINT Execute(SettingConfgInfo *pCurSettingConfgInfo,
			CFlash			*pflash,
			CRootTable		*pRootTable,
			eMMC_CIS_INFO *pCISInfo,
			UINT 			*eCISADDR,
			UINT			*Original_EraseCnt,
			UINT			Terminate);
};

#endif /* CCISDL_H_ */
