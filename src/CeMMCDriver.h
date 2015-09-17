/*
 * CeMMCDriver.h
 *
 *  Created on: Apr 17, 2015
 *      Author: vli
 */

#ifndef CEMMCDRIVER_H_
#define CEMMCDRIVER_H_
#include "common.h"
#include "IeMMCDriver.h"
#include "CPackedCommand.h"

class CeMMCDriver: public IeMMCDriver {
private:
	CPackedCommand *pmPackedCommand;
public:
	CeMMCDriver();
	virtual ~CeMMCDriver();
	UINT readData(ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT writeData(ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT enableMPFunction();
	UINT enableReadSpareArea();
	UINT SpareAccessRead(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT SpareAccessWrite(BYTE MI_CMD, BYTE COLA1, BYTE COLA0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT EraseBlock(ULONG Address, BYTE *buffer);
	UINT sendSetCommand(VendorCMD VCMD, BYTE *buffer);
	UINT sendGetCommand(VendorCMD VCMD, BYTE *buffer);
	UINT MarkBad(BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer);
	UINT BlockOtherRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT BlockAccessWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT BlockAccessRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT UFDSettingRead(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT AccessMemoryRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT AccessMemoryWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT BlockCheckRead(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE COLA1, BYTE COLA0, BYTE CFG0, BYTE CFG1, BYTE CFG2, USHORT PlaneBlock, BYTE *buffer);
	UINT MultiPlaneRead(BYTE MI_CMD, BYTE PageSec, BYTE ECC,  BYTE adapter_id, BYTE target_id, USHORT BlockPage, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT MultiPlaneWrite(BYTE MI_CMD, BYTE PageSec, BYTE ECC, SPARETYPE Spare,  BYTE adapter_id, BYTE target_id, ULONG Address, ULONG BufLen, BYTE *buffer);
	UINT SetInfoWriteCMD(BYTE adapter_id, BYTE target_id, VendorCMD VCMD, BYTE *buffer);
	UINT UFDSettingWrite(BYTE MI_CMD, BYTE CFG0, BYTE adapter_id, BYTE target_id,ULONG Address, USHORT BufLen, BYTE *buffer);
	UINT SendTestUnitReady(BYTE Lun, BYTE adapter_id, BYTE target_id, BYTE *pScsiStatus);
	UINT ReadFlashID(BYTE CE,BYTE *buffer);
	UINT BlockMarkWrite(BYTE MI_CMD, BYTE adapter_id, BYTE target_id, ULONG Address, USHORT BufLen, BYTE LEN0, BYTE CFG0, BYTE CFG1, BYTE *buffer);
	UINT CopySLCtoTLC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, WORD LunOffset);
	UINT MLCVBWrite(BYTE MI_CMD, WORD BlockAddr, WORD LunOffset);
	UINT FillMainFIFO(BYTE MI_CMD, ULONG BufLen, BYTE *buffer);
	UINT BlockCheckECC(BYTE MI_CMD, BYTE CE, BYTE CH, WORD BlockAddr, BYTE Mode, USHORT BufLen, BYTE *buffer);
	UINT SetThreeSLCVB(BYTE MI_CMD);
};

#endif /* CEMMCDRIVER_H_ */
