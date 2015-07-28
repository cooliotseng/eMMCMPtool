/*
 * CCISTool.h
 *
 *  Created on: May 14, 2015
 *      Author: via
 */

#ifndef SRC_CCISTOOL_H_
#define SRC_CCISTOOL_H_
#include "common.h"
#include "ICISTool.h"



class CCISTool: public ICISTool {
public:
	CCISTool();
	virtual ~CCISTool();
	UINT setBlockMaptoBitMap(CFlash *pflash,CRootTable *roottable,LPMapChipSelect pUFDBlockMap,eMMC_CIS_INFO *pCISInfo);
};

#endif /* SRC_CCISTOOL_H_ */
