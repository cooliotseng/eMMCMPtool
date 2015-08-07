/*
 * IeMMCDriver.h
 *
 *  Created on: Apr 17, 2015
 *      Author: vli
 */

#ifndef IEMMCDRIVER_H_
#define IEMMCDRIVER_H_
#include "common.h"

class IeMMCDriver {
public:
	IeMMCDriver();
	virtual ~IeMMCDriver();
	virtual UINT readData(ULONG Address,USHORT BufLen,BYTE *buffer) = 0;
	virtual UINT writeData(ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT enableMPFunction()=0;
	virtual UINT enableReadSpareArea()=0;
	virtual UINT SpareAccessRead(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, USHORT BufLen, BYTE *buffer)=0;
	virtual UINT SpareAccessWrite(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer)=0;
	virtual UINT EraseBlock(ULONG Address, BYTE *buffer)=0;
	virtual UINT sendSetCommand(VendorCMD VCMD, BYTE *buffer) = 0;
	virtual UINT sendGetCommand(VendorCMD VCMD, BYTE *buffer) = 0;
	virtual UINT MarkBad(BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer)=0;
	virtual UINT BlockOtherRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT BlockAccessWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer) = 0;
	virtual UINT BlockAccessRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer) = 0;
	virtual UINT UFDSettingRead(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT AccessMemoryRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT AccessMemoryWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT BlockCheckRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE COLA1, BYTE COLA0, BYTE CFG0, BYTE CFG1, BYTE CFG2, USHORT PlaneBlock, BYTE *buffer) = 0;
	virtual UINT MultiPlaneRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer) = 0;
	virtual UINT MultiPlaneWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer) = 0;
	virtual UINT SetInfoWriteCMD(BYTE adapter_id, BYTE target_id, VendorCMD VCMD, BYTE *buffer) = 0;
	virtual UINT UFDSettingWrite(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer) = 0;
	virtual UINT SendTestUnitReady(BYTE Lun, BYTE adapter_id, BYTE target_id, BYTE *pScsiStatus) = 0;

};

#endif /* IEMMCDRIVER_H_ */
