/*
 * CCISDLD0.h
 *
 *  Created on: Sep 15, 2015
 *      Author: vli
 */

#ifndef SRC_CCISDLD0_H_
#define SRC_CCISDLD0_H_
#include "common.h"
#include "ICISDL.h"
#include "ICISTool.h"


class CCISDLD0: public ICISDL {
private:
	UINT SetSLCPageCIS(CFlash *pflash, USHORT *TurboPage, USHORT *TurboPage_NUM, USHORT *TurboPage_Type);  //Cody_20150420
	UINT SetTPMTeCIS(CFlash *pflash, USHORT *TurboPage, USHORT *TurboPage_NUM, USHORT *TurboPage_Type);
public:
	CCISDLD0(ICISTool * pcistool);
	virtual ~CCISDLD0();
	UINT Execute(SettingConfgInfo *pCurSettingConfgInfo,
				CFlash			*pflash,
				CRootTable		*pRootTable,
				UINT 			*eCISADDR,
				UINT			*Original_EraseCnt,
				UINT			Terminate);

};

#endif /* SRC_CCISDLD0_H_ */
