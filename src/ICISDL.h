/*
 * ICISDL.h
 *
 *  Created on: Apr 19, 2015
 *      Author: vli
 */

#ifndef ICISDL_H_
#define ICISDL_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "common.h"
#include "CRootTable.h"
#include "CFlash.h"
#include "ICISTool.h"

class ICISDL {
protected:
	ICISTool *pmCisTool;
public:
	ICISDL();
	virtual ~ICISDL();
	virtual UINT Execute(SettingConfgInfo *pCurSettingConfgInfo,
			CFlash			*pflash,
			CRootTable		*pRootTable,
			eMMC_CIS_INFO *pCISInfo,
			UINT 			*eCISADDR,
			UINT			*Original_EraseCnt,
			UINT			Terminate)=0;
};

#endif /* ICISDL_H_ */
