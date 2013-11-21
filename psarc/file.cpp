/*
 * Open PSARC PS3 extractor
 * Copyright (C) 2011 Matthieu Milan
 */

#define _CRT_SECURE_NO_WARNINGS

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "file.h"

struct File_impl {
	bool _ioErr;
	uint64_t _offset;
	File_impl() : _ioErr(false) {}
	virtual ~File_impl() {}
	virtual bool open(const char *path, const char *mode) = 0;
	virtual void close() = 0;
	virtual uint64_t offset() = 0;
	virtual void seek(uint64_t off) = 0;
	virtual void shift(uint64_t off) = 0;
	virtual void read(void *ptr, uint32_t size) = 0;
	virtual void write(void *ptr, uint32_t size) = 0;
};

struct stdFile : File_impl {
	FILE *_fp;
	stdFile() : _fp(0) {}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_offset = 0;
		_fp = fopen(path, mode);
		return (_fp != 0);
	}
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = 0;
		}
	}
	void seek(uint64_t off) {
		if (_fp) {
			_offset = off;
			_fseeki64(_fp, off, SEEK_SET);
		}
	}
	void shift(uint64_t off) {
		if (_fp) {
			_offset += off;
			_fseeki64(_fp, _offset, SEEK_SET);
		}
	}
	uint64_t offset() {
		if (_fp) {
			return _offset;
		}

		return 0; // TODO: trouver mieux
	}
	void read(void *ptr, uint32_t size) {
		if (_fp) {
			_offset += size;
			uint32_t r = fread(ptr, 1, size, _fp);
			if (r != size)
				_ioErr = true;
		}
	}
	void write(void *ptr, uint32_t size) {
		if (_fp) {
			_offset += size;
			uint32_t r = fwrite(ptr, 1, size, _fp);
			if (r != size)
				_ioErr = true;
		}
	}
};

File::File() {
	_impl = new stdFile;
}

File::~File() {
	if (_impl) {
		_impl->close();
		delete _impl;
	}
}

bool File::open(const char *filename, const char *directory, const char *mode) {
	if (_impl) {
		_impl->close();
		delete _impl;
		_impl = 0;
	}

	if (!_impl)
		_impl = new stdFile;

	char buf[512];
	snprintf(buf, sizeof(buf), "%s/%s", directory, filename);

	return _impl->open(buf, mode);
}

void File::close() {
	if (_impl) {
		_impl->close();
	}
}

bool File::ioErr() const {
	return _impl->_ioErr;
}

void File::seek(uint64_t off) {
	_impl->seek(off);
}

void File::shift(uint64_t off) {
	_impl->shift(off);
}

uint64_t File::offset() {
	return _impl->offset();
}

void File::read(void *ptr, uint32_t size) {
	_impl->read(ptr, size);
}

uint8_t File::readByte() {
	uint8_t b;
	read(&b, 1);

	return b;
}

uint32_t File::readInt24BE(uint8_t *ptr) {
	read(ptr, 3);

	return READ_BE_INT24(ptr);
}

uint64_t File::readInt40BE(uint8_t *ptr) {
	read(ptr, 5);

	return READ_BE_INT40(ptr);
}

uint16_t File::readUint16BE(uint8_t *ptr) {
	read(ptr, 2);

	return READ_BE_UINT16(ptr);
}

uint32_t File::readUint32BE(uint8_t *ptr) {
	read(ptr, 4);

	return READ_BE_UINT32(ptr);
}

void File::write(void *ptr, uint32_t size) {
	_impl->write(ptr, size);
}

#undef snprintf
