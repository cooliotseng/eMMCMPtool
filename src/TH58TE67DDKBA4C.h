/*
 * TH58TE67DDKBA4C.h
 *
 *  Created on: 2015¦~6¤ë17¤é
 *      Author: Coolio
 */

#ifndef TH58TE67DDKBA4C_H_
#define TH58TE67DDKBA4C_H_
#include "common.h"
#include "IFlashStructure.h"

class TH58TE67DDKBA4C: public IFlashStructure {
private:
	FlashStructure *pmFlashStructure;
	Flash_FwScheme *pmFlashFwScheme;
public:
	TH58TE67DDKBA4C();
	virtual ~TH58TE67DDKBA4C();
	FlashStructure *CreatFlashStructure(SettingConfgInfo *pCurSettingConfgInfo);
};

#endif /* TH58TE67DDJBA4C_H_ */
