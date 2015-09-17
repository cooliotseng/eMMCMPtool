/*
 * CeMMCDriver.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: vli
 */
#include <string.h>
#include "CeMMCDriver.h"
#include <stddef.h>
#include <unistd.h>
CeMMCDriver::CeMMCDriver() {
	// TODO Auto-generated constructor stub
	pmPackedCommand = new CPackedCommand();
}

CeMMCDriver::~CeMMCDriver() {
	// TODO Auto-generated destructor stub
}

UINT CeMMCDriver::readData(ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;

	Status=AccessMemoryRead(MI_READ_DATA, 0, 0, Address, BufLen, buffer);

	return Status;
}

UINT CeMMCDriver::writeData(ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;

	Status=AccessMemoryWrite( MI_WRITE_DATA, 0, 0, Address, BufLen, buffer);

	return Status;

}

UINT CeMMCDriver::enableMPFunction() {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;

	BYTE	Register = 0xff;
	Status = readData(0x1FF8001C,1,&Register);

	if (Status == Fail_State){
		return Status;
	}
	Register|=0x00000010 ; // Enable Write
	Status = writeData(0x1FF8001C,1,&Register);

	return Status;
}


UINT CeMMCDriver::BlockAccessRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_BLOCK_ACCESS;
	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));
	sptwb.spt.Cdb[6] = MI_CMD;
	sptwb.spt.Cdb[7] = PageSec;
	sptwb.spt.Cdb[8] = (UCHAR)(BufLen/512);

	sptwb.spt.Cdb[9] = ECC; // Sherlock_20131106, Bit_7 is Encryption Switch


// Sherlock_20140725
	sptwb.spt.Cdb[11] = HIBYTE(BlockPage);
	sptwb.spt.Cdb[12] = LOBYTE(BlockPage);


//	sptwb.spt.Cdb[9] &= (~bit_7); // Sherlock_20131025, Temp For Derek Test

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

//	Sleep(10);		// Sherlock_20130306, Add Delay For BlockWrite & BlockRead, And L85 Must Delay

///////////////////////////////////////////
// Sherlock_20110411
//CString DebugStr;
//OutputDebugString(" BlockAccessRead()=====");
//DebugStr.Format("CDB[7]=%x, CDB[8]=%x, CDB[9]=%x", PageSec, (BufLen/512), ECC);
//OutputDebugString(DebugStr);
///////////////////////////////////////////

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;


	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_READ_DATA,
											UNPACK_STATUS);

	return status;

}


UINT CeMMCDriver::BlockAccessWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
	//OutputDebugString(" BlockAccessWrite()");
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
//	DWORD dwError;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;

	sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset =
   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_BLOCK_ACCESS;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = PageSec;
	sptwb.spt.Cdb[8] = (UCHAR)(BufLen/512);

	sptwb.spt.Cdb[9] = ECC; // Sherlock_20131106, Bit_7 is Encryption Switch
//	sptwb.spt.Cdb[9] &= (~bit_7); // Sherlock_20131025, Temp For Derek Test

	sptwb.spt.Cdb[10] = Spare.SPARE0;
	sptwb.spt.Cdb[11] = Spare.SPARE1;
	sptwb.spt.Cdb[12] = Spare.SPARE2;
	sptwb.spt.Cdb[13] = Spare.SPARE3;
	sptwb.spt.Cdb[14] = Spare.SPARE4;
	sptwb.spt.Cdb[15] = Spare.SPARE5;

	sptwb.spt.ScsiStatus = 1;    // Sherlock_20121130, Add SCSI Protection For Multi-Device

	memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;

	Status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_WRITE_DATA,
											UNPACK_STATUS);

	return Status;
}


UINT CeMMCDriver::BlockOtherRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		UINT status = 0;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_BLOCK_OTHER;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;


		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_CONTROL,
												UNPACK_REG1BYTE); // UNPACK_STATUS); Sherlock_20140812, Result Status From Byte[26]


	return status;
}

UINT CeMMCDriver::enableReadSpareArea() {
	// TODO Auto-generated constructor stub
	UINT	Status = Fail_State;
	BYTE	Register = 0;

	// 0x1FF820AB or Bit_2:0x42->0x46
	Status = readData( 0x1FF820AB, 1, &Register);
	Register |= BIT2;
	Status = writeData( 0x1FF820AB, 1, &Register);
	// 0x1FF82004 = 0:0x40->0x00
	Register = 0;
	Status = writeData( 0x1FF82004, 1, &Register);
	// 0x1FF82809 OR Bit_1:0x80->0x82
	Status = readData( 0x1FF82809, 1, &Register);
	Register |= BIT1;
	Status = writeData( 0x1FF82809, 1, &Register);

	return Status;
}

UINT CeMMCDriver::SpareAccessRead(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =  offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_SPARE_ACCESS;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.Cdb[7] = HIBYTE(BufLen);
		sptwb.spt.Cdb[8] = LOBYTE(BufLen);

		sptwb.spt.Cdb[9] = COLA1; // Encryption
		sptwb.spt.Cdb[10] = COLA0;
		sptwb.spt.Cdb[11] = HIBYTE(BlockPage);
		sptwb.spt.Cdb[12] = LOBYTE(BlockPage);

		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	// Sherlock_20140415	Sleep(10);		// Sherlock_20130306, Add Delay For BlockWrite & BlockRead, And L85 Must Delay

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) + sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_READ_INFO,
												UNPACK_DATA);

		return status;
}

UINT CeMMCDriver::BlockCheckRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE COLA1, BYTE COLA0, BYTE CFG0, BYTE CFG1, BYTE CFG2, USHORT PlaneBlock, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT 	status = 0;
	ULONG 	length = 0;
//	Sleep(10); // Sherlock_20120926, Test

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 30;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =  offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_BLK_CHECK;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = HIBYTE(BufLen);
	sptwb.spt.Cdb[8] = LOBYTE(BufLen);

	sptwb.spt.Cdb[9] = COLA1;
	sptwb.spt.Cdb[10] = COLA0;

	sptwb.spt.Cdb[11] = CFG0;
	sptwb.spt.Cdb[12] = CFG1;

	sptwb.spt.Cdb[13] = CFG2; // Sherlock_20131025, Change For eMMC, Bit_7 is Encryption ON/OFF

	sptwb.spt.Cdb[14] = HIBYTE(PlaneBlock); // Sherlock_20140724, For eMMC Scan Block
	sptwb.spt.Cdb[15] = LOBYTE(PlaneBlock);

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_READ_DATA,
											UNPACK_STATUS);

	return status;
}

UINT CeMMCDriver:: AccessMemoryWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub

		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		UINT	status = Fail_State;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_AccessMemory;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

	//	sptwb.spt.Cdb[4] = HIBYTE(Address);
	//	sptwb.spt.Cdb[5] = LOBYTE(Address);

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.Cdb[7] = HIBYTE(BufLen);
		sptwb.spt.Cdb[8] = LOBYTE(BufLen);

		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_SET_INFO,
												UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::AccessMemoryRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub

		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		UINT	status = Fail_State;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_AccessMemory;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

	//	sptwb.spt.Cdb[4] = HIBYTE(Address);
	//	sptwb.spt.Cdb[5] = LOBYTE(Address);

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.Cdb[7] = HIBYTE(BufLen);
		sptwb.spt.Cdb[8] = LOBYTE(BufLen);

		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device


		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_READ_DATA, // Sherlock_20130923
											UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::UFDSettingRead(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	//	OutputDebugString(" UFDSettingRead()");
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		UINT status = 0;
		ULONG length = 0;


		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_UFD_SETTING;
		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));


		sptwb.spt.Cdb[6] = MI_CMD;

		if(MI_CMD==MI_READ_FLASH_ID)
			sptwb.spt.Cdb[8] = 5;
		else
			sptwb.spt.Cdb[8] = 0;

		sptwb.spt.Cdb[11] = CFG0;
		sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;


		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_READ_INFO,
												UNPACK_DATA);

		return status;
}

UINT CeMMCDriver::SpareAccessWrite(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;


	sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SPARE_ACCESS;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = HIBYTE(BufLen);
	sptwb.spt.Cdb[8] = LOBYTE(BufLen);

	sptwb.spt.Cdb[9] = COLA1;
	sptwb.spt.Cdb[10] = COLA0;

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_SET_INFO,
											UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::MarkBad(BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer) {
	// TODO Auto-generated constructor stub
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		UINT status = 0;
		ULONG length = 0;

		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = VDR_BLK_MARK;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));


		sptwb.spt.Cdb[6] = MI_MARK_BAD;

		sptwb.spt.Cdb[8] = LEN0;

		sptwb.spt.Cdb[11] = CFG0;
		sptwb.spt.Cdb[12] = CFG1;


		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_CONTROL,
												UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::EraseBlock(ULONG Address, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	UINT Status=0;

	Status=BlockOtherRead(MI_BLOCK_ERASE, 0, 0, Address, 1, buffer);

	return Status;
}

UINT CeMMCDriver::sendSetCommand(VendorCMD VCMD, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = 0;
		sptwb.spt.TargetId = 0;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;

		sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

		sptwb.spt.DataTransferLength = VCMD.BufLen;
		// Sherlock_20140710, Unit of BufLen in (EE,02,2A) is Sector, Must Change To Byte
		if((VCMD.OPCode == 0xEE) && (VCMD.MJCMD == 0x02) && (VCMD.MICMD == 0x2A))
			sptwb.spt.DataTransferLength = (ULONG)VCMD.BufLen * 0x200;

		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = VCMD.OPCode;
		sptwb.spt.Cdb[1] = VCMD.MJCMD;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(VCMD.Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(VCMD.Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(VCMD.Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(VCMD.Address));


		sptwb.spt.Cdb[6] = VCMD.MICMD;

		sptwb.spt.Cdb[7] = HIBYTE(VCMD.BufLen);
		sptwb.spt.Cdb[8] = LOBYTE(VCMD.BufLen);

		sptwb.spt.Cdb[9] = HIBYTE(VCMD.COLA);
		sptwb.spt.Cdb[10] = LOBYTE(VCMD.COLA);

		sptwb.spt.Cdb[11] = VCMD.CFG[0];
		sptwb.spt.Cdb[12] = VCMD.CFG[1];
		sptwb.spt.Cdb[13] = VCMD.CFG[2];
		sptwb.spt.Cdb[14] = VCMD.CFG[3];
		sptwb.spt.Cdb[15] = VCMD.CFG[4];

		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
	       		sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												VCMD.Address,
												sptwb.spt.DataTransferLength,//VCMD.BufLen, Sherlock_20140710
												buffer,
												PACK_EMMC,
												ACCESS_WRITE_DATA,
												UNPACK_STATUS);
		return status;



}
UINT CeMMCDriver::sendGetCommand(VendorCMD VCMD, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = VCMD.BufLen;

// Sherlock_20140710, Unit of BufLen in (EE,02,28) is Sector, Must Change To Byte
	if((VCMD.OPCode == 0xEE) && (VCMD.MJCMD == 0x02) && (VCMD.MICMD == 0x28))
	sptwb.spt.DataTransferLength = (ULONG)VCMD.BufLen * 0x200;

	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = VCMD.OPCode;
	sptwb.spt.Cdb[1] = VCMD.MJCMD;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(VCMD.Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(VCMD.Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(VCMD.Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(VCMD.Address));

	sptwb.spt.Cdb[6] = VCMD.MICMD;

	sptwb.spt.Cdb[7] = HIBYTE(VCMD.BufLen);
	sptwb.spt.Cdb[8] = LOBYTE(VCMD.BufLen);

	sptwb.spt.Cdb[9] = HIBYTE(VCMD.COLA);
	sptwb.spt.Cdb[10] = LOBYTE(VCMD.COLA);

	sptwb.spt.Cdb[11] = VCMD.CFG[0];
	sptwb.spt.Cdb[12] = VCMD.CFG[1];
	sptwb.spt.Cdb[13] = VCMD.CFG[2];
	sptwb.spt.Cdb[14] = VCMD.CFG[3];
	sptwb.spt.Cdb[15] = VCMD.CFG[4];


	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
			sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											VCMD.Address,
											sptwb.spt.DataTransferLength,//VCMD.BufLen, Sherlock_20140710
											buffer,
											PACK_EMMC,
											ACCESS_READ_INFO,
											UNPACK_DATA);

		return status;

}

UINT CeMMCDriver::MultiPlaneRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = 0x51;
	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));
	sptwb.spt.Cdb[6] = MI_CMD;
	sptwb.spt.Cdb[7] = PageSec;
	sptwb.spt.Cdb[8] = (UCHAR)(BufLen/512);
	sptwb.spt.Cdb[9] = ECC; // Sherlock_20131106, Bit_7 is Encryption Switch
	sptwb.spt.Cdb[11] = HIBYTE(BlockPage);
	sptwb.spt.Cdb[12] = LOBYTE(BlockPage);

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	///////////////////////////////////////////
	// Sherlock_20110411
	//CString DebugStr;
	//OutputDebugString(" BlockAccessRead()=====");
	//DebugStr.Format("CDB[7]=%x, CDB[8]=%x, CDB[9]=%x", PageSec, (BufLen/512), ECC);
	//OutputDebugString(DebugStr);
	///////////////////////////////////////////

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;


	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_READ_DATA,
											UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::MultiPlaneWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer) {
	// TODO Auto-generated constructor stub
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
		ULONG length = 0;
	//	DWORD dwError;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;

		sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

		sptwb.spt.DataTransferLength = BufLen;
		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
		sptwb.spt.Cdb[1] = 0x51;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));

		sptwb.spt.Cdb[6] = MI_CMD;

		sptwb.spt.Cdb[7] = PageSec;
		sptwb.spt.Cdb[8] = (UCHAR)(BufLen/512);

		sptwb.spt.Cdb[9] = ECC; // Sherlock_20131106, Bit_7 is Encryption Switch
	//	sptwb.spt.Cdb[9] &= (~bit_7); // Sherlock_20131025, Temp For Derek Test

		sptwb.spt.Cdb[10] = Spare.SPARE0;
		sptwb.spt.Cdb[11] = Spare.SPARE1;
		sptwb.spt.Cdb[12] = Spare.SPARE2;
		sptwb.spt.Cdb[13] = Spare.SPARE3;
		sptwb.spt.Cdb[14] = Spare.SPARE4;
		sptwb.spt.Cdb[15] = Spare.SPARE5;

		sptwb.spt.ScsiStatus = 1;    // Sherlock_20121130, Add SCSI Protection For Multi-Device

		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
						sptwb.spt.DataTransferLength;


		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_EMMC,
												ACCESS_WRITE_DATA,
												UNPACK_STATUS);

		return status;
}

UINT CeMMCDriver::SetInfoWriteCMD(BYTE adapter_id, BYTE target_id, VendorCMD VCMD, BYTE *buffer) {
	// TODO Auto-generated constructor stub
		SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
		UINT status = 0;
		ULONG length = 0;

		memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
		sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
		sptwb.spt.PathId = adapter_id;
		sptwb.spt.TargetId = target_id;
		sptwb.spt.Lun = 0;
		sptwb.spt.CdbLength = 16;
		sptwb.spt.SenseInfoLength = 24;

		sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

		sptwb.spt.DataTransferLength = VCMD.BufLen;
	   // Sherlock_20140710, Unit of BufLen in (EE,02,2A) is Sector, Must Change To Byte
		if((VCMD.OPCode == 0xEE) && (VCMD.MJCMD == 0x02) && (VCMD.MICMD == 0x2A))
			sptwb.spt.DataTransferLength = (ULONG)VCMD.BufLen * 0x200;

		sptwb.spt.TimeOutValue = 3;
		sptwb.spt.DataBufferOffset =
	   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
		sptwb.spt.SenseInfoOffset =
	       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

		sptwb.spt.Cdb[0] = VCMD.OPCode;
		sptwb.spt.Cdb[1] = VCMD.MJCMD;

		sptwb.spt.Cdb[2] = HIBYTE(HIWORD(VCMD.Address));
		sptwb.spt.Cdb[3] = LOBYTE(HIWORD(VCMD.Address));
		sptwb.spt.Cdb[4] = HIBYTE(LOWORD(VCMD.Address));
		sptwb.spt.Cdb[5] = LOBYTE(LOWORD(VCMD.Address));


		sptwb.spt.Cdb[6] = VCMD.MICMD;

		sptwb.spt.Cdb[7] = HIBYTE(VCMD.BufLen);
		sptwb.spt.Cdb[8] = LOBYTE(VCMD.BufLen);

		sptwb.spt.Cdb[9] = HIBYTE(VCMD.COLA);
		sptwb.spt.Cdb[10] = LOBYTE(VCMD.COLA);

		sptwb.spt.Cdb[11] = VCMD.CFG[0];
		sptwb.spt.Cdb[12] = VCMD.CFG[1];
		sptwb.spt.Cdb[13] = VCMD.CFG[2];
		sptwb.spt.Cdb[14] = VCMD.CFG[3];
		sptwb.spt.Cdb[15] = VCMD.CFG[4];

		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
	       		sptwb.spt.DataTransferLength;

		status = pmPackedCommand->SendPackageCmd(&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												VCMD.Address,
												sptwb.spt.DataTransferLength,//VCMD.BufLen, Sherlock_20140710
												buffer,
												PACK_EMMC,
												ACCESS_SET_INFO,
												UNPACK_STATUS);

		return status;
}


UINT CeMMCDriver::UFDSettingWrite(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer)
{
//	OutputDebugString(" UFDSettingWrite()");
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;

	if(BufLen!=0)
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
	else
		sptwb.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;

	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset =
   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_UFD_SETTING;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));
	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = HIBYTE((BufLen/512));
	sptwb.spt.Cdb[8] = LOBYTE((BufLen/512));




	sptwb.spt.Cdb[11] = CFG0;

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	memcpy(sptwb.ucDataBuf, buffer,sptwb.spt.DataTransferLength);


	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											Address,
											BufLen,
											buffer,
											PACK_EMMC,
											ACCESS_WRITE_DATA,
											UNPACK_STATUS);
	return status;

}


UINT CeMMCDriver::SendTestUnitReady(BYTE Lun, BYTE adapter_id, BYTE target_id, BYTE *pScsiStatus)
{
	cout<<" SendTestUnitReady()"<<endl;
    SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
    UINT status = 0;
    ULONG length = 0;

    memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = adapter_id;
    sptwb.spt.TargetId = target_id;
    sptwb.spt.Lun = Lun;
    sptwb.spt.CdbLength = 6;
    sptwb.spt.SenseInfoLength = 24;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 0;
    sptwb.spt.TimeOutValue = 3;
    sptwb.spt.DataBufferOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptwb.spt.SenseInfoOffset =
       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = SCSI_TST_U_RDY;
    sptwb.spt.Cdb[4] = 0;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       		sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(&sptwb,
											offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
											0,
											sptwb.spt.DataTransferLength,//VCMD.BufLen, Sherlock_20140710
											pScsiStatus,
											PACK_EMMC,
											ACCESS_READ_INFO,
											UNPACK_DATA);
	return status;
}

UINT CeMMCDriver::ReadFlashID(BYTE CE,BYTE *buffer)
{
	UINT status = 0;

	return status;
}

UINT CeMMCDriver::FillMainFIFO(BYTE MI_CMD, ULONG BufLen, BYTE *buffer)
{
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;

	sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SORTING;

	sptwb.spt.Cdb[2] = 0;
	sptwb.spt.Cdb[3] = 0;
	sptwb.spt.Cdb[4] = 0;
	sptwb.spt.Cdb[5] = 0;

	sptwb.spt.Cdb[6] = MI_CMD;
//return 1;

	sptwb.spt.ScsiStatus = 1;    // Sherlock_20121130, Add SCSI Protection For Multi-Device

	if(BufLen != 32*1024)
	{	// Protection, Must Be 32K
		return false;
	}

	memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       		sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(	&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												0,//Address,
												BufLen,
												buffer,
												PACK_SCSI,
												ACCESS_WRITE_DATA,
												UNPACK_STATUS);

	return status;

}
UINT CeMMCDriver::MLCVBWrite(BYTE MI_CMD, WORD BlockAddr, WORD LunOffset)//, USHORT OldEraseCnt)//,  WORD PageAddr, ULONG BufLen, LPBYTE buffer)
{
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;
	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;

	sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;

	sptwb.spt.DataTransferLength = 0;
	sptwb.spt.TimeOutValue = 6;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SORTING;

	//sptwb.spt.Cdb[2] = HIBYTE(BlockAddr2);
	//sptwb.spt.Cdb[3] = LOBYTE(BlockAddr2);
	sptwb.spt.Cdb[2] = 0;
	sptwb.spt.Cdb[3] = 0;
	sptwb.spt.Cdb[4] = HIBYTE(BlockAddr);
	sptwb.spt.Cdb[5] = LOBYTE(BlockAddr);

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = HIBYTE(LunOffset);
	sptwb.spt.Cdb[8] = LOBYTE(LunOffset);


	sptwb.spt.ScsiStatus = 1;    // Sherlock_20121130, Add SCSI Protection For Multi-Device

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       		sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(	&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												BlockAddr,
												0,//BufLen,
												NULL,//buffer,
												PACK_SCSI,
												ACCESS_CONTROL,
												UNPACK_STATUS);
	return status;

}



UINT CeMMCDriver::BlockMarkWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer)
{

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT status = 0;
	ULONG length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 3;
	sptwb.spt.DataBufferOffset =
   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_BLK_MARK;

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(Address));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(Address));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(Address));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(Address));


	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[8] = LEN0;

	sptwb.spt.Cdb[11] = CFG0;
	sptwb.spt.Cdb[12] = CFG1;


	memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(	&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												Address,
												BufLen,
												buffer,
												PACK_SCSI,
												ACCESS_CONTROL,
												UNPACK_STATUS);
	return status;

}

UINT CeMMCDriver::BlockCheckECC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, USHORT BufLen, BYTE *buffer)
{	// Sherlock_20130205 1
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT 	status = 0;
	ULONG 	length = 0;

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = BufLen;
	sptwb.spt.TimeOutValue = 30;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =  offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SORTING;

	sptwb.spt.Cdb[2] = (BYTE)((CE<<4)|CH);
	sptwb.spt.Cdb[3] = Mode;
	sptwb.spt.Cdb[4] = HIBYTE(BlockAddr);
	sptwb.spt.Cdb[5] = LOBYTE(BlockAddr);

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

//	Sleep(10);		// Sherlock_20130306, Add Delay For BlockWrite & BlockRead, And L85 Must Delay

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;

	status = pmPackedCommand->SendPackageCmd(	&sptwb,
												offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
												BlockAddr,
												BufLen,
												buffer,
												PACK_SCSI,
												ACCESS_READ_INFO,
												UNPACK_DATA);
	return status;

}

UINT CeMMCDriver::SetThreeSLCVB(BYTE MI_CMD)//, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, USHORT BufLen, LPBYTE buffer)
{	// Sherlock_20130205 2
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT 	status = 0;
	ULONG 	length = 0;

	usleep(10000); // Sherlock_20120926, Test

	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = 0;//BufLen;
	sptwb.spt.TimeOutValue = 30;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =  offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SORTING;

	sptwb.spt.Cdb[2] = 0; // (BYTE)((CE<<4)|CH);
	sptwb.spt.Cdb[3] = 0; // Mode;
	sptwb.spt.Cdb[4] = 0; // HIBYTE(BlockAddr);
	sptwb.spt.Cdb[5] = 0; // LOBYTE(BlockAddr);

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;
/*
	status = DeviceIoControl(	DiskPath,
							IOCTL_SCSI_PASS_THROUGH,
							&sptwb,
							offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
							&sptwb,
							length,
							&returned,
							FALSE);

	if ( (status) && (!sptwb.spt.ScsiStatus) )
	{
//		memcpy(buffer, sptwb.ucDataBuf, BufLen);
		return true;
	}
	else
	{
		if(!status)
			OutputDebugString("status err");
		if(sptwb.spt.ScsiStatus)
			OutputDebugString("SCSI status err");
		DWORD ErrorCode=GetLastError();
		StrTmp.Format("ErrorCode=%x", ErrorCode);
		OutputDebugString(StrTmp);
		return false;
	}*/
	return 0;
}

UINT CeMMCDriver::CopySLCtoTLC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, WORD LunOffset)//, USHORT BufLen, LPBYTE buffer)
{	// Sherlock_20130205 3
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	UINT 	status = 0;
	ULONG 	length = 0;

	usleep(10000); // Sherlock_20120926, Test


	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = 0;
	sptwb.spt.TargetId = 0;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 16;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = 0;//BufLen;
	sptwb.spt.TimeOutValue = 30;
	sptwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =  offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[0] = SCSI_VLIVENDOR;
	sptwb.spt.Cdb[1] = VDR_SORTING;

	sptwb.spt.Cdb[2] = 0; //(BYTE)((CE<<4)|CH);
	sptwb.spt.Cdb[3] = Mode;
	sptwb.spt.Cdb[4] = HIBYTE(BlockAddr);
	sptwb.spt.Cdb[5] = LOBYTE(BlockAddr);

	sptwb.spt.Cdb[6] = MI_CMD;

	sptwb.spt.Cdb[7] = HIBYTE(LunOffset);
	sptwb.spt.Cdb[8] = LOBYTE(LunOffset);

	sptwb.spt.ScsiStatus = 1;	 // Sherlock_20121130, Add SCSI Protection For Multi-Device

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
					sptwb.spt.DataTransferLength;
/*
	status = DeviceIoControl(	DiskPath,
							IOCTL_SCSI_PASS_THROUGH,
							&sptwb,
							offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf),
							&sptwb,
							length,
							&returned,
							FALSE);

	if ( (status) && (!sptwb.spt.ScsiStatus) )
	{
//		memcpy(buffer, sptwb.ucDataBuf, BufLen);
		return true;
	}
	else
	{
		if(!status)
			OutputDebugString("status err");
		if(sptwb.spt.ScsiStatus)
			OutputDebugString("SCSI status err");
		DWORD ErrorCode=GetLastError();
		StrTmp.Format("ErrorCode=%x", ErrorCode);
		OutputDebugString(StrTmp);
		return false;
	}
	*/
	return 0;
}
