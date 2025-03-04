// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "DEV9/ATA/ATA.h"
#include "DEV9/DEV9.h"

void ATA::DRQCmdPIODataToHost(u8* buff, int buffLen, int buffIndex, int size, bool sendIRQ)
{
	//Data in PIO ready to be sent
	pioPtr = 0;
	pioEnd = size >> 1;

	memcpy(pioBuffer, &buff[buffIndex], size < (buffLen - buffIndex) ? size : (buffLen - buffIndex));

	regStatus &= ~ATA_STAT_BUSY;
	regStatus |= ATA_STAT_DRQ;

	// Only set pendingInterrupt if nIEN is cleared
	if (regControlEnableIRQ && sendIRQ)
	{
		pendingInterrupt = true;
		_DEV9irq(ATA_INTR_INTRQ, 1);
	}
}
void ATA::PostCmdPIODataToHost()
{
	pioPtr = 0;
	pioEnd = 0;
	//AnyMoreData?
	if (pioDRQEndTransferFunc != nullptr)
	{
		regStatus |= ATA_STAT_BUSY;
		regStatus &= ~ATA_STAT_DRQ;
		//Call cmd to retrive more data
		(this->*pioDRQEndTransferFunc)();
	}
	else
		regStatus &= ~ATA_STAT_DRQ;
}

//FromHost
u16 ATA::ATAreadPIO()
{
	//DevCon.WriteLn("DEV9: *ATA_R_DATA 16bit read, pio_count %i,  pio_size %i", pioPtr, pioEnd);
	if (pioPtr < pioEnd)
	{
		const u16 ret = *(u16*)&pioBuffer[pioPtr * 2];
		//DevCon.WriteLn("DEV9: *ATA_R_DATA returned value is  %x", ret);
		pioPtr++;
		if (pioPtr >= pioEnd) //Fnished transfer (Changed from MegaDev9)
			PostCmdPIODataToHost();

		return ret;
	}
	return 0xFF;
}
//ATAwritePIO

void ATA::HDD_IdentifyDevice()
{
	if (!PreCmd())
		return;
	DevCon.WriteLn("DEV9: HddidentifyDevice");

	//IDE transfer start
	CreateHDDinfo(hddImageSize / 512);

	pioDRQEndTransferFunc = nullptr;
	DRQCmdPIODataToHost(identifyData, 256 * 2, 0, 256 * 2, true);
}

//Read Buffer

void ATA::HDD_ReadMultiple(bool isLBA48)
{
	sectorsPerInterrupt = curMultipleSectorsSetting;
	HDD_ReadPIO(isLBA48);
}

void ATA::HDD_ReadSectors(bool isLBA48)
{
	sectorsPerInterrupt = 1;
	HDD_ReadPIO(isLBA48);
}

void ATA::HDD_ReadPIO(bool isLBA48)
{
	//Log_Info("HDD_ReadPIO");
	if (!PreCmd())
		return;

	if (sectorsPerInterrupt == 0)
	{
		CmdNoDataAbort();
		return;
	}

	IDE_CmdLBA48Transform(isLBA48);

	regStatus &= ~ATA_STAT_SEEK;
	if (!HDD_CanSeek())
	{
		regStatus |= ATA_STAT_ERR;
		regStatusSeekLock = -1;
		regError |= ATA_ERR_ID;
		PostCmdNoData();
		return;
	}
	else
		regStatus |= ATA_STAT_SEEK;

	HDD_ReadSync(&ATA::HDD_ReadPIOS2);
}

void ATA::HDD_ReadPIOS2()
{
	//Log_Info("HDD_ReadPIO Stage 2");
	pioDRQEndTransferFunc = &ATA::HDD_ReadPIOEndBlock;
	DRQCmdPIODataToHost(readBuffer, readBufferLen, 0, 256 * 2, true);
}

void ATA::HDD_ReadPIOEndBlock()
{
	//Log_Info("HDD_ReadPIO End Block");
	rdTransferred += 512;
	if (rdTransferred >= nsector * 512)
	{
		//Log_Info("HDD_ReadPIO Done");
		HDD_SetErrorAtTransferEnd();
		regStatus &= ~ATA_STAT_BUSY;
		pioDRQEndTransferFunc = nullptr;
		rdTransferred = 0;
	}
	else
	{
		if ((rdTransferred / 512) % sectorsPerInterrupt == 0)
			DRQCmdPIODataToHost(readBuffer, readBufferLen, rdTransferred, 256 * 2, true);
		else
			DRQCmdPIODataToHost(readBuffer, readBufferLen, rdTransferred, 256 * 2, false);
	}
}

//Write Buffer

//Write Multiple

//Write Sectors

//Download Microcode (Used for FW updates)
