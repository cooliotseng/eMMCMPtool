/*
 * CPackedCommand.cpp
 *
 *  Created on: 2015�~6��29��
 *      Author: Coolio
 */

#include "CPackedCommand.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
//pack message function
void PackSCSI(IN LPVOID lpInBuffer,IN UINT AccessType,OUT BYTE *PackDataBuf);
void PackeMMC(IN LPVOID lpInBuffer,IN UINT AccessType,OUT BYTE *PackDataBuf);
//unpack message function
BOOL UnPackStatus(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf);
BOOL UnPackData(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf);
BOOL UnPackReg1Byte(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf);

CPackedCommand::CPackedCommand() {
	// TODO Auto-generated constructor stub

}

CPackedCommand::~CPackedCommand() {
	// TODO Auto-generated destructor stub
}

BOOL CPackedCommand::SendPackageCmd(LPVOID lpInBuffer,DWORD nInBufferSize,
							ULONG LBA, ULONG BufLen, BYTE *buffer,
							UINT PackMode,
							UINT AccessType,
							UINT UnPackMode){

	BYTE	Error_Code = 0;
	void (*PackMessage[])(IN LPVOID lpInBuffer,IN UINT AccessType,OUT BYTE *PackDataBuf)
		= {PackSCSI, PackeMMC};
	BOOL (*UnpackMessage[])(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf)
		= {UnPackStatus, UnPackData, UnPackReg1Byte};

	enum STATE state=CMD_PHASE;
	BOOL	stateExit = TRUE, status = FALSE;
	ULONG	V_Address=0x001E;
	UINT	SCSIRetryCnt = RETRY_COUNT, eMMCRetryCnt = RETRY_COUNT;
	BYTE	*PackBuf;
	PackBuf=(BYTE *)malloc(sizeof(BYTE)*PACKET_BLOCK_SIZE);
	memset(PackBuf, 0, PACKET_BLOCK_SIZE);
//packet cmomand
	PackMessage[PackMode](lpInBuffer,AccessType,PackBuf);

//send SCSI Command
	while(stateExit){
		switch(state)
		{
			case CMD_PHASE:
			{
				printf("state is CMD_PHASE\n");
				status = SendReadWriteCMD(V_Address, PACKET_BLOCK_SIZE, PackBuf, ACCESS_WRITE_DATA, 0,0,0);
				// Always Sent 512Byte Command From PackBuf to FW
				if(status == FALSE)
				{
					SCSIRetryCnt--;
					state = CMD_PHASE;
					if(SCSIRetryCnt == 0 )
					{
						free(PackBuf);
						return FALSE;
					}
					break;
				}
				SCSIRetryCnt = RETRY_COUNT;

				if((AccessType == ACCESS_READ_DATA) || (AccessType == ACCESS_WRITE_DATA))
					state = DATA_PHASE;
				else
					state = STATUS_PHASE;

				break;
			}
			case DATA_PHASE:
			{
				printf("state is DATA_PHASE\n");
				status = SendReadWriteCMD(V_Address, BufLen, buffer, AccessType, 0,0,0);
				// Get/Sent 64KByte(Max) Data Through buffer From/To FW
				if(status == FALSE)
				{
					SCSIRetryCnt--;
					state = DATA_PHASE;
					if(SCSIRetryCnt == 0 )
					{
						free(PackBuf);
						return FALSE;
					}
					break;
				}
				SCSIRetryCnt = RETRY_COUNT;

				state = STATUS_PHASE;

				break;
			}
			case STATUS_PHASE:
			{
				printf("state is STATUS_PHASE\n");
				status = SendReadWriteCMD(V_Address, PACKET_BLOCK_SIZE, PackBuf, ACCESS_READ_DATA, 0,0,0);
				// Always Get 512Byte Status From FW to PackBuf
				if(status == FALSE)
				{
					SCSIRetryCnt--;
					state = STATUS_PHASE;
					if(SCSIRetryCnt == 0 )
					{
						free(PackBuf);
						return FALSE;
					}
					break;
				}
				SCSIRetryCnt = RETRY_COUNT;
				//Unpack message
				Error_Code = UnpackMessage[UnPackMode](buffer, BufLen, PackBuf);
				// For Read STS, Only Get Error_Code
				// For Read Info, Also Get 256Byte(Max) Info From PackBuf into buffer
				if(Error_Code != TRUE)
				{
					state = CMD_PHASE;
					eMMCRetryCnt--;
					if(eMMCRetryCnt == 0 )
					{
						free(PackBuf);
						return FALSE;
					}
					break;
				}
				stateExit = FALSE;
				break;
			}
			default:
				printf("ERROR state!!!!\n");
				break;
		}
	}

	free(PackBuf);
	return TRUE;
}

BOOL CPackedCommand::SendReadWriteCMD(ULONG LBA, ULONG BufLen, BYTE *buffer,UINT AccessType,BYTE Lun = 0,BYTE adapter_id = 0, BYTE target_id = 0)
{
	//printf("this is CeMMCDeviceIO:%s SCSIRead:%d\n",&mSymbolicname,this);
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	BOOL status = 0;
	ULONG OutBufferlength = 0, InBufferlength = 0,returned = 0;
	WORD BlockNum=0;
/*
	BlockNum = (WORD)((BufLen+511)/512);
	ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.PathId = adapter_id;
	sptwb.spt.TargetId = target_id;
	sptwb.spt.Lun = 0;
	sptwb.spt.CdbLength = 10;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataTransferLength = BlockNum*512; // BufLen;
	sptwb.spt.TimeOutValue = 30;
	sptwb.spt.DataBufferOffset =
   			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptwb.spt.SenseInfoOffset =
       		offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

	sptwb.spt.Cdb[2] = HIBYTE(HIWORD(LBA));
	sptwb.spt.Cdb[3] = LOBYTE(HIWORD(LBA));
	sptwb.spt.Cdb[4] = HIBYTE(LOWORD(LBA));
	sptwb.spt.Cdb[5] = LOBYTE(LOWORD(LBA));
	sptwb.spt.Cdb[6] = 0;
	sptwb.spt.Cdb[7] = HIBYTE(BlockNum);
	sptwb.spt.Cdb[8] = LOBYTE(BlockNum);
	sptwb.spt.Cdb[9] = 0;

	OutBufferlength = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       		sptwb.spt.DataTransferLength;
*/
	if(AccessType == ACCESS_READ_DATA)
	{

		InBufferlength = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);

	}else{

		memcpy(sptwb.ucDataBuf, buffer, sptwb.spt.DataTransferLength);

		InBufferlength = OutBufferlength;

	}
/*
	status = DeviceIoControl(DiskPath,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             InBufferlength,
                             &sptwb,
                             OutBufferlength,
                             &returned,
                             FALSE);
*/
	if ( (status) && (!sptwb.spt.ScsiStatus) )
	{
		if(AccessType == ACCESS_READ_DATA)
		{
			memcpy(buffer, sptwb.ucDataBuf, BufLen);
		}

		return TRUE;
	}
	else
		return FALSE;
}

void PackSCSI(IN LPVOID lpInBuffer, IN UINT AccessType,OUT BYTE *PackDataBuf)
{

	printf("this is PackSCSI\n");
	/*
	SCSI_PASS_THROUGH_WITH_BUFFERS *sptwb= (PSCSI_PASS_THROUGH_WITH_BUFFERS)lpInBuffer;
	BYTE Tmp[PACKET_BLOCK_SIZE], Text_tmp[17];

	memset(PackDataBuf,0,PACKET_BLOCK_SIZE);

	memcpy(Text_tmp,SIGNATURE,sizeof(SIGNATURE));
//1.Set signature
	for(int i= 0;i<16;i++)
	{
		PackDataBuf[i] = Text_tmp[15-i];
	}
//2.Set VU Command mode
	PackDataBuf[16]=0xEE;
//3.Set SCSI VU command
	memcpy(PackDataBuf+17, &sptwb->spt.Cdb[1], sizeof(BYTE)*15);
	memcpy(Tmp, PackDataBuf, sizeof(BYTE)*PACKET_BLOCK_SIZE);
//4.Set Check Sum
	for(int i=0;i<4;i++)
	{

		PackDataBuf[32]^=PackDataBuf[16+i*4];
		PackDataBuf[33]^=PackDataBuf[17+i*4];
		PackDataBuf[34]^=PackDataBuf[18+i*4];
		PackDataBuf[35]^=PackDataBuf[19+i*4];
	}

//	memcpy(Tmp, PackDataBuf, sizeof(BYTE)*PACKET_BLOCK_SIZE);

//5.Set Data
	if((AccessType == ACCESS_SET_INFO) && (sptwb->spt.DataTransferLength > 0))
	{
		if(sptwb->spt.DataTransferLength > 256)	OutputDebugString("@@ DataTransferLength ERROR !");
		memcpy(PackDataBuf+256, sptwb->ucDataBuf, sizeof(BYTE)*sptwb->spt.DataTransferLength);
	}
	memcpy(Tmp, PackDataBuf, sizeof(BYTE)*PACKET_BLOCK_SIZE);
	*/
}

//-------------------------------------------------------------------------------------------------

void PackeMMC(IN LPVOID lpInBuffer, IN UINT AccessType,OUT BYTE *PackDataBuf)
{
	SCSI_PASS_THROUGH_WITH_BUFFERS *sptwb= (PSCSI_PASS_THROUGH_WITH_BUFFERS)lpInBuffer;
	BYTE Text_tmp[17];

	memset(PackDataBuf,0,PACKET_BLOCK_SIZE);

	memcpy(Text_tmp,SIGNATURE,sizeof(SIGNATURE));
//1.Set signature
	for(int i= 0;i<16;i++)
	{
		PackDataBuf[i] = Text_tmp[15-i];
	}
//2.Set VU Command mode
	PackDataBuf[16]=0xEE;
//3.Set SCSI VU command
	memcpy(PackDataBuf+17, &sptwb->spt.Cdb[1], sizeof(BYTE)*15);

//4.Set Check Sum
	for(int i=0;i<4;i++)
	{

		PackDataBuf[32]^=PackDataBuf[16+i*4];
		PackDataBuf[33]^=PackDataBuf[17+i*4];
		PackDataBuf[34]^=PackDataBuf[18+i*4];
		PackDataBuf[35]^=PackDataBuf[19+i*4];
	}
//5.Set Data
	if((AccessType == ACCESS_SET_INFO) && (sptwb->spt.DataTransferLength > 0)){

		memcpy(PackDataBuf+256, sptwb->ucDataBuf, sizeof(BYTE)*sptwb->spt.DataTransferLength);

	}

}

//-------------------------------------------------------------------------------------------------

BOOL UnPackStatus(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf)
{
	printf("this is UnPackStatus\n");
	BYTE Tmp_check_sum[4] = {0,0,0,0};
//	memset(Tmp_check_sum,0,sizeof(BYTE)*4);
	//1.check Result
	if(PackDataBuf[26] & 0x01)
	{
		memset(DataBuffer,0,sizeof(BYTE)*BufLen);
		return FALSE;
	}else
	{
		//2.Get check sum
		for(int i=0;i<4;i++)
		{
			Tmp_check_sum[0]^=PackDataBuf[16+i*4];
			Tmp_check_sum[1]^=PackDataBuf[17+i*4];
			Tmp_check_sum[2]^=PackDataBuf[18+i*4];
			Tmp_check_sum[3]^=PackDataBuf[19+i*4];
		}
		//3.check check sum
		if(	PackDataBuf[32]^Tmp_check_sum[0] ||
			PackDataBuf[33]^Tmp_check_sum[1] ||
			PackDataBuf[34]^Tmp_check_sum[2] ||
			PackDataBuf[35]^Tmp_check_sum[3] )
		{
			memset(DataBuffer,0,sizeof(BYTE)*BufLen);
			return FALSE;
		}

		return TRUE;

	}
}

//-------------------------------------------------------------------------------------------------

BOOL UnPackData(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf)
{
	printf("this is UnPackData\n");
	BYTE Tmp_check_sum[4] = {0,0,0,0};
//	memset(Tmp_check_sum,0,sizeof(BYTE)*4);
	//1.check Result
	if(PackDataBuf[26] & 0x01)
	{
		memset(DataBuffer,0,sizeof(BYTE)*BufLen);
		return FALSE;
	}else
	{
		//2.Get check sum
		for(int i=0;i<4;i++)
		{
			Tmp_check_sum[0]^=PackDataBuf[16+i*4];
			Tmp_check_sum[1]^=PackDataBuf[17+i*4];
			Tmp_check_sum[2]^=PackDataBuf[18+i*4];
			Tmp_check_sum[3]^=PackDataBuf[19+i*4];
		}
		//3.check check sum
		if(	PackDataBuf[32]^Tmp_check_sum[0] ||
			PackDataBuf[33]^Tmp_check_sum[1] ||
			PackDataBuf[34]^Tmp_check_sum[2] ||
			PackDataBuf[35]^Tmp_check_sum[3] )
		{
			memset(DataBuffer,0,sizeof(BYTE)*BufLen);
			return FALSE;
		}

		memcpy(DataBuffer, PackDataBuf+256, BufLen);
		return TRUE;

	}
}

//-------------------------------------------------------------------------------------------------

BOOL UnPackReg1Byte(INOUT BYTE *DataBuffer,IN ULONG BufLen,IN BYTE *PackDataBuf)
{
	printf("this is UnPackReg1Byte\n");

	if(BufLen == 1) //Sherlock_20140812, For Block Erase Issue
		memcpy(DataBuffer, PackDataBuf+26, BufLen);
	else if(BufLen == 2)
		memcpy(DataBuffer, PackDataBuf+25, BufLen);

	return TRUE;
}
