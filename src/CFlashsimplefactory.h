/*
 * CFlashsimplefactory.h
 *
 *  Created on: 2015¦~6¤ë18¤é
 *      Author: Coolio
 */

#ifndef CFLASHSIMPLEFACTORY_H_
#define CFLASHSIMPLEFACTORY_H_
#include <string>
#include "CFlash.h"
#include "IFlashStructure.h"
#include "CFlashStructureFactory.h"

class CFlashsimplefactory {
private:
	SettingConfgInfo *pmCurSettingConfgInfo;
public:
	CFlashsimplefactory(SettingConfgInfo *pCurSettingConfgInfo);
	virtual ~CFlashsimplefactory();
	CFlash *CreatFalsh(UINT ID);

};

#endif /* CFLASHSIMPLEFACTORY_H_ */
