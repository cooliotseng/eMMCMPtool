//============================================================================
// Name        : MPTool.cpp
// Author      : Coolio
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "CeMMC.h"
#include "CCISTool.h"
#include "CCISDL.h"
#include "CCIS.h"
#include "CFlashsimplefactory.h"


using namespace std;

extern SettingConfgInfo * initCurSettingConfgInfo();
extern TurboPageInfo *creatTurboPageInfo(SettingConfgInfo *CurSettingConfgInfo);

int main() {
	UINT status;
	CCIS	*pmCIS;
	CFlash *pmflash;
	CRootTable *ptroottable;
	SettingConfgInfo *pmCurSettingConfgInfo;
	CFlashsimplefactory *pflashfactory;
	pmCurSettingConfgInfo = initCurSettingConfgInfo();
	pflashfactory = new CFlashsimplefactory(pmCurSettingConfgInfo);

	//pmflash = pflashfactory->CreatFalsh(Th58TE67DDJBA4C);
	pmflash = pflashfactory->CreatFalsh(Th58TE67DDKBA4C);

	// ====================================================
	// ==         STEP 3/10: DOWNLOAD VDR FW             ==
	// ==                                                ==
	// == 3-1 Download "VDR_FW\EMMC_VDR.bin"             ==
	// ====================================================
   // status = pmflash->DownloadVDRFw("VDRFE.bin");

    if(status != Success_State){
    	return status ;
    }

    // =========================================================================
    // ==         STEP 4/10: SET PAGE SIZE & ECC                              ==
    // ==                                                                     ==
    // == 4-1 Get Block Size & Page Size                                      ==
    // == 4-2 Set BlockPage, PageSize & ECC of Reg[0x1FF82600] [0x1FF82608]   ==
    // =========================================================================

    status = pmflash->resetEcc();

    if(status != Success_State){
   //coolio   	return status ;
    }

    status = pmflash->setFlashSize();

    if(status != Success_State){
       	return status ;
     }

    // ==========================================================
    // ==    STEP 5/10: SET ERASE COUNT OF ROOT TABLE          ==
    // ==                                                      ==
    // == 5-1 Set Turbo Pages For VDR Use                      ==
    // == 5-2 Get Root Table Address From VDR                  ==
    // == 5-3 Set Erase Count of Root Table & Cache Blocks     ==
    // ==========================================================

    if(pmflash->isOldVersionCISExit()){

    		goto SKIP_SET_RT_ERASECOUNT;
    }

    status = pmflash->writeTPMT();

    ptroottable = new CRootTable(pmflash);

    ptroottable->updateEraseCount();

SKIP_SET_RT_ERASECOUNT:

    // ====================================================
	// ==          STEP 6/10: SCAN FLASH                 ==
	// ==                                                ==
	// == 5-1 Set Page_Size & ECC_Bit                    ==
	// == 5-2 Scan Flash Block                           ==
	// ====================================================

	if(pmflash->isOldVersionCISExit()){

		status = ptroottable->writeCellMap();
		status = ptroottable->writeEccMap();
		if(status == Fail_State){
			 return status ;
	    }
	    //pmflash->Sorting(0,pmCurSettingConfgInfo);  //option function
	    status = ptroottable->ScanBlock(pmCurSettingConfgInfo);
	    if(status == Fail_State){
	    	return status ;
	    }
	    status = ptroottable->setSystemBlock(pmflash);
  	 }

	// ====================================================
	// ==           STEP 7/10: FIND CIS BLOCK            ==
	// ==                                                ==
	// == 7-1 Prepare TableCfg Data                      ==
	// == 7-2 Find CIS Address                           ==
	// ====================================================

    CCISTool *tCistool = new CCISTool();

    CCISDL *tCisdl = new CCISDL(tCistool);

    pmCIS = new CCIS(tCisdl,pmCurSettingConfgInfo,pmflash,ptroottable,1024);


    // ====================================================
    // ==           STEP 8/10: BUILD eCIS BLOCK          ==
    // ==                                                ==
    // ==  8-1 Build eCIS Block                          ==
    // ====================================================

    pmCIS->DownloadCIS(pmCurSettingConfgInfo,ptroottable);


	return 0;
}
