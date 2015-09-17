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
extern void OpeneMMCTest();
extern void CloseMMCTest();
extern SettingConfgInfo * initCurSettingConfgInfo();
extern TurboPageInfo *creatTurboPageInfo(SettingConfgInfo *CurSettingConfgInfo);

int main() {
	UINT status;
	CCIS	*pmCIS;
	CFlash *pmflash;
	CRootTable *ptroottable;
	SettingConfgInfo *pmCurSettingConfgInfo;
	CFlashsimplefactory *pflashfactory;

	cout <<"start MP process" <<endl;
	OpeneMMCTest();
	pmCurSettingConfgInfo = initCurSettingConfgInfo();
	pflashfactory = new CFlashsimplefactory(pmCurSettingConfgInfo);

	//pmflash = pflashfactory->CreatFalsh(Th58TE67DDJBA4C);
	pmflash = pflashfactory->CreatFalsh(Th58TE67DDKBA4C);

	// ====================================================
	// ==         STEP 1/6: DOWNLOAD VDR FW             ==
	// ==                                                ==
	// == 3-1 Download "VDR_FW\EMMC_VDR.bin"             ==
	// ====================================================
	cout <<"STEP 1/6: DOWNLOAD VDR FW " <<endl;
    status = pmflash->DownloadVDRFw("EMMC_VDR_2P_MP_D0_TOSHIBA.bin");

    if(status != Success_State){
    	return status ;
    }

    // =========================================================================
    // ==         STEP 2/6: SET PAGE SIZE & ECC                              ==
    // ==                                                                     ==
    // == 4-1 Get Block Size & Page Size                                      ==
    // == 4-2 Set BlockPage, PageSize & ECC of Reg[0x1FF82600] [0x1FF82608]   ==
    // =========================================================================
    cout <<" STEP 2/6: SET PAGE SIZE & ECC " <<endl;
    status = pmflash->resetEcc();

    if(status != Success_State){
   //coolio   	return status ;
    }

    status = pmflash->setFlashSize();

    if(status != Success_State){
       	return status ;
     }

    // ==========================================================
    // ==    STEP 3/6: SET ERASE COUNT OF ROOT TABLE          ==
    // ==                                                      ==
    // == 5-1 Set Turbo Pages For VDR Use                      ==
    // == 5-2 Get Root Table Address From VDR                  ==
    // == 5-3 Set Erase Count of Root Table & Cache Blocks     ==
    // ==========================================================
    cout <<" STEP 3/6: SET ERASE COUNT OF ROOT TABLE " <<endl;
    if(pmflash->isOldVersionCISExit()){

    		goto SKIP_SET_RT_ERASECOUNT;
    }

    status = pmflash->writeTPMT();

    ptroottable = (CRootTable *)new CRootTable(pmflash);

    ptroottable->updateEraseCount();

SKIP_SET_RT_ERASECOUNT:

    // ====================================================
	// ==          STEP 4/6: SCAN FLASH                 ==
	// ==                                                ==
	// == 5-1 Set Page_Size & ECC_Bit                    ==
	// == 5-2 Scan Flash Block                           ==
	// ====================================================
	cout <<" STEP 4/6: SCAN FLASH  " <<endl;
	//if(pmflash->isOldVersionCISExit()){

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
	    status = ptroottable->setEccErrBlock(pmflash);
  	 //}

	// ====================================================
	// ==           STEP 5/6: FIND CIS BLOCK            ==
	// ==                                                ==
	// == 7-1 Prepare TableCfg Data                      ==
	// == 7-2 Find CIS Address                           ==
	// ====================================================
	cout <<"STEP 5/6: FIND CIS BLOCK" <<endl;
    CCISTool *tCistool = new CCISTool();

    CCISDL *tCisdl = new CCISDL(tCistool);

    pmCIS = new CCIS(ptroottable,tCisdl,pmCurSettingConfgInfo,pmflash,ptroottable,1024);


    // ====================================================
    // ==           STEP 6/6: BUILD eCIS BLOCK          ==
    // ==                                                ==
    // ==  8-1 Build eCIS Block                          ==
    // ====================================================
    cout <<"STEP 6/6: BUILD eCIS BLOCK" <<endl;
    pmCIS->DownloadCIS(pmCurSettingConfgInfo,ptroottable);

    CloseMMCTest();
    cout <<"end MP process" <<endl;
	return 0;
}
