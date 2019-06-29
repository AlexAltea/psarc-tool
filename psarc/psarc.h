/*
 * Open PSARC PS3 extractor
 * Copyright (C) 2011-2018 Matthieu Milan
 */

#ifndef PSARC_H__
#define PSARC_H__

#include "file.h"

#define kPSARCMagicNumber 0x50534152

struct Pack {
	int32_t _id;
	uint64_t _length;
	char *_name;
	uint32_t _zIndex;
	uint64_t _zOffset;
};

struct PSARC {
	File _f;
	uint8_t *_buffer;

	Pack *_entries;
	uint32_t _numEntries;

	PSARC();
	~PSARC();

	void inflateEntry(uint32_t entry, uint32_t *zBlocks, uint32_t cBlockSize, char *basename, char *dirname);
	void read(const char *arcName, uint32_t start, uint32_t end, bool header);
	void read(const char *arcName, uint32_t start, uint32_t end);
	void readHeader(const char *arcName);
};

#endif // PSARC_H__
