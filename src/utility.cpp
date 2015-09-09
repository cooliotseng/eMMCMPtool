#include <stdio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>

extern void OpeneMMCTest(){
	int fd_0, fd_1;
	char Shellbuf[4096];
	string tShellbuf ;
	string mCmdIndexString;
	stringstream streamIndex;
	char *majornum;
	FILE *pp;
	cout <<"insmod vdr_test driver" <<endl;
	if((pp=popen("lsmod | grep mmc_block","r")) == NULL){
				std::cout << "Popen() error: " << std::endl;
	}

	if(fgets(Shellbuf,sizeof(Shellbuf),pp)!=NULL){
		system("echo vli | sudo -S rmmod mmc_block");

	}

	if((pp=popen("cat /proc/devices | grep vdr_test","r")) == NULL){
		std::cout << "Popen() error: " << std::endl;
	}
	majornum = strtok(fgets(Shellbuf,sizeof(Shellbuf),pp)," ");

	if(majornum == NULL){
		system("echo vli | sudo -S insmod mmc_test.ko");
	}

	if((pp=popen("ls /dev/vdr_test*","r")) == NULL){
		std::cout << "Popen() error: " << std::endl;
	}

	if(fgets(Shellbuf,sizeof(Shellbuf),pp)==NULL){

		if((pp=popen("cat /proc/devices | grep vdr_test","r")) == NULL){
						std::cout << "Popen() error: " << std::endl;
			}

			majornum = strtok(fgets(Shellbuf,sizeof(Shellbuf),pp)," ");

			for(int i=0;i<2;i++){
				streamIndex.str("");
				streamIndex << i;
				tShellbuf.assign("echo vli | sudo -S mknod /dev/vdr_test");
				tShellbuf.append(streamIndex.str()).append(" ")
						.append("c ")
						.append(majornum).append(" ")
						.append(streamIndex.str());
				system(tShellbuf.c_str());
			}
	}
	system("echo vli | sudo -S chown vli:vli /dev/vdr_test*");
	pclose(pp);
}

extern void CloseMMCTest(){
	system("ls /dev/vdr_test*");
	system("echo vli | sudo -S rm -f  /dev/vdr_test*");
	system("echo vli | sudo -S rmmod mmc_test");
	system("echo vli | sudo -S insmod /lib/modules/3.19.3/kernel/drivers/mmc/card/mmc_block.ko");
	cout <<"remove vdr_test driver" <<endl;
}


extern ULONG FileSize(FILE *fp) {

	ULONG prev = ftell(fp);
	fseek(fp,0L,SEEK_END);
	ULONG sz = ftell(fp);
	fseek(fp,prev,SEEK_SET);

	return sz;
}

extern UINT BitCount(UINT Value)
{
	Value = (Value >> 1  & 0x55555555) + (Value & 0x55555555);
	Value = ((Value >> 2) & 0x33333333) + (Value & 0x33333333);
	Value = ((Value >> 4) + Value) & 0x0f0f0f0f;
	Value = ((Value >> 8) + Value);
	return (Value + (Value >> 16)) & 0xff;
}

extern UINT getFlashType(SettingConfgInfo *CurSettingConfgInfo) {
	// TODO Auto-generated constructor stub
	WORD	BaseFType = Normal_M_2P;

	Flash_FwScheme *tFLH_FwScheme=&CurSettingConfgInfo->CisDataEx.FLH_FwScheme;
	if(	    (tFLH_FwScheme->FLH_ID[0]==0xEC) && (tFLH_FwScheme->FLH_ID[1]==0xDE)&&(tFLH_FwScheme->FLH_ID[2]==0x95)&&(tFLH_FwScheme->FLH_ID[3]==0x76)&&(tFLH_FwScheme->FLH_ID[4]==0x68))
		BaseFType = Samsung_I_M_2P;	// K9LCG08U0A(New)
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && (tFLH_FwScheme->FLH_ID[1]==0xDE)&&(tFLH_FwScheme->FLH_ID[2]==0xD5)&&(tFLH_FwScheme->FLH_ID[3]==0x7A)&&(tFLH_FwScheme->FLH_ID[4]==0x58))
		BaseFType = Samsung_I_M_2P;	// K9HDG08U1A														BaseFType = Samsung_U_M_4P;	// K9GCGY8S0A
	// ----- Samsung -----
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x04))
		BaseFType = Samsung_I_M_2P;	// Samsung && IntrChip && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x08))
		BaseFType = Samsung_I_M_4P;	// Samsung && IntrChip && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x04))
		BaseFType = Samsung_I_T_2P;	// Samsung && IntrChip && TLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x08))
		BaseFType = Samsung_I_T_4P;	// Samsung && IntrChip && TLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x04))
		BaseFType = Samsung_M_2P;	// Samsung && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x08))
		BaseFType = Samsung_M_4P;	// Samsung && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x04))
		BaseFType = Samsung_T_2P;	// Samsung && TLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0xEC) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0E)==0x08))
		BaseFType = Samsung_T_4P;	// Samsung && TLC && 4_Plane
	// ----- Micron ----- Sherlock_20121128
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x01))
		BaseFType = Micron_I_M_2P;	// Micron && IntrChip && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x02))
		BaseFType = Micron_I_M_4P;	// Micron && IntrChip && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x01))
		BaseFType = Micron_I_T_2P;	// Micron && IntrChip && TLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x03)!=0x00) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x02))
		BaseFType = Micron_I_T_4P;	// Micron && IntrChip && TLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x01))
		BaseFType = Micron_M_2P;		// Micron && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x02))
		BaseFType = Micron_M_4P;		// Micron && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x01))
		BaseFType = Micron_T_2P;		// Micron && TLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x2C) && ((tFLH_FwScheme->FLH_ID[2]&0x0C)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x03)==0x02))
		BaseFType = Micron_T_4P;		// Micron && TLC && 4_Plane
	// ----- Toshiba -----
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))
		BaseFType = Toshiba_M_2P;		// Toshiba && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_M_4P;		// Toshiba && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x05) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_I_M_2P; 	// Toshiba && IntrChip && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x05) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x0C))		BaseFType = Toshiba_I_M_4P; 	// Toshiba && IntrChip && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x00))		BaseFType = Toshiba_T_2P;		// Toshiba && ED3 && 2_Plane (1_District + 1_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))		BaseFType = Toshiba_T_4P;		// Toshiba && ED3 && 4_Plane (2_District + 2_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x09) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))		BaseFType = Toshiba_I_T_2P;		// Toshiba && ED3 && 2_Plane (1_District + 1_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x98) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x09) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_I_T_4P;		// Toshiba && ED3 && 4_Plane (2_District + 2_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))		BaseFType = Toshiba_M_2P;		// Toshiba && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x04) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_M_4P;		// Toshiba && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x05) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_I_M_2P; 	// Toshiba && IntrChip && MLC && 2_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x05) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x0C))		BaseFType = Toshiba_I_M_4P; 	// Toshiba && IntrChip && MLC && 4_Plane
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x00))		BaseFType = Toshiba_T_2P;		// Toshiba && ED3 && 2_Plane (1_District + 1_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x08) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))		BaseFType = Toshiba_T_4P;		// Toshiba && ED3 && 4_Plane (2_District + 2_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x09) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x04))		BaseFType = Toshiba_I_T_2P; 	// Toshiba && ED3 && 2_Plane (1_District + 1_Pseudo)
	else if((tFLH_FwScheme->FLH_ID[0]==0x45) && ((tFLH_FwScheme->FLH_ID[2]&0x0F)==0x09) && ((tFLH_FwScheme->FLH_ID[4]&0x0C)==0x08))		BaseFType = Toshiba_I_T_4P; 	// Toshiba && ED3 && 4_Plane (2_District + 2_Pseudo)

	return BaseFType;
}

extern TurboPageInfo *creatTurboPageInfo(SettingConfgInfo *CurSettingConfgInfo) {
	// TODO Auto-generated constructor stub

	UINT     Index, TPMTLen;
	UINT BaseFType;
	TurboPageInfo *mTurboPageInfo =new TurboPageInfo();
	Flash_FwScheme *tFLH_FwScheme=&CurSettingConfgInfo->CisDataEx.FLH_FwScheme;

	BaseFType = getFlashType(CurSettingConfgInfo);

	BYTE	Type = 1; // Default Type
	BYTE	TC58NVG6D2GTA00[8]	= {0x98, 0xDE, 0x94, 0x82, 0x76, 0x56, 0x01, 0x20};	// Toshiba MLC SDR 24nm
	BYTE	MT29F64G08CBABB[8]	= {0x2C, 0x64, 0x44, 0x4B, 0xA9, 0x00, 0x00, 0x00};	// Micorn L84
	BYTE	MT29F128G08CBCAB[8]	= {0x2C, 0x84, 0x64, 0x3C, 0xA5, 0x00, 0x00, 0x00};	// Micorn L85
	BYTE	MT29F256G08CJABB[8]	= {0x2C, 0x84, 0xC5, 0x4B, 0xA9, 0x00, 0x00, 0x00};	// Micorn L84 2Die
	BYTE	JS29F64G08AAMF1[8]	= {0x89, 0x88, 0x24, 0x4B, 0xA9, 0x84, 0x00, 0x00};	// New L84
	BYTE	MICRON_M73_1[8]		= {0x2C, 0x88, 0x01, 0xA7, 0xA9, 0x00, 0x00, 0x00};	// Micorn M 73
	BYTE	MICRON_M73_2[8]		= {0x2C, 0x68, 0x00, 0xA7, 0xA9, 0x00, 0x00, 0x00};	// Micorn M 73
	BYTE	MT29F256G08CECAB[8]	= {0x2C, 0x84, 0x64, 0x3C, 0xA5, 0x00, 0x00, 0x00};	// Micron DDP 2Die 2CE == L85
	BYTE	MT29F512G08CKCAB[8]	= {0x2C, 0xA4, 0xE5, 0x3C, 0xA5, 0x00, 0x00, 0x00};	// Micron QDP 4Die 2CE
	BYTE	MT29F1T08CUCAB[8]		= {0x2C, 0xA4, 0xE5, 0x3C, 0xA5, 0x00, 0x00, 0x00};	// Micron ODP 8Die 4CE == QDP
	BYTE	MT29F64G08CBCDB[8]	= {0x2C, 0x64, 0x64, 0x3C, 0xA5, 0x04, 0x00, 0x00};	// Micron L84C

	if( memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_1",11)==0)
		Type = 1;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_2",11)==0)
		Type = 2;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_3",11)==0)
		Type = 3;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_4",11)==0)
		Type = 4;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_5",11)==0)
		Type = 5;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_6",11)==0)
		Type = 6;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_7",11)==0)
		Type = 7;	// 20121213
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_8",11)==0)
		Type = 8;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_9",11)==0)
		Type = 9;	// FW Test Use
	else if(memcmp(CurSettingConfgInfo->m_CurAlias,"TPMT_TYPE_10",11)==0)
		Type = 10;	// FW Test Use
	else if(memcmp(&TC58NVG6D2GTA00, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 1; // Toshiba Special Case, Sherlock_20120807
	else if(memcmp(MT29F64G08CBABB,	 CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 6; // New L84
	else if(memcmp(MT29F128G08CBCAB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 7; // Micorn L85
	else if(memcmp(MT29F256G08CJABB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 6; // New L84
	else if(memcmp(JS29F64G08AAMF1, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 6; // New L84
	else if(memcmp(MICRON_M73_1, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 8; // M 73
	else if(memcmp(MICRON_M73_2, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 8; // M 73
	else if(memcmp(MT29F256G08CECAB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 7; // Micron DDP 2Die 2CE
	else if(memcmp(MT29F512G08CKCAB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 7; // Micron QDP 4Die 2CE
	else if(memcmp(MT29F1T08CUCAB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 7; // Micron ODP 8Die 4CE
	else if(memcmp(MT29F64G08CBCDB, CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID, FID_Chk_Len) == 0)
		Type = 11; // Micron L84C
	else if((BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Toshiba|FT_TLC))
		Type = 3; 	// Toshiba ED3
	else if((BaseFType & (FT_Vendor|FT_Cell_Level)) == (FT_Samsung|FT_TLC))
		Type = 4; 	// Samsung TLC
	else if((tFLH_FwScheme->FLH_ID[0]== 0x2C) || (tFLH_FwScheme->FLH_ID[0] == 0x89))
		Type = 1;	// Micron/Intel MLC
	else if((tFLH_FwScheme->FLH_ID[0]== 0x98) || (tFLH_FwScheme->FLH_ID[0] == 0xEC) || (tFLH_FwScheme->FLH_ID[0] == 0x45))
		Type = 2; 	// Toshiba/Samsung/SanDisk MLC
	else
		Type = 2; 	// Default

		if(Type ==1) // Intel Class, JS29F16B08CAME1
		{
			TPMTLen = 128;			// pCISSetDataEx->FLH_FwScheme.BlockPage/2=256/2
			mTurboPageInfo->TurboPage[0] = 0;
			mTurboPageInfo->TurboPage[1] = 1;
			for(Index=0; Index<(TPMTLen-2); Index=Index+2)
			{
				mTurboPageInfo->TurboPage[Index+2] = (USHORT)(2*Index + 2);
				mTurboPageInfo->TurboPage[Index+3] = (USHORT)(2*Index + 3);
			}
		}
		else if(Type == 2) // Toshiba Class, TC58NVG4D2FTA
		{
			TPMTLen = tFLH_FwScheme->BlockPage / 2;

			mTurboPageInfo->TurboPage[0] = 0;
			for(Index=1; Index<TPMTLen; Index++)
			{
				mTurboPageInfo->TurboPage[Index] = (USHORT)(2*Index -1) ;
			}
		}
		else if(Type == 3) // Toshiba ED3 Flash, Sherlock_20110926
		{
			TPMTLen = 86;
			for(Index=0; Index<TPMTLen; Index++)
			{
				mTurboPageInfo->TurboPage[Index] = (USHORT)Index;
			}
		}
		else if(Type == 4) // Samsung TLC Flash, Sherlock_20110926
		{
			TPMTLen = 192;
			for(Index=0; Index<TPMTLen; Index++)
			{
				mTurboPageInfo->TurboPage[Index] = (USHORT)Index;
			}
			TPMTLen = 64; // Special Tony Test Only
		}
		else if(Type == 5) // Micorn L84, Sherlock_20120413
		{
			TPMTLen = 64;
			mTurboPageInfo->TurboPage[0] = 0;
			for(Index=1; Index<TPMTLen; Index++)
			{
				mTurboPageInfo->TurboPage[Index] = (USHORT)(4*Index-1);
			}
		}
		else if(Type == 6) // New L84, Sherlock_20120703
		{
			TPMTLen = 64;
			mTurboPageInfo->TurboPage[0] = 0;
			mTurboPageInfo->TurboPage[63] = 254;
			for(Index=1; Index<(TPMTLen-1); Index=Index+1)
			{
				mTurboPageInfo->TurboPage[Index] 	= (USHORT)(4*Index+4);
			}
		}
		else if(Type ==7) // 20121213
		{
			TPMTLen = 128;
			mTurboPageInfo->TurboPage[0] = 0;
			mTurboPageInfo->TurboPage[1] = 1;
			for(Index=0; Index<(TPMTLen-2); Index=Index+2)
			{
				mTurboPageInfo->TurboPage[Index+2] = (USHORT)(2*Index + 4);
				mTurboPageInfo->TurboPage[Index+3] = (USHORT)(2*Index + 5);
			}
		}
		else if(Type == 8) // M73 Flash, Sherlock_20130111
		{
			TPMTLen = 128;
			for(Index=0; Index<TPMTLen; Index++)
			{
				mTurboPageInfo->TurboPage[Index] = (USHORT)Index;
			}
		}
		else if(Type == 9) // Sherlock_20130319
		{
			TPMTLen = 128;
			mTurboPageInfo->TurboPage[0] = 0;
			mTurboPageInfo->TurboPage[1] = 2;
			mTurboPageInfo->TurboPage[2] = 3;
			mTurboPageInfo->TurboPage[3] = 7;
			for(Index=4; Index<TPMTLen; Index=Index+2)
			{
				mTurboPageInfo->TurboPage[Index]	= (USHORT)(2*Index + 2);
				mTurboPageInfo->TurboPage[Index+1]	= (USHORT)(2*Index + 3);
			}
		}
		else if(Type == 10) // Sherlock_20140321
		{
			TPMTLen = 128;
			mTurboPageInfo->TurboPage[0] = 0;
			mTurboPageInfo->TurboPage[1] = 2;
			for(Index=2; Index<TPMTLen; Index=Index+1)
			{
				mTurboPageInfo->TurboPage[Index]	= (USHORT)(4*(Index - 1));
			}
		}
		else if(Type == 11)
		{
			TPMTLen = 256;
			for(Index=0; Index<6; Index=Index+1)
				mTurboPageInfo->TurboPage[Index] = Index;

			for(Index=6; Index<TPMTLen; Index=Index+2)
			{
				mTurboPageInfo->TurboPage[Index]	= (USHORT)(2*Index - 4);
				mTurboPageInfo->TurboPage[Index+1]	= (USHORT)(2*Index - 4) +1;
			}
		}

		// ----- General Setting for All NAND Flash -----
		for(Index=TPMTLen; Index<256; Index++)				// Set Other Data as 0xFF
			mTurboPageInfo->TurboPage[Index] = (USHORT)0xFFFF;

		mTurboPageInfo->TurboPageNUM = TPMTLen;
		mTurboPageInfo->TurboPageType = Type;
		return mTurboPageInfo;
}


//========================for test=========================================================
extern SettingConfgInfo * initCurSettingConfgInfo() {

	SettingConfgInfo *CurSettingConfgInfo= new SettingConfgInfo();

	CurSettingConfgInfo->FWFileName.assign("FW_CODE_RELEASE_20150810_1.bin");
    CurSettingConfgInfo->USB_VID.assign("0BDA");
    CurSettingConfgInfo->USB_PID.assign("0307");
    CurSettingConfgInfo->HUB_VID.assign("2109");
    CurSettingConfgInfo->HUB_PID.assign("0811");
    CurSettingConfgInfo->ImageType[0]=130;
    CurSettingConfgInfo->ImageType[1]=129;
    CurSettingConfgInfo->ImageType[2]=131;
    CurSettingConfgInfo->ImageType[3]=129;
    CurSettingConfgInfo->FormatLunBitMap=1;
    CurSettingConfgInfo->PassWordStr.assign("0123456789ABCDEF");
    CurSettingConfgInfo->CIS_Mark.assign("VD3CIS");
    CurSettingConfgInfo->Inquiry_VID.assign("Generic");
    CurSettingConfgInfo->Inquiry_PID.assign("USB3 Flash Disk");
    CurSettingConfgInfo->ProductStr.assign("VL752");
    CurSettingConfgInfo->ManufactureStr.assign("VIA Labs, Inc");
    CurSettingConfgInfo->TestProcedureMask=56;
    CurSettingConfgInfo->TargetStoreMedia=2;
    CurSettingConfgInfo->LunTypeBitMap = 2420113489;
    CurSettingConfgInfo->LunTypeBitMap2=24;
    CurSettingConfgInfo->BootCodeFileName.assign("FW_CODE_RELEASE_20150810_1.bin");
    CurSettingConfgInfo->VenCIDFileName.assign("");
    CurSettingConfgInfo->VenCSDFileName.assign("");
    CurSettingConfgInfo->ExtCSDFileName.assign("extCSD.bin");
    CurSettingConfgInfo->ForceCE = 4;
	CurSettingConfgInfo->ForceCH = 1;
    CurSettingConfgInfo->SupportFlashIndex=4294967295;
    CurSettingConfgInfo->U2HUB_VIDPID=554248210;
    CurSettingConfgInfo->U3HUB_VIDPID=554240018;
    CurSettingConfgInfo->Hub_Port_Num=4;
    CurSettingConfgInfo->Inquiry_REV.assign("0.00");
    CurSettingConfgInfo->BadColumnSwitch=1;
    CurSettingConfgInfo->MaxCapforPublicLun=1;
    CurSettingConfgInfo->ForceToshibaDDR=1;
    CurSettingConfgInfo->MLCSortingECC=10;
    CurSettingConfgInfo->TLCSortingECC=30;
    CurSettingConfgInfo->PlaneNum=2;
    CurSettingConfgInfo->PortPowOffTime=3;
    CurSettingConfgInfo->PortPowOnDelay=10;
    CurSettingConfgInfo->SortingClearEraseCount=1;
    CurSettingConfgInfo->CisData.CheckSum=0;
    BYTE tCIS_MARK[6] = {'V','D','3','C','I','S'};
    memcpy( &CurSettingConfgInfo->CisData.CIS_MARK,&tCIS_MARK,sizeof(BYTE)*6);
    CurSettingConfgInfo->CisData.LED_CTL0=3;
    CurSettingConfgInfo->CisData.LED_CTL1=6;
    CurSettingConfgInfo->CisData.MaxPower=25;
    CurSettingConfgInfo->CisData.USB_VID=3034;
    CurSettingConfgInfo->CisData.USB_PID=775;
    CurSettingConfgInfo->CisData.LunCapacity[0]=61440000;
    BYTE   tLunAttribute[4] = {96,96,16,96};
    memcpy( &CurSettingConfgInfo->CisData.LunAttribute,&tLunAttribute,sizeof(BYTE)*4);
    UINT   tLunType[4] = {0,129,2,131};
    memcpy( &CurSettingConfgInfo->CisData.LunType,&tLunType,sizeof(BYTE)*4);
    CurSettingConfgInfo->CisData.SysStartLBA=4660;
    CurSettingConfgInfo->CisData.SysCapacity=22136;
    BYTE   tManufacture[32] = "VIA Labs, Inc";
    memcpy( &CurSettingConfgInfo->CisData.Manufacture,&tManufacture,sizeof(BYTE)*32);
    BYTE   tProduct[32] = "VL752";
    memcpy( &CurSettingConfgInfo->CisData.Product,&tProduct,sizeof(BYTE)*32);
    BYTE   tPassWordKey[17] ="0123456789ABCDEF";
    memcpy( &CurSettingConfgInfo->CisData.PassWordKey,&tPassWordKey,sizeof(BYTE)*16);
    BYTE   tInquiry_VID[8] = {'G','e','n','e','r','i','c'};
    memcpy( &CurSettingConfgInfo->CisData.Inquiry_VID,&tInquiry_VID,sizeof(BYTE)*8);
    BYTE   tInquiry_PID[16] ={'U','S','B','3',' ','F','l','a','s','h',' ','D','i','s','k'};
    memcpy( &CurSettingConfgInfo->CisData.Inquiry_PID,&tInquiry_PID,sizeof(BYTE)*16);
    BYTE   tInquiry_REV[5] = "0.00";
    memcpy( &CurSettingConfgInfo->CisData.Inquiry_REV,&tInquiry_REV,sizeof(BYTE)*4);
	BYTE   tReserved2[8] = {0,130,0,0,1,1,2,0};
	memcpy( &CurSettingConfgInfo->CisData.Reserved2,&tReserved2,sizeof(BYTE)*8);
	//UINT   tFLH_ID[8] = {152,222,148,147,118,215,8,4};
	UINT   tFLH_ID[8] = {152,222,148,147,118,80,8,4};
    memcpy( &CurSettingConfgInfo->CisDataEx.FLH_FwScheme.FLH_ID,&tFLH_ID,sizeof(UINT)*8);
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model0=101;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model1=255;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model2=49;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model3=149;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model4=8;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model5=35;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model6=24;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.Model7=0;
    //CurSettingConfgInfo->CisDataEx.FLH_FwScheme.LessBlock=68;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.LessBlock=84;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.LessPage=256;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.BlockPage=256;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.PageSEC=32;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.SelectPlane=2;
    //CurSettingConfgInfo->CisDataEx.FLH_FwScheme.PlaneBlock=1058;
    CurSettingConfgInfo->CisDataEx.FLH_FwScheme.PlaneBlock=1066;
    CurSettingConfgInfo->SNInfo.SerialNumber=5;
    CurSettingConfgInfo->SNInfo.SNLength=16;
    CurSettingConfgInfo->SNInfo.Interval=1;
    CurSettingConfgInfo->SNInfo.SNType=2;
    CurSettingConfgInfo->SNInfo.DecCheck=1;
    CurSettingConfgInfo->SNInfo.SNMask.assign("################");
    //BYTE tCurAlias[64] ={'T','H','5','8','T','E','G','7','D','D','J','B','A','4','C'};
    BYTE tCurAlias[64] ={'T','H','5','8','T','E','G','7','D','D','K','B','A','4','C'};
    memcpy( &CurSettingConfgInfo->m_CurAlias,&tCurAlias,sizeof(BYTE)*64);


    return CurSettingConfgInfo;
}

