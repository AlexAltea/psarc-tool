/*
 * Open PSARC PS3 extractor
 * Copyright (C) 2011 Matthieu Milan
 */

#ifndef FILE_H__
#define FILE_H__

#include "sys.h"

struct File_impl;

struct File {
	File_impl *_impl;

	File();
	~File();

	bool open(const char *filename, const char *directory, const char *mode="rb");
	void close();
	bool ioErr() const;
	void seek(uint64_t off);
	void shift(uint64_t off);
	uint64_t offset();

	void read(void *ptr, uint32_t size);
	uint8_t readByte();
	uint32_t readInt24BE(uint8_t *ptr);
	uint64_t readInt40BE(uint8_t *ptr);
	uint16_t readUint16BE(uint8_t *ptr);
	uint32_t readUint32BE(uint8_t *ptr);
	void write(void *ptr, uint32_t size);
};

#endif // FILE_H__
