// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#define _PC_	// disables MIPS opcode macros.

#include "R3000A.h"
#include "Common.h"
#include "Sif.h"
#include "IopHw.h"

_sif sif1;

static bool done = false;
static bool sif1_dma_stall = false;

static __fi void Sif1Init()
{
	SIF_LOG("SIF1 DMA start...");
	done = false;
	sif1.ee.cycles = 0;
	sif1.iop.cycles = 0;
}

// Write from the EE to Fifo.
static __fi bool WriteEEtoFifo()
{
	// There's some data ready to transfer into the fifo..

	SIF_LOG("Sif 1: Write EE to Fifo");
	const int writeSize = std::min((s32)sif1ch.qwc, sif1.fifo.sif_free() >> 2);

	tDMA_TAG *ptag;

	ptag = sif1ch.getAddr(sif1ch.madr, DMAC_SIF1, false);
	if (ptag == NULL)
	{
		DevCon.Warning("Write EE to Fifo: ptag == NULL");
		return false;
	}

	sif1.fifo.write((u32*)ptag, writeSize << 2);

	sif1ch.madr += writeSize << 4;
	hwDmacSrcTadrInc(sif1ch);
	sif1.ee.cycles += writeSize;		// fixme : BIAS is factored in above
	sif1ch.qwc -= writeSize;

	return true;
}

// Read from the fifo and write to IOP
static __fi bool WriteFifoToIOP()
{
	// If we're reading something, continue to do so.

	SIF_LOG("Sif1: Write Fifo to IOP");
	const int readSize = std::min(sif1.iop.counter, sif1.fifo.size);

	SIF_LOG("Sif 1 IOP doing transfer %04X to %08X", readSize, HW_DMA10_MADR);

	sif1.fifo.read((u32*)iopPhysMem(hw_dma10.madr), readSize);
	psxCpu->Clear(hw_dma10.madr, readSize);
	hw_dma10.madr += readSize << 2;
	sif1.iop.cycles += readSize >> 2;		// fixme: should be >> 4
	sif1.iop.counter -= readSize;

	return true;
}

// Get a tag and process it.
static __fi bool ProcessEETag()
{
	// Chain mode
	tDMA_TAG *ptag;
	SIF_LOG("Sif1: ProcessEETag");

	// Process DMA tag at sif1ch.tadr
	ptag = sif1ch.DMAtransfer(sif1ch.tadr, DMAC_SIF1);
	if (ptag == NULL)
	{
		Console.WriteLn("Sif1 ProcessEETag: ptag = NULL");
		return false;
	}

	if (sif1ch.chcr.TTE)
	{
		Console.WriteLn("SIF1 TTE");
		sif1.fifo.write((u32*)ptag + 2, 2);
	}

	SIF_LOG("%s", ptag->tag_to_str().c_str());
	sif1ch.madr = ptag[1]._u32;

	sif1.ee.end = hwDmacSrcChain(sif1ch, ptag->ID);

	if (sif1ch.chcr.TIE && ptag->IRQ)
	{
		//Console.WriteLn("SIF1 TIE");
		sif1.ee.end = true;
	}

	return true;
}

// Write fifo to data, and put it in IOP.
static __fi bool SIFIOPReadTag()
{
	// Read a tag.
	sif1.fifo.read((u32*)&sif1.iop.data, 4);
	//sif1words = (sif1words + 3) & 0xfffffffc; // Round up to nearest 4.
	SIF_LOG("SIF 1 IOP: dest chain tag madr:%08X wc:%04X id:%X irq:%d",
		sif1data & 0xffffff, sif1words, sif1tag.ID, sif1tag.IRQ);

	// Only use the first 24 bits.
	hw_dma10.madr = sif1data & 0xffffff;


	if (sif1words > 0xFFFFC) DevCon.Warning("SIF1 Overrun %x", sif1words);
	//Maximum transfer amount 1mb-16 also masking out top part which is a "Mode" cache stuff, we don't care :)
	sif1.iop.counter = sif1words & 0xFFFFC;

	if (sif1tag.IRQ  || (sif1tag.ID & 4)) sif1.iop.end = true;

	return true;
}

// Stop processing EE, and signal an interrupt.
static __fi void EndEE()
{
	sif1.ee.end = false;
	sif1.ee.busy = false;
	SIF_LOG("Sif 1: End EE");

	// Voodoocycles : Okami wants around 100 cycles when booting up
	// Other games reach like 50k cycles here, but the EE will long have given up by then and just retry.
	// (Cause of double interrupts on the EE)
	if (sif1.ee.cycles == 0)
	{
		SIF_LOG("SIF1 EE: cycles = 0");
		sif1.ee.cycles = 1;
	}

	CPU_SET_DMASTALL(DMAC_SIF1, false);
	CPU_INT(DMAC_SIF1, /*std::min((int)(*/sif1.ee.cycles*BIAS/*), 384)*/);
}

// Stop processing IOP, and signal an interrupt.
static __fi void EndIOP()
{
	sif1data = 0;
	sif1.iop.end = false;
	sif1.iop.busy = false;
	SIF_LOG("Sif 1: End IOP");

	//Fixme ( voodoocycles ):
	//The *24 are needed for ecco the dolphin (CDVD hangs) and silver surfer (Pad not detected)
	//Greater than *35 break rebooting when trying to play Tekken5 arcade history
	//Total cycles over 1024 makes SIF too slow to keep up the sound stream in so3...
	if (sif1.iop.cycles == 0)
	{
		DevCon.Warning("SIF1 IOP: cycles = 0");
		sif1.iop.cycles = 1;
	}
	// iop is 1/8th the clock rate of the EE and psxcycles is in words (not quadwords)
	PSX_INT(IopEvt_SIF1, /*std::min((*/sif1.iop.cycles/* * 26*//*), 1024)*/);
}

// Handle the EE transfer.
static __fi void HandleEETransfer()
{
	if(!sif1ch.chcr.STR)
	{
		//DevCon.Warning("Replacement for irq prevention hack EE SIF1");
		sif1.ee.end = false;
		sif1.ee.busy = false;
		return;
	}

	/*if (sif1ch.qwc == 0)
		if (sif1ch.chcr.MOD == NORMAL_MODE)
			if (!sif1.ee.end){
				DevCon.Warning("sif1 irq prevented CHCR %x QWC %x", sif1ch.chcr, sif1ch.qwc);
				done = true;
				return;
			}*/

	// If there's no more to transfer.
	if (sif1ch.qwc <= 0)
	{
		// If NORMAL mode or end of CHAIN then stop DMA.
		if ((sif1ch.chcr.MOD == NORMAL_MODE) || sif1.ee.end)
		{
			done = true;
			EndEE();
		}
		else
		{
			done = false;
			if (!ProcessEETag()) return;
		}
	}
	else
	{
		if (dmacRegs.ctrl.STD == STD_SIF1)
		{
			if ((sif1ch.chcr.MOD == NORMAL_MODE) || ((sif1ch.chcr.TAG >> 28) & 0x7) == TAG_REFS)
			{
				//DevCon.Warning("SIF1 Stall Control");
				const int writeSize = std::min((s32)sif1ch.qwc, sif1.fifo.sif_free() >> 2);
				if ((sif1ch.madr + (writeSize * 16)) > dmacRegs.stadr.ADDR)
				{
					hwDmacIrq(DMAC_STALL_SIS);
					sif1_dma_stall = true;
					CPU_SET_DMASTALL(DMAC_SIF1, true);
					return;
				}
			}
				//DevCon.Warning("SIF1 stall control Not Implemented"); // STD == fromSIF1
		}
		if (sif1.fifo.sif_free() > 0)
		{
			WriteEEtoFifo();
		}
	}
}

// Handle the IOP transfer.
static __fi void HandleIOPTransfer()
{
	if (sif1.iop.counter > 0)
	{
		if (sif1.fifo.size > 0)
		{
			WriteFifoToIOP();
		}
	}

	if (sif1.iop.counter <= 0)
	{
		if (sif1.iop.end)
		{
			done = true;
			EndIOP();
		}
		else if (sif1.fifo.size >= 4)
		{

			done = false;
			SIFIOPReadTag();
		}
	}
}

static __fi void Sif1End()
{
	psHu32(SBUS_F240) &= ~0x40;
	psHu32(SBUS_F240) &= ~0x4000;

	DMA_LOG("SIF1 DMA End");
}

// Transfer EE to IOP, putting data in the fifo as an intermediate step.
__fi void SIF1Dma()
{
	int BusyCheck = 0;

	if (sif1_dma_stall)
	{
		const int writeSize = std::min((s32)sif1ch.qwc, sif1.fifo.sif_free() >> 2);
		if ((sif1ch.madr + (writeSize * 16)) > dmacRegs.stadr.ADDR)
			return;
	}

	sif1_dma_stall = false;
	Sif1Init();

	do
	{
		//I realise this is very hacky in a way but its an easy way of checking if both are doing something
		BusyCheck = 0;

		if (sif1.ee.busy && !sif1_dma_stall)
		{
			if(sif1.fifo.sif_free() > 0 || (sif1.ee.end && sif1ch.qwc == 0))
			{
				BusyCheck++;
				HandleEETransfer();
			}
		}

		if (sif1.iop.busy)
		{
			if(sif1.fifo.size >= 4 || (sif1.iop.end && sif1.iop.counter == 0))
			{
				BusyCheck++;
				HandleIOPTransfer();
			}
		}

	} while (/*!done &&*/ BusyCheck > 0);

	Sif1End();
}

__fi void  sif1Interrupt()
{
	HW_DMA10_CHCR &= ~0x01000000; //reset TR flag
	psxDmaInterrupt2(3);
}

__fi void  EEsif1Interrupt()
{
	hwDmacIrq(DMAC_SIF1);
	sif1ch.chcr.STR = false;
}

// Do almost exactly the same thing as psxDma10 in IopDma.cpp.
// Main difference is this checks for iop, where psxDma10 checks for ee.
__fi void dmaSIF1()
{
	SIF_LOG("dmaSIF1 %s", sif1ch.cmqt_to_str().c_str());

	if (sif1.fifo.readPos != sif1.fifo.writePos)
	{
		SIF_LOG("warning, sif1.fifoReadPos != sif1.fifoWritePos");
	}

	psHu32(SBUS_F240) |= 0x4000;
	sif1.ee.busy = true;

	CPU_SET_DMASTALL(DMAC_SIF1, false);
	// Okay, this here is needed currently (r3644).
	// FFX battles in the thunder plains map die otherwise, Phantasy Star 4 as well
	// These 2 games could be made playable again by increasing the time the EE or the IOP run,
	// showing that this is very timing sensible.
	// Doing this DMA unfortunately brings back an old warning in Legend of Legaia though, but it still works.

	//Updated 23/08/2011: The hangs are caused by the EE suspending SIF1 DMA and restarting it when in the middle
	//of processing a "REFE" tag, so the hangs can be solved by forcing the ee.end to be false
	// (as it should always be at the beginning of a DMA).  using "if iop is busy" flags breaks Tom Clancy Rainbow Six.
	// Legend of Legaia doesn't throw a warning either :)
	sif1.ee.end = false;

	if (sif1ch.chcr.MOD == CHAIN_MODE && sif1ch.qwc > 0)
	{
		if ((sif1ch.chcr.tag().ID == TAG_REFE) || (sif1ch.chcr.tag().ID == TAG_END) || (sif1ch.chcr.tag().IRQ && vif1ch.chcr.TIE))
		{
			sif1.ee.end = true;
		}
	}

	SIF1Dma();

}
