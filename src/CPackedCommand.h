/*
 * CPackedCommand.h
 *
 *  Created on: 2015�~6��29��
 *      Author: Coolio
 */

#ifndef CPACKEDCOMMAND_H_
#define CPACKEDCOMMAND_H_
#include "common.h"

#define MaxTransferSize			0x10000
#define PACKET_BLOCK_SIZE		0x200
#define PACKET_DATA_SIZE		0x100
#define SIGNATURE		"493PJECTVIA-LABS"

#define IN
#define OUT
#define INOUT
#define RETRY_COUNT		1 // Sherlock_20140711

// ----- For AccessType -----
#define ACCESS_READ_DATA	0	// CMD + Data + Status
#define ACCESS_WRITE_DATA	1	// CMD + Data + Status
#define ACCESS_CONTROL		2	// CMD + Status
#define ACCESS_READ_INFO	3	// CMD + Status(Data)
#define ACCESS_SET_INFO	4	// CMD(Data) + Status

// ----- For PackMode -----
#define PACK_SCSI		0
#define PACK_EMMC		1

// ----- For UnPackMode -----
#define UNPACK_STATUS		0	// Only Get Status From STATUS_PHASE
#define UNPACK_DATA		1	// Extract Data From STATUS_PHASE
#define UNPACK_REG1BYTE	2

#define FALSE false
#define TRUE true

enum STATE{CMD_PHASE,DATA_PHASE,STATUS_PHASE};


class CPackedCommand {
public:
	CPackedCommand();
	virtual ~CPackedCommand();
	UINT SendPackageCmd(LPVOID lpInBuffer,DWORD nInBufferSize,
						ULONG LBA, ULONG BufLen, BYTE *buffer,
						UINT PackMode,
						UINT AccessType,
						UINT UnPackMode);
private:
	UINT SendReadWriteCMD(ULONG BufLen, BYTE *buffer,UINT AccessType);
	void CalCheckSum(IN UCHAR *DataIn,OUT BYTE *DataOut);
};

#endif /* CPACKEDCOMMAND_H_ */
