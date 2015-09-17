#ifndef ICOMMON_H_
#define ICOMMON_H_
#include <iostream>

using namespace std;

#define BOOL bool
#define BYTE unsigned char
#define UINT unsigned int
#define USHORT unsigned short
#define ULONG unsigned long
#define UCHAR unsigned char
#define INT  int
#define TCHAR  char
#define DWORD  unsigned long
#define WORD   long
#define ULONGLONG unsigned long long
#define MAX_PATH 32767
#define LPVOID void *


#if !defined(LOBYTE)
#define LOBYTE(w)	((unsigned char)(w))
#endif

#if !defined(HIBYTE)
#define HIBYTE(w)	((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))
#endif


#if !defined(LOWORD)
#define LOWORD(d)           ((unsigned short)(d))
#endif

#if !defined(HIWORD)
#define HIWORD(d)           ((unsigned short)((((unsigned long)(d)) >> 16) & 0xFFFF))
#endif

typedef struct _F_TABLE_ADDR
{
	BYTE        Chip;
	BYTE        Page;
	USHORT      Block;
}F_TBL_A, *LPF_TBL_A;

typedef struct _TAT_INFO
{
	F_TBL_A     LFT_A[512];
}TAT_INFO, *LPTAT_INFO;

typedef struct _SaveBadBlockInfo // Sherlock_20120130
{
	UINT	BBIName[3];
	UINT	BBICnt;
	UINT	BBIAddr[1020];
}SaveBadBlockInfo, *LPSaveBadBlockInfo;

typedef struct _Flash_FwScheme
{
	UINT   FLH_ID[8];

	BYTE   Model0;
	BYTE   Model1;
	BYTE   Model2;
	BYTE   Model3;
	BYTE   Model4;
	BYTE   Model5;
	BYTE   Model6;
	BYTE   Model7;

	USHORT LessBlock;	// Extended Block
	UINT LessPage;
	UINT BlockPage;
	BYTE   PageSEC;
	BYTE   SelectPlane;
	UINT PlaneBlock;

	BYTE   Rsv1[6];	// [0~3]:As SettingCapacity & Return RealCapacity
					// [4]: Flag of New FW Algorithm, 0:Old, 1:New

	BYTE   SelectNO;
	BYTE   ChannelNO;
	BYTE   InterleaveNO;
	BYTE   Rsv2[13];

	BYTE   VB_Block;
	BYTE   Select_VB;
	BYTE   WCT_VB;
	BYTE   Rsv3[2];
	BYTE   TAT_NO;
	USHORT LFT_NO;
	USHORT LFT_VB;
	USHORT LFT_RsvVB;
	UINT   Capacity;
}Flash_FwScheme, *LPFlash_FwScheme;



typedef struct _RTblInfo
{
	ULONG	RAWAddr0;	// RAW Address 0
	ULONG	RAWAddr1;	// RAW Address 1
	USHORT	PairAddr;	// Pair Block Addr From FW
	BYTE	PU_Index;	// PU Index From FW
	BYTE	CellType;	// 0: SLC Type		1: MLC Type
	BYTE	ECCType;	// 0: Defaut ECC		1: 60-bits ECC
	BYTE	Exist;		// 0: RT Not Exist, 		1: RT is Exist
	BYTE	IgnoreRT;	// 0: Normal Mode		1: Ignore RT
}RTblInfo, *LPRTblInfo;

#define	MAX_ZONE			256
#define	MAX_CACHE_NO		32
#define	MAX_MTBL_BLK_NO	32
#define	QDEPTH_GBLK		5

typedef struct
{
	USHORT	wBlkAdr;
	USHORT	wCap;
	USHORT	wEraCnt;
}CBUF_VARS;

typedef struct
{
	USHORT	wfmlcmaxcnt;
	USHORT	wfmlcmaxblk;
	USHORT	wfslcmaxcnt;
	USHORT	wfslcmaxblk;
}SWL_VARS;

typedef struct
{
	BYTE	chResvPtr;
	BYTE	chResvDepth;
	BYTE	cdReaValidBit;
	BYTE	r1;
	USHORT	wBlk[QDEPTH_GBLK];
	USHORT	wEra[QDEPTH_GBLK];
}FB_VARS;

typedef struct
{
	BYTE	chAgeTegMLC;
	BYTE	chAgeTegSLC;
	USHORT	wThreLow;
	USHORT	wThreHigh;
	USHORT	wAllSlcBlkCnt;
	USHORT	wAllMlcBlkCnt;
	USHORT	chMlcAccCnt;
	USHORT	chSlcAccCnt;
	USHORT	wCount[3];
	UINT	dwTotalMlcEraCnt;
	UINT	dwTotalSlcEraCnt;
}DWL_VARS;

typedef struct _ROOT_VARS
{
	UINT	dwBlkBitMapTbl[262];			// 1024B, 2048blk * (4bits) = 1024B = 256 DW
	BYTE	chBlkTagBitMapTbl[262];			// 256B, 2048blk * (1bits) = 256B		==> Cell_Map[], 0xFF
	BYTE	chBlkECCBitMapTbl[262];			// 256B, 2048blk * (1bits) = 256B		==> ECC_Map[], 0x00
	USHORT	wGEraMinCnt[MAX_ZONE];			// 512B, 256 Zone * 2B
//	Cache relative parameters				// cache Tbl:((16+16+2)*4B) * 16 = 128B+2048;
//	Cache relative parameters(no BBM)		// cache Tbl:((2)*4B) * 16 = 128B;
	CBUF_VARS CBufT[MAX_CACHE_NO];
//	M4k Link Tbl Address
	UINT	dwLinkM4K[MAX_ZONE];			// 1K, 256Zone * 4B
	USHORT	wRTBlk[8];
//	Misc.									// others:8B
	BYTE	chWPtrBlkIdx;
	BYTE	chMrgBlkIdx;
	USHORT	wFreeSlcBlkCnt;
	UINT	dwLinkC4K;						// C4k Link Tbl Address
//	Misc data for power cycling				//36B + (MAX_MTBL_BLK_NO*4)
	BYTE	chPu;
	BYTE	PP_Counter;
	BYTE	chNxtM4kNo;
	BYTE	chRTbl_StoreIDX;
	USHORT	wSlcBlkBitMapSrchPnt;
	USHORT	wMlcBlkBitMapSrchPnt;
	USHORT	wFreeMlcBlkCnt;
	USHORT	TagEraCnt;
	USHORT	TagBlk;
	USHORT	TagPge;
	USHORT	wRTblEraCnt;
	USHORT	wCTblEraCnt;
	USHORT	wOldRTblEraCnt;
	USHORT	wOldCTblEraCnt;
	USHORT	wOldCTblBlk;
	USHORT	wOldRTblBlk;
	USHORT	wMTblEraCnt[MAX_MTBL_BLK_NO];
	USHORT	wMTblBlk[MAX_MTBL_BLK_NO];
	UINT	dwMTblWPgePtr;
	UINT	dwCTblWPgePtr;
	UINT	dwRTblWPgePtr;
	UINT	dwLastRTblWPgePtr;
	UINT	dwNextRTblWPgePtr;
	USHORT	wNextRTblEraCnt;
	USHORT	wCISWPgePtr;
	DWL_VARS	DWL;
	SWL_VARS	SWL;
	FB_VARS		FB[3];
//Remained, 4096 = 7K - 256*4(chBlkTagBitMapTbl) - 256(chBlkTagBitMapTbl) -256(chBlkECCBitMapTbl) - 256*2(wGEraMinCnt) - 1024(dwLinkM4K)
    BYTE	Revs[4060-MAX_CACHE_NO*sizeof(CBUF_VARS)-sizeof(DWL_VARS)-sizeof(SWL_VARS)-3*sizeof(FB_VARS)-MAX_MTBL_BLK_NO*4-20*4];
//	OLD BYTE	Revs[4096-MAX_CACHE_NO*sizeof(CBUF_VARS)-sizeof(DWL_VARS)-sizeof(SWL_VARS)-MAX_MTBL_BLK_NO*4-15*4-2*sizeof(FB_VARS)-4];
//Remained, 4352 = 7K - 256*4(chBlkTagBitMapTbl) - 256(chBlkTagBitMapTbl) - 256*2(wGEraMinCnt) - 1024(dwLinkM4K)
//	BYTE Revs[4352-MAX_CACHE_NO*sizeof(CBUF_VARS)-sizeof(DWL_VARS)-sizeof(SWL_VARS)-3*sizeof(FB_VARS)-MAX_MTBL_BLK_NO*4-20*4];
	UINT	dwRTBLVersion;
} ROOT_VARS, *LPROOT_VARS;

typedef struct _BlockRec
{
	BYTE	CHIndex;
	BYTE	CEIndex;
	UINT	EntryIndex;
	BYTE	BlockIndex;
	UINT	EraseCount;
	UINT	Plane0EntryIndex;
	UINT	Plane1EntryIndex;
	UINT	Plane2EntryIndex;
	UINT	Plane3EntryIndex;
	BYTE	Plane0BlockIndex;
	BYTE	Plane1BlockIndex;
	BYTE	Plane2BlockIndex;
	BYTE	Plane3BlockIndex;
}BlockRec, *LPBlockRec;

typedef struct _BlockRecTable
{
	UINT        ItemNum;
	LPBlockRec  pBlockRec;
}BlockRecTable, *LPBlockRecTable;

typedef struct _TableCFG
{
	BYTE	MaxChipFind;
	ULONG	MaxPageFind;
	BYTE	MaxVaildCISBlockFind;
	BYTE	MaxTableChipFind;
	BYTE	MaxLogBlock;	// ex:8
	BYTE	MaxFreeBlock;	// ex:16
	BYTE	SelectPlane;		// Plane No in 1 Chip, ex:2 (New)
	BYTE	InternalChip;	// Internal Chip(CE), ex: 1(New)
	USHORT	BasicBlock;		// From Model_5 (New)
	USHORT	ExtendedBlock;	// LessBlock (New)
	USHORT	AddressPage;	// BlockPage, Must Be 2's Exponentiation (New)
	USHORT	LessPage;		// Actual Page of Capacity, ex:258, 192
	BYTE	PageSEC;
	BYTE	ErrorBitRate;
	BYTE	ECCSET;
	BYTE	Type;			// Original Type
	UINT	BaseFType;		// FlashType, Same As MPTool Using (New)
	UINT	Mode;			// ToolMode (New)
	UINT	ED3EraseCount;	// For ED3 Erase Count
	UINT	RsvInfo;

}TableCFG, *LPTableCFG;

typedef struct _eMMC_CIS_INFO
{
	USHORT	CheckSum;		// 0000-0001
	BYTE	CE_NUM;		// 0002
	BYTE	CH_NUM;		// 0003
	BYTE	FLH_DRV;		// 0004
	BYTE	CTL_DRV;		// 0005
	BYTE	CPU_CLK;		// 0006
	BYTE	Encryption;		// 0007
	USHORT	PlaneBlock;		// 0008-0009
	USHORT	TP_NUM;		// 000A-000B
	BYTE	eMMC_RegInfo;	// 000C
	BYTE	FLH_INFO;		// 000D
	BYTE	FW_PAGE;		// 000E
	BYTE	CIS_PAGE;		// 000F
	BYTE	PAGE_MODE;		// 000F
	BYTE	PreMP_MODE;		// 000F
	BYTE	Sorting_MODE;		// 000F
	BYTE	Reserved0[13];	// 0010-001F
	BYTE	PRE_LOAD_DATA;	// 0x20
	BYTE	CIS_Version;		// 0x21
	BYTE	EACH_PAGE;			// 0022
	BYTE	Reserved2[7];		// 0023-0029
	USHORT	CheckSumLen;		// 002A-002B
	BYTE	Reserved3[4];		// 002C-002F
	BYTE	Reserved4[32];		// 0030-004F
	BYTE	Reserved5[16];	// 0050-005F
	BYTE	Reserved6[16];	// 0060-006F
	BYTE	Reserved7[16];	// 0070-007F
	BYTE	Reserved8[16];	// 0080-008F
	BYTE	Reserved9[16];	// 0090-009F
	BYTE	Reserved10[16];	// 00A0-00AF

	UINT    TotalMlcEraCnt[4];
	UINT    TotalSlcEraCnt[4];
	UINT	CIS_RowAddr[4];	// 00D0-00DF
	USHORT	TurboPage[256];	// 00E0-02DF
	BYTE	CID[16];			// 02E0 -02EF
	BYTE	CSD[16];			// 02F0-02FF
	BYTE	Ext_CSD[512];	// 0300-04FF
	BYTE	Bit_Map[0x1580];		// 0500-1A7F
	BYTE	Cell_Map[0x0AC0];	// 1A80-253F
	BYTE	ECC_Map[0x0AC0];	// 2540-2FFF
}eMMC_CIS_INFO, *LPeMMC_CIS_INFO;

typedef union _MapEntryItem
{
	struct{
			BYTE Block0:1;
			BYTE Block1:1;
			BYTE Block2:1;
			BYTE Block3:1;
			BYTE Block4:1;
			BYTE Block5:1;
			BYTE Block6:1;
			BYTE Block7:1;
			}MapToBit;

	BYTE MapToByte;
}MapEntryItem, *LPMapEntryItem;

#define MaxChannelNo    0x04
#define MaxChipSelectNo 0x04
#define MaxEntryItemNo  0x2000
#define MAX_DISK_ITEM 32

typedef struct _MapChannel
{
	//LPMapEntryItem CEItem[MaxChipSelectNo];
	LPMapEntryItem ChannelItem[MaxChannelNo];

}MapChannel, *LPMapChannel;

typedef struct _MapChipSelect
{
	BYTE	ChannelNum;
	BYTE	ChipSelectNum;
	INT		EntryItemNum; // HIWORD: Extended Block (From LessBlock), LOWORD: Entry
	UINT	BadBlockCnt[MaxChipSelectNo*MaxChannelNo];
	UINT	SysBlkAdr[3076];	// Sherlock_20140730 #if(eMMC_CFG_A2CMD == 1), Reserve Last 4 UINT For Future Use
	UINT	EccErrBlkAdr[512];	//Cody 20150225
	LPMapChannel CEItem[MaxChipSelectNo];
}MapChipSelect, *LPMapChipSelect;

typedef struct _MapUFDBlock
{
	BYTE         UFDNum;
	BYTE         ChannelNum;
	BYTE         ChipSelectNum;
	//LPMapChannel UFDItem[MAX_DISK_ITEM];
	LPMapChipSelect UFDItem[MAX_DISK_ITEM];
}MapUFDBlock, *LPMapUFDBlock;

typedef struct _VendorCMD
{
	BYTE   OPCode;	// 0
	BYTE   MJCMD;	// 1
	ULONG  Address;	// 2~5
	BYTE   MICMD;	// 6
	USHORT BufLen;	// 7~8
	USHORT COLA;	// 9~10
	BYTE   CFG[5];	// 11~15
}VendorCMD, *pVendorCMD;

typedef struct _TurboPageInfo
{
	USHORT 	TurboPage[256];
	USHORT 	TurboPageNUM;
	USHORT 	TurboPageType;
}TurboPageInfo, *LPTurboPageInfo;

typedef struct _SPARETYPE
{
	BYTE SPARE0;
	BYTE SPARE1;
	BYTE SPARE2;
	BYTE SPARE3;
	BYTE SPARE4;
	BYTE SPARE5;
}SPARETYPE, *LPSPARETYPE;

typedef struct _CIS_INFO
{
	USHORT CheckSum;
	BYTE   CIS_MARK[6];
	BYTE   LED_CTL0;
	BYTE   LED_CTL1;
	BYTE   MaxPower;
	BYTE   USB_MaxLUN;
	UINT USB_VID;
	UINT USB_PID;
	UINT   LunStartLBA[4];
	UINT   LunCapacity[4];
	BYTE   LunAttribute[4];
	UINT   LunType[4];
	UINT   SysStartLBA;
	UINT   SysCapacity;
	BYTE   Manufacture[32];
	BYTE   Product[32];
	BYTE   SerialNumber[16];
	BYTE   PassWordKey[16];
	BYTE   Reserved1[32]; // [0~15]:UUID
	BYTE   Inquiry_Head8B[8];
	BYTE   Inquiry_VID[8];
	BYTE   Inquiry_PID[16];
	BYTE   Inquiry_REV[4];
	BYTE   Inquriy_VDR[20];
	BYTE   Reserved2[8];	// [0]:EEPROM, [4]:CPUCLK, [5]:Encryption
}CIS_INFO, *LPCIS_INFO;

typedef struct _CIS_INFO_EX
{
	Flash_FwScheme FLH_FwScheme;
	BYTE	Reserved3[0x40]; // ImageSize[4]
	UINT	CIS_RowAddr[2];
	UINT	ISP_RowAddr[2];
	BYTE	Reserved4[0x70];
	BYTE	Reserved5[0x200]; // [0~0xFF]:TPMTTable[0x100], [0x100~0x101]:TMPTLen[2]
//	BYTE	Reserved6[0x400]; // Add 1k Byte For eMMC Use
}CIS_INFO_EX, *LPCIS_INFO_EX;

typedef struct _SerialNumberInfo
{
	ULONGLONG SerialNumber;
	UINT   SNLength;
	int       Interval;
	int       SNType;
	int       DecCheck;
	string   SNMask;
}SerialNumberInfo, *LPSerialNumberInfo;

typedef struct _SettingConfgInfo
{
	string FWFileName;
	string BLBinFileName;
	string USB_VID;
	string USB_PID;
	string HUB_VID;
	string HUB_PID;
	string SourcePublicLunPath;
	string ImagePublicLunPath;
	string SourceSecurityLunPath;
	string ImageSecurityLunPath;
	string SourceIso9660LunPath;
	string ImageIso9660LunPath;
	string SourceHiddenLunPath;
	string ImageHiddenLunPath;
	string LogFilePath;
	string FWBackupFileName;
	string VolumeLable[4];
	BYTE    ImageType[4];
	BYTE    FormatLunBitMap;
	string PassWordStr;
	string CIS_Mark;
	string Inquiry_VID;
	string Inquiry_PID;
	string ProductStr;
	string ManufactureStr;

	UINT	TestProcedureMask;
	int		TargetStoreMedia;
	int		FWDLRetryCnt;
	int		CISDLRetryCnt;
	ULONGLONG	LunTypeBitMap;
	UINT	LunTypeBitMap2;
	CIS_INFO CisData;
	CIS_INFO_EX CisDataEx;
	SerialNumberInfo SNInfo;
	BYTE	m_CurAlias[64];
	string	BootCodeFileName;
	string	VenCIDFileName;
	string	VenCSDFileName;
	string	ExtCSDFileName;
	BYTE	ForceCE;
	BYTE	ForceCH;
	UINT	IndicateCapacity;
	UINT	StressTestSize;
	ULONGLONG	SupportFlashIndex;
	UINT	U2HUB_VIDPID;
	UINT	U3HUB_VIDPID;
	BYTE	Hub_Port_Num;

	string	Inquiry_REV;
	BYTE	BadColumnSwitch;
	BYTE	FlashDriving;
	BYTE	ControllerDriving;
	BYTE	SSC;
	BYTE	AutoFWSwitch;		// 20130121
	BYTE	MaxCapforPublicLun;	// 20130121
	BYTE	ForceSamsungDDR;	// 20130125
	BYTE	ForceSanDiskDDR;	// 20130125
	BYTE	ForceToshibaDDR;	// 20130125
	BYTE	ARFlashType;		// 20130222

	BYTE	MLCSortingECC;		// 20130307
	BYTE	TLCSortingECC;		// 20130307
	BYTE	PlaneNum;			// 20130916
	BYTE	PortPowOffTime;		// 20131025, Hub Down Port Power Off Time
	BYTE	PortPowOnDelay;	// 20131106, Hub Down Port Power On Enum Delay
	BYTE	SortingClearEraseCount;	// 20140819
	BYTE    AddExtendedBlock;
}SettingConfgInfo, *LPSettingConfgInfo;

typedef struct _FlashStructure
{
	UINT	BlockPage;
	UINT	PageSize;
	BYTE	PlaneNum;
	UINT 	ChipSelectNum;
	BYTE 	ChannelNum;
	UINT 	EntryItemNum;
	WORD 	BaseFType;
	BYTE	ForceCE;
	BYTE	ForceCH;
	Flash_FwScheme *FlashFwScheme;
	TurboPageInfo *turbopageinfo;
}FlashStructure, *LPFlashStructure;

typedef struct _SCSI_PASS_THROUGH {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    //ULONG_PTR DataBufferOffset;
    ULONG DataBufferOffset;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;


typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             ucSenseBuf[32];
    UCHAR             ucDataBuf[65536];  //64K
    } SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


#define VT3468				0x3468	//Chip is VT3468

#define BIT0   0x00000001
#define BIT1   0x00000002
#define BIT2   0x00000004
#define BIT3   0x00000008
#define BIT4   0x00000010
#define BIT5   0x00000020
#define BIT6   0x00000040
#define BIT7   0x00000080
#define BIT8   0x00000100
#define BIT9   0x00000200
#define BIT10  0x00000400
#define BIT11  0x00000800
#define BIT12  0x00001000
#define BIT13  0x00002000
#define BIT14  0x00004000
#define BIT15  0x00008000
#define BIT16  0x00010000
#define BIT17  0x00020000
#define BIT18  0x00040000
#define BIT19  0x00080000
#define BIT20  0x00100000
#define BIT21  0x00200000
#define BIT22  0x00400000
#define BIT23  0x00800000
#define BIT24  0x01000000
#define BIT25  0x02000000
#define BIT26  0x04000000
#define BIT27  0x08000000
#define BIT28  0x10000000
#define BIT29  0x20000000
#define BIT30  0x40000000
#define BIT31  0x80000000

#define Fail_State			0x00
#define Success_State		0x01
#define ReStart_State			0x02
#define Open_FW_File_Error			0x03
#define Read_FW_File_Error			0x04
#define FWandIC_NotMatch_Error		0x05
#define FW_DownLoad_Error			0x06
#define FW_UpLoad_Error				0x07
#define FW_Compare_Error			0x08
#define EEPROM_DownLoad_Error		0x09
#define EEPROM_UpLoad_Error		0x0A
#define SNGENEATER_Error			0x0B
#define EEPROM_Compare_Error		0x0C
#define Open_FT_Image_Error		0x0D
#define Write_PublicLun_Image_Error		0x0E
#define Write_SecurityLun_Image_Error	0x0F
#define Write_Iso9660Lun_Image_Error	0x10
#define Write_HiddenLun_Image_Error		0x11
#define PublicLun_NTFS_Format_Error		0x12
#define SecurityLun_NTFS_Format_Error	0x13
#define Iso9660Lun_NTFS_Format_Error	0x14
#define HiddenLun_NTFS_Format_Error	0x15
#define CIS_Address_Error			0x16
#define ISP_Address_Error			0x17
#define Stress_Test_Error				0x18
#define Only_Scan_Block				0x19
#define Need_Add_Offset				0x1A
#define Need_FW_Download_Again	0x1B
#define Open_BootCode_File_Error	0x1C
#define Read_BootCode_File_Error		0x1D
#define BootCode_DownLoad_Error	0x1E
#define BootCode_UpLoad_Error		0x1F
#define BootCode_Compare_Error		0x20
#define Block_Erase_Error			0x21
#define INIT_ISP_FW_Error			0x22
#define CPU_Reset_Error				0x23
#define Build_UFDSystemTable_Error	0x24
#define Scan_Block_Error				0x25
#define FTImage_Type_Error			0x26
#define SettingCap_OverSize_Error	0x27
#define Format_OverSize_Error		0x28
#define Read_Capacity_Error			0x29
#define SettingCap_Real_Error		0x2A
#define FlashType_NotSupport_Error	0x2B
#define ReadHDSerialNumber_Error	0x2C
#define Disk_Lock_Error				0x2D
#define Chip_Reset_Error				0x2E
#define Channel_1_Error				0x2F
#define Channel_2_Error				0x30
#define Channel_3_Error				0x31
#define Channel_4_Error				0x32
#define ChipSelect_1_Error			0x33
#define ChipSelect_2_Error			0x34
#define ChipSelect_3_Error			0x35
#define ChipSelect_4_Error			0x36
#define Disk_UnLock_Error			0x37
#define SPIFlash_NotSupport_Error	0x38
#define SPIFlash_Erase_Error			0x39
#define FW_Ver_Not_Match			0x3A
#define ISP_Ver_Not_Match			0x3B
#define Real_Capacity_Not_Enough	0x3C
#define Format_LessSize_Error		0x3D
#define INIT_VDR_FW_Error			0x3E
#define INIT_1ST_FW_Error			0x3F
#define PreMP_Scan_Error			0x40
#define PreMP_Bad_Block_Over		0x41
#define Register_Read_Error			0x42
#define Register_Write_Error			0x43
#define NandBoot_FW_Compare_Error	0x44
#define FreeBlock_Not_Enough		0x45
#define FW_Type_Not_Match			0x46
#define AutoFW_Not_Found			0x47
#define Not_Support_ARgrade			0x48
#define No_Good_Block_For_CIS		0x49
#define Flash_Sorting_Error			0x4A
#define CIS_Read_Error				0x4B
#define Illegal_CIS_Define			0x4C
#define Find_CIS_Error				0x4D
#define ReStart_Device_Fail			0x4E
#define CIS_CheckSum_Error			0x4F

#define FID_Chk_Len			6		// Flash ID Check Length, Not Check All 8 Bytes, Sherlock_20111019

// Use in "BaseFType"
// ----- New Define Of BaseFType -----
#define	FT_Vendor			0xF000	// Bit_15~Bit_12
#define		FT_Samsung		0x1000
#define		FT_Hynix		0x2000
#define		FT_Micron		0x3000
#define		FT_Intel			0x4000
#define		FT_Toshiba		0x5000
#define	FT_Type			0x0F00	// Bit_11~Bit_8
#define		FT_NotUse1		0x0800	// 	Bit_11
#define		FT_NotUse2		0x0400	//	Bit_10
#define		FT_IntrChip		0x0200	// 	Bit_9
#define		FT_UnAligned	0x0100	// 	Bit_8
#define	FT_Cell_Level		0x00F0	// Bit_7~Bit_4
#define		FT_SLC			0x0010
#define		FT_MLC			0x0020
#define		FT_TLC			0x0030
#define	FT_Plane_No		0x000F	// Bit_3~Bit_0
#define		FT_2Plane		0x0002
#define		FT_4Plane		0x0004




#define Samsung_M_2P		(FT_Samsung|			FT_MLC|FT_2Plane)	// = 0x1022
#define Samsung_M_4P		(FT_Samsung|			FT_MLC|FT_4Plane)	// = 0x1024
#define Samsung_T_2P		(FT_Samsung|			FT_TLC|FT_2Plane)	// = 0x1032
#define Samsung_T_4P		(FT_Samsung|			FT_TLC|FT_4Plane)	// = 0x1034
#define Samsung_U_M_2P		(FT_Samsung|FT_UnAligned|FT_MLC|FT_2Plane)	// = 0x1122
#define Samsung_U_M_4P		(FT_Samsung|FT_UnAligned|FT_MLC|FT_4Plane)	// = 0x1124
#define Samsung_U_T_2P		(FT_Samsung|FT_UnAligned|FT_TLC|FT_2Plane)	// = 0x1132
#define Samsung_U_T_4P		(FT_Samsung|FT_UnAligned|FT_TLC|FT_4Plane)	// = 0x1134
#define Samsung_I_M_2P		(FT_Samsung|FT_IntrChip|	FT_MLC|FT_2Plane)	// = 0x1222, K9LCG08U0A(New), K9HDG08U1A
#define Samsung_I_M_4P		(FT_Samsung|FT_IntrChip|	FT_MLC|FT_4Plane)	// = 0x1224
#define Samsung_I_T_2P		(FT_Samsung|FT_IntrChip|	FT_TLC|FT_2Plane)	// = 0x1232
#define Samsung_I_T_4P		(FT_Samsung|FT_IntrChip|	FT_TLC|FT_4Plane)	// = 0x1234

#define Micron_M_2P			(FT_Micron|				FT_MLC|FT_2Plane)	// = 0x3022
#define Micron_M_4P			(FT_Micron|				FT_MLC|FT_4Plane)	// = 0x3024
#define Micron_T_2P			(FT_Micron|				FT_TLC|FT_2Plane)	// = 0x3032
#define Micron_T_4P			(FT_Micron|				FT_TLC|FT_4Plane)	// = 0x3034
#define Micron_I_M_2P		(FT_Micron|FT_IntrChip|	FT_MLC|FT_2Plane)	// = 0x3222
#define Micron_I_M_4P		(FT_Micron|FT_IntrChip|	FT_MLC|FT_4Plane)	// = 0x3224
#define Micron_I_T_2P		(FT_Micron|FT_IntrChip|	FT_TLC|FT_2Plane)	// = 0x3232
#define Micron_I_T_4P		(FT_Micron|FT_IntrChip|	FT_TLC|FT_4Plane)	// = 0x3234

#define Toshiba_M_2P		(FT_Toshiba|				FT_MLC|FT_2Plane)	// = 0x5022
#define Toshiba_M_4P		(FT_Toshiba|				FT_MLC|FT_4Plane)	// = 0x5024
#define Toshiba_T_2P			(FT_Toshiba|				FT_TLC|FT_2Plane)	// = 0x5032
#define Toshiba_T_4P			(FT_Toshiba|				FT_TLC|FT_4Plane)	// = 0x5034
#define Toshiba_I_M_2P		(FT_Toshiba|FT_IntrChip|	FT_MLC|FT_2Plane)	// = 0x5222
#define Toshiba_I_M_4P		(FT_Toshiba|FT_IntrChip|	FT_MLC|FT_4Plane)	// = 0x5224
#define Toshiba_I_T_2P		(FT_Toshiba|FT_IntrChip|	FT_TLC|FT_2Plane)	// = 0x5232
#define Toshiba_I_T_4P		(FT_Toshiba|FT_IntrChip|	FT_TLC|FT_4Plane)	// = 0x5234

#define Normal_M_2P			(						FT_MLC|FT_2Plane)	// = 0x0022
#define Normal_M_4P			(						FT_MLC|FT_4Plane)	// = 0x0024

// Use in "pTableCfg.Type"
#define ClrWCTVB_TYPE		0x01 //bit_0: clear log block and free block
#define DmpForFW_TYPE		0x02 //bit_1: Dump Sys Table into File
#define MustMarkBad_TYPE	0x04 //bit_2: Mark Bad During Build Table
#define Use6KWCT_Type		0x08 //bit_3: Use 6K WCT and New VDR Command (FW>=41)

// Use in "pTableCfg.Mode"
#define Recovery_Mode		0x02 // bit_1: Recovery Mode
#define PreMP_Mode			0x04 // bit_2: PreMP Mode
#define FlashSorting_Mode	0x08 // bit_3: Sorting Mode
#define ARgrade_Mode		0x10 // bit_4: AR grade Mode (One Plane)

// Use in "pTableCfg.RsvInfo"
#define FreeBlkFail_Info		0x01 // bit_0: Free Block Not Enough For FW Use
#define GoodBlkFail_Info		0x02 // bit_1: No Good Block For CIS/ISP Area
#define CISinBadBlock_Info	0x04 // bit_2: For ARgrade, Special "BlockStressTest" that Only Check 1 Page as CIS Blocks

// Use in LunTypeBitMap2
#define BitErrorRate     (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)

// Use in TestProcedureMask
#define	Proc_BuildSysTbl	0x01	// Bit_0
#define	Proc_CleanMPInfo	0x02	// Bit_1
#define	Proc_DisableCISDL	0x04	// Bit_2
#define	Proc_DisableReset	0x08	// Bit_3, Original_Format
#define	Proc_StressTest		0x10	// Bit_4

#define	Proc_HubTestOnly	0x100	// Bit_8
#define	Proc_DLVDROnly		0x200	// Bit_9
#define	Proc_ReStartFWOnly	0x400	// Bit_10

const BYTE Default_CSD[16] = {0x8D,0x00,0x40,0x92,0xEF,0x7F,0xFC,0xE4,0xFF,0x03,0x59,0x7F,0x32,0x02,0x27,0xD0};

const BYTE Default_ExtCSD[512] = {
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x01 ,0x01 ,0x01,
    0x07 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x05 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00,
    0x06 ,0x00 ,0x02 ,0x00 ,0x17 ,0x00 ,0x0A ,0x01 ,0x06 ,0x06 ,0x04 ,0x04 ,0x00 ,0x3C ,0x3C ,0x3C,
    0x3C ,0x3C ,0x3C ,0x00 ,0x33 ,0x33 ,0x73 ,0x00 ,0x00 ,0x17 ,0x00 ,0x00 ,0x00 ,0x01 ,0x01 ,0x01,
    0x08 ,0x07 ,0x01 ,0x00 ,0x05 ,0x01 ,0x01 ,0x50 ,0x01 ,0x00 ,0x50 ,0x28 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x01,
    0x15 ,0x00 ,0x00 ,0x00 ,0x04 ,0x04 ,0x01 ,0x03 ,0x1F ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    };

const BYTE ReadCMD_TSB_2P[27] = {0x02,0x1B,0x0,0x5,0x32,0x0,0x0,0x0,0x0,0x5,0x30,0x0,0x0,0x0,0x3F,0x0,0x5,0x5,0x2,0xE0,0x0,0x0,0x5,0x5,0x2,0xE0,0x0};
const BYTE WriteCMD_TSB_2P[14] = {0x02,0x1B,0x80,0x5,0x11,0x0,0x0,0x0,0x81,0x5,0x10,0x0,0x0,0x0};

#define Scan_ClearSysBlk		0x01 // bit_0: clear system block
#define Scan_CheckBadCnt	0x02 // bit_1: check bad block count
#define Scan_EraseGoodBlk	0x04 // bit_2: erase good block
#define Scan_EraseEncrypt	0x08 // bit_3: Erase Encryption Good Block
#define Scan_ScanOneChip	0x10 // bit_4: Only Scan One Chip
#define Scan_SaveBadBlkInfo	0x20 // bit_5: Save Bad Block Info into Flash
#define Scan_PreMPTestMode	0x40 // bit_6: PreMPTestMode, Skip Encryption Set
#define Scan_DumpMessage	0x80 // bit_7: Dump Message into BadBlockRec.txt

#define AutoTune				BIT31
#define UseSST				BIT30	//for < vt3427B0 version use
#define ForceFstTBBuild		BIT29
#define ClearSysBlock		BIT28
#define ScanBlockOnly		BIT27
#define DumpDebugMemory		BIT26
#define PublicLunOnly		BIT25
#define HDDPowrEanble		BIT24
#define SSCEnable			BIT23
#define ForceSecTBBuild		BIT22
#define DisplayRWRate		BIT21	//0:disable		1:enable
#define CapLedCfg			BIT20	//0:display used	1:display free
//BIT19 ~ BIT16 rev
#define CapFromCis			BIT15	//0:form ini file	1:from Cis
#define MXIC_25L512E		BIT14	//0:disable		1:enable
#define ATMEL				BIT13	//0:disable		1:enable
#define ErassAllBlk			BIT12 	//0:disable		1:enable
#define EraseGoodBlk			BIT11	//0:disable 		1:enable
#define DoFlashSorting		BIT10	//0:disable		1:enable	 20130624
#endif /* ICOMMON_H_ */

//SCSI
#define SCSI_IOCTL_DATA_OUT          0
#define SCSI_IOCTL_DATA_IN           1
#define SCSI_IOCTL_DATA_UNSPECIFIED  2

#define SCSI_VLIVENDOR 0xEE


#define	VDR_AccessMemory    	0x01
#define	VDR_ConfigureLUN    	0x02
#define	VDR_LUN_CONFIG      	0x65
#define	VDR_SECURITY_CONFIG 	0x66
#define	VDR_CACHE_CLEAR     	0x6F
#define	VDR_USD_SETTING     	0x80
#define	VDR_POWER_SETTING   	0x8B
#define	VDR_UFD_SETTING     	0x90
#define	VDR_SPARE_ACCESS    	0x95
#define	VDR_BLOCK_ACCESS    	0x95
#define	VDR_BLOCK_OTHER     	0x95
#define	VDR_ISP_ACCESS      	0x9F
#define	VDR_BLK_MARK        	0x50
#define	VDR_BLK_CHECK       	0x51



//VDR_BLK_MARK
#define MI_READ_STS			0x70
#define MI_MARK_BAD			0x80

#define MaxChekBlockNo 0x200


//VDR_UFD_SETTING
#define MI_INIT_ISP           0x81
#define MI_READ_FLASH_ID      0x82
#define MI_READ_SYS_READY     0x85
#define MI_WRITE_SYS_READY    0x86
#define MI_READ_SYS_ADDRESS   0x8C
#define MI_WRITE_SYS_ADDRESS  0x8D
#define MI_READ_FLASH_METHOD  0x8E
#define MI_WRITE_FLASH_METHOD 0x8F

//VDR_BLOCK_ACCESS
#define MI_BLOCK_READ		0x89
#define MI_BLOCK_WRITE		0x8B
#define MI_WCT_WRITE		0x8C
#define MI_WCT_READ			0x8D
#define MI_BLOCK_ERASE		0x8E

//VDR_AccessMemory MI CMD
#define MI_READ_CODE          0x01
#define MI_READ_DATA          0x02
#define MI_WRITE_DATA         0x03
#define MI_READ_XDATA         0x04
#define MI_WRITE_XDATA        0x05


//VDR_BLK_CHECK
#define MI_CHK_BAD			0x00
#define MI_ERASE_SYS		0x01

//VDR_SPARE_ACCESS
#define MI_SPARE_READ		0x88
#define MI_SPARE_WRITE		0x8A

#define Th58TE67DDJBA4C		0x1
#define Th58TE67DDKBA4C		0x2

#define MaxChipSelect 4

//***************************************************************************
//				 %%% Commands for all Device Types %%%
//***************************************************************************
#define SCSI_CHANGE_DEF	0x40		// Change Definition (Optional)
#define SCSI_COMPARE		0x39		// Compare (O)
#define SCSI_COPY			0x18		// Copy (O)
#define SCSI_COP_VERIFY	0x3A		// Copy and Verify (O)
#define SCSI_INQUIRY		0x12		// Inquiry (MANDATORY)
#define SCSI_LOG_SELECT	0x4C		// Log Select (O)
#define SCSI_LOG_SENSE	0x4D		// Log Sense (O)
#define SCSI_MODE_SEL6	0x15		// Mode Select 6-byte (Device Specific)
#define SCSI_MODE_SEL10	0x55		// Mode Select 10-byte (Device Specific)
#define SCSI_MODE_SEN6	0x1A		// Mode Sense 6-byte (Device Specific)
#define SCSI_MODE_SEN10	0x5A		// Mode Sense 10-byte (Device Specific)
#define SCSI_READ_BUFF	0x3C		// Read Buffer (O)
#define SCSI_REQ_SENSE	0x03		// Request Sense (MANDATORY)
#define SCSI_SEND_DIAG	0x1D		// Send Diagnostic (O)
#define SCSI_TST_U_RDY	0x00		// Test Unit Ready (MANDATORY)
#define SCSI_WRITE_BUFF	0x3B		// Write Buffer (O)

#define VDR_SORTING			0xF8
#define MI_SORT_READECC		0x80
#define MI_SORT_Set3SLCVB	0x82
#define MI_SORT_COPYTLC		0x84
#define MI_SORT_FillFIFO	0x86
#define MI_SORT_COPYMLC		0x88

// Use in TestProcedureMask
#define	Proc_BuildSysTbl	0x01	// Bit_0
#define	Proc_CleanMPInfo	0x02	// Bit_1
#define	Proc_DisableCISDL	0x04	// Bit_2
#define	Proc_DisableReset	0x08	// Bit_3, Original_Format
#define	Proc_StressTest		0x10	// Bit_4

#define	Proc_HubTestOnly	0x100	// Bit_8
#define	Proc_DLVDROnly		0x200	// Bit_9
#define	Proc_ReStartFWOnly	0x400	// Bit_10
#define	Proc_SLCPageMode	0x800 // Bit_11


//Use in LunTypeBitMap

#define SortingFW			BIT19 //0:disable  1:enable sorting FW
#define PreMPFWWait1Sec	BIT18
#define PreMPFW			BIT17
#define NormalFW			BIT16

