// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "CDVD/IsoFileFormats.h"
#include "Host.h"

#include "common/Console.h"
#include "common/FileSystem.h"
#include "common/StringUtil.h"

#include "fmt/format.h"

#include <errno.h>

OutputIsoFile::OutputIsoFile()
{
	_init();
}

OutputIsoFile::~OutputIsoFile()
{
	Close();
}

void OutputIsoFile::_init()
{
	m_version = 0;

	m_offset = 0;
	m_blockofs = 0;
	m_blocksize = 0;
	m_blocks = 0;
}

bool OutputIsoFile::Create(std::string filename, int version)
{
	Close();
	m_filename = std::move(filename);

	m_version = version;
	m_offset = 0;
	m_blockofs = 24;
	m_blocksize = 2048;

	m_outstream = FileSystem::OpenCFile(m_filename.c_str(), "wb");
	if (!m_outstream)
	{
		Console.Error(fmt::format("(OutputIsoFile::Create) Unable to open the file '{}' for writing: {}", m_filename, errno));
		_init();
		return false;
	}

	Console.WriteLn("isoFile create ok: %s ", m_filename.c_str());
	return true;
}

// Generates format header information for blockdumps.
void OutputIsoFile::WriteHeader(int _blockofs, uint _blocksize, uint _blocks)
{
	m_blocksize = _blocksize;
	m_blocks = _blocks;
	m_blockofs = _blockofs;

	Console.WriteLn("blockoffset = %d", m_blockofs);
	Console.WriteLn("blocksize   = %u", m_blocksize);
	Console.WriteLn("blocks	     = %u", m_blocks);

	if (m_version == 2)
	{
		WriteBuffer("BDV2", 4);
		WriteValue(m_blocksize);
		WriteValue(m_blocks);
		WriteValue(m_blockofs);
	}
}

void OutputIsoFile::WriteSector(const u8* src, uint lsn)
{
	if (m_version == 2)
	{
		// Find and ignore blocks that have already been dumped:
		if (std::any_of(std::begin(m_dtable), std::end(m_dtable), [=](const u32 entry) { return entry == lsn; }))
			return;

		m_dtable.push_back(lsn);

		WriteValue<u32>(lsn);
	}
	else
	{
		const s64 ofs = (s64)lsn * m_blocksize + m_offset;
		FileSystem::FSeek64(m_outstream, ofs, SEEK_SET);
	}

	WriteBuffer(src + m_blockofs, m_blocksize);
}

void OutputIsoFile::Close()
{
	m_dtable.clear();

	if (m_outstream)
	{
		std::fclose(m_outstream);
		m_outstream = nullptr;
	}

	_init();
}

void OutputIsoFile::WriteBuffer(const void* src, size_t size)
{
	if (std::fwrite(src, size, 1, m_outstream) != 1)
	{
		Host::ReportErrorAsync("Write Error", fmt::format("errno {} when trying to write {} bytes to block dump file.\n\nClosing file.", errno, size));
		Close();
	}
}

bool OutputIsoFile::IsOpened() const
{
	return m_outstream != nullptr;
}

u32 OutputIsoFile::GetBlockSize() const
{
	return m_blocksize;
}
