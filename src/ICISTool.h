/*
 * ICISTool.h
 *
 *  Created on: May 14, 2015
 *      Author: via
 */

#ifndef SRC_ICISTOOL_H_
#define SRC_ICISTOOL_H_
#include "common.h"
#include "CRootTable.h"
#include "CFlash.h"

class ICISTool {
public:
	ICISTool();
	virtual ~ICISTool();
	virtual UINT setBlockMaptoBitMap(CFlash *pflash,CRootTable *roottable,LPMapChipSelect pUFDBlockMap,eMMC_CIS_INFO *pCISInfo) = 0;
};

#endif /* SRC_ICISTOOL_H_ */
