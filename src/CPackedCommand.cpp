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
#include <sys/stat.h>	//read, write, open
#include <fcntl.h>	//open
#include <unistd.h>	//close
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
	int fd_status,n;

	fd_status = open ("/dev/vdr_test0", O_RDWR);
	if (fd_status < 0/* || fd_1 < 0*/) {
		perror ("Open /dev/vdr_test error! QQ!");
		exit (1);
	}

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
				//status = SendReadWriteCMD(V_Address, PACKET_BLOCK_SIZE, PackBuf, ACCESS_WRITE_DATA, 0,0,0);
				n = write (fd_status, PackBuf, PACKET_BLOCK_SIZE);


				// Always Sent 512Byte Command From PackBuf to FW
				if(n < 0)
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
				status = SendReadWriteCMD(BufLen, buffer, AccessType);
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
				//status = SendReadWriteCMD(V_Address, PACKET_BLOCK_SIZE, PackBuf, ACCESS_READ_DATA, 0,0,0);
				n = read(fd_status, PackBuf, PACKET_BLOCK_SIZE);
				// Always Get 512Byte Status From FW to PackBuf
				if(n < 0 )
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
	close(fd_status);
	free(PackBuf);
	return TRUE;
}

BOOL CPackedCommand::SendReadWriteCMD(ULONG BufLen, BYTE *buffer,UINT AccessType)
{
	//printf("this is CeMMCDeviceIO:%s SCSIRead:%d\n",&mSymbolicname,this);
	ULONG OutBufferlength = 0, Datalength = 0,returned = 0;
	int fd_RW;
	WORD DataTransferLength;

	fd_RW = open ("/dev/vdr_test1", O_RDWR);
    DataTransferLength = (WORD)((BufLen+511)/512)*512;
	
	
	if (fd_RW < 0) {
		perror ("Open /dev/vdr_test error! QQ!");
		exit (1);
	}

	if(AccessType == ACCESS_READ_DATA)
	{
		Datalength = read(fd_RW, buffer,DataTransferLength);
	}else{
		Datalength = write(fd_RW, buffer,DataTransferLength);
	}

	close (fd_RW);
	if (Datalength > 0){
		return TRUE;
	}else{
		return FALSE;
	}
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
