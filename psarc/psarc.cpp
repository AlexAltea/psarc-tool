/*
 * Open PSARC PS3 extractor
 * Copyright (C) 2011 Matthieu Milan
 */

#define _CRT_SECURE_NO_WARNINGS

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "psarc.h"
#include "strndup.h"

PSARC::PSARC() {
	_buffer = (uint8_t *)malloc(600 * 1024);
	_entries = NULL;
}

PSARC::~PSARC() {
	_f.close();
	free(_buffer);

	if (_entries != NULL)
		delete _entries;
}

void PSARC::inflateEntry(uint32_t entry, uint32_t *zBlocks, uint32_t cBlockSize, char *basename, char *dirname) {
	uint64_t length = 0;

	if (entry == 0) {
		dirname = (char *)"tmp";
		basename = (char *)"psarc.temp";
	}

	File stream;
	if (stream.open(basename, dirname, "wb")) {
		if (_entries[entry]._length != 0) {
			_f.seek(_entries[entry]._zOffset);
			uint32_t zIndex = _entries[entry]._zIndex;
			do {
				if (zBlocks[zIndex] == 0) {
					_f.read(_buffer, cBlockSize);
					stream.write(_buffer, cBlockSize);
				} else {
					uint16_t isGzipped = _f.readUint16BE(_buffer);
					_f.shift(-2);
					_f.read(_buffer, zBlocks[zIndex]);
					if (isGzipped == 0x78da) {
						uint8_t *uncompressData;
						uLongf uncompressSize;
						uint32_t val = _entries[entry]._length - (zIndex - _entries[entry]._zIndex) * cBlockSize;
						if (val < cBlockSize) {
							uncompressData = (uint8_t *)malloc(val);
							uncompressSize = (uLongf)val;
						} else {
							uncompressData = (uint8_t *)malloc(cBlockSize);
							uncompressSize = (uLongf)cBlockSize;
						}
						uncompress(uncompressData, &uncompressSize, _buffer, zBlocks[zIndex]);
						stream.write(uncompressData, uncompressSize);
						free(uncompressData);
					} else
						stream.write(_buffer, zBlocks[zIndex]);
				}
				zIndex++;
			} while (stream.offset() < _entries[entry]._length);
		}

		if (entry == 0) {
			length = stream.offset();
			stream.close();

			File reader;
			reader.open(basename, dirname, "rb");

			_entries[0]._name = (char *)"NamesBlock.bin";
			for (uint32_t i = 1; i < _numEntries; i++) {
				int32_t pos = reader.offset();
				uint8_t byte = reader.readByte();
				uint8_t count = 1;

				while ((byte != 10) && (reader.offset() < length)) {
					byte = reader.readByte();
					count++;
				}

				reader.seek(pos);
				if (byte == 10) {
					reader.read(_buffer, count);
					_entries[i]._name = strndup((char *)_buffer, count);
					reader.shift(1);
				} else {
					reader.read(_buffer, count);
					_entries[i]._name = strndup((char *)_buffer, count+1);
				}
			}

			reader.close();
			remove("tmp/psarc.temp");
		} else {
			if (stream.offset() != _entries[entry]._length)
				printf("File size : %llu bytes. Expected size: %llu bytes\n", stream.offset(), _entries[entry]._length);

			stream.close();
		}
	}
}

void PSARC::read(const char *arcName, uint32_t start, uint32_t end, bool header) {
	char *dirNamec = _strdup(arcName);
	char *fileNamec = _strdup(arcName);

	char *dirName = dirname(dirNamec);
	char *fileName = basename(fileNamec);

	if (_f.open(fileName, dirName)) {
		if (_f.readUint32BE(_buffer) == kPSARCMagicNumber) {
			_f.seek(8);
			if (_f.readUint32BE(_buffer) == 0x7a6c6962) {
				uint32_t zSize = _f.readUint32BE(_buffer);
				_f.seek(20);
				_numEntries = _f.readUint32BE(_buffer);
				_f.seek(24);
				uint32_t cBlockSize = _f.readUint32BE(_buffer);

				uint8_t zType = 1;
				uint32_t i = 256;
				do {
					i *= 256;
					zType = (uint8_t)(zType + 1);
				} while (i < cBlockSize);

				_f.seek(32);
				_entries = new Pack[_numEntries];
				for (uint32_t i = 0; i < _numEntries; i++) {
					_f.shift(16);

					_entries[i]._id = i;
					_entries[i]._zIndex = _f.readUint32BE(_buffer);
					_entries[i]._length = _f.readInt40BE(_buffer);
					_entries[i]._zOffset = _f.readInt40BE(_buffer);
				}

				uint32_t numBlocks = (zSize - (uint32_t)_f.offset()) / zType;
				uint32_t *zBlocks = new uint32_t[numBlocks];
				for (uint32_t i = 0; i < numBlocks; i++) {
					switch (zType) {
						case 2:
							zBlocks[i] = _f.readUint16BE(_buffer);
							break;

						case 3:
							zBlocks[i] = _f.readInt24BE(_buffer);
							break;

						case 4:
							zBlocks[i] = _f.readUint32BE(_buffer);
							break;
					}
				}

				char *baseDir = NULL;
				char ext[] = ".psarc";
				if (strlen(fileName) >= 6 && strncmp(fileName + strlen(fileName) - strlen(ext), ext, strlen(ext)) == 0) {
					baseDir = (char *)malloc(strlen(fileName));
					memcpy(baseDir, fileName, strlen(fileName));
					baseDir[strlen(fileName)-6] = 0x0;
				} else {
					char data[] = "_data";

					baseDir = (char *)malloc(strlen(fileName) + strlen(data) + 1);
					snprintf(baseDir, strlen(fileName) + strlen(data) + 1, "%s%s", fileName, data);
				}

				inflateEntry(0, zBlocks, cBlockSize, NULL, NULL);

				if (header) {
					char ext[] = ".txt";
					char *outList = (char *)malloc(strlen(fileName) + strlen(ext) + 1);
					snprintf(outList, strlen(fileName) + strlen(ext) + 1, "%s%s", fileName, ext);

					File list;
					if (list.open(outList, ".", "wb")) {
						for (uint32_t i = 1; i < _numEntries; i++) {
							char msg[512];

							if (_entries[i]._length < 0x400L)
								sprintf(msg, "%d %1.2lld b %s\n", i, _entries[i]._length, _entries[i]._name);
							else if (_entries[i]._length < 0x100000L)
								sprintf(msg, "%d %1.2f Kb %s\n", i, _entries[i]._length / 1024.0, _entries[i]._name);
							else if (_entries[i]._length < 0x40000000L)
								sprintf(msg, "%d %1.2f Mb %s\n", i, _entries[i]._length / 1048576.0, _entries[i]._name);
							else
								sprintf(msg, "%d %1.2f Gb %s\n", i, _entries[i]._length / 1073741824.0, _entries[i]._name);

							list.write(msg, strlen(msg));
						}

						list.close();
					}

					free(outList);
				} else {
					bool flag = true;
					if (start == 0) {
						start = 1;
						end = _numEntries;
					} else if ((start > (_numEntries - 1)) || (end > (_numEntries - 1))) {
						flag = false;
					} else {
						end++;
					}

					if (flag) {
						_mkdir(baseDir);

						for (uint32_t i = start; i < end; i++) {
							printf("%i %s\n", _entries[i]._id, _entries[i]._name);

							char *subOutDirc = _strdup(_entries[i]._name);
							char *outFilec = _strdup(_entries[i]._name);

							char *subOutDir = dirname(subOutDirc);
							char *outFile = basename(outFilec);
							char *outDir;
							if (strncmp("/", _entries[i]._name, 1) == 0) {
								// outDir = baseDir + subOutDir
								outDir = (char *)malloc(strlen(baseDir) + strlen(subOutDir) + 1);
								memcpy(outDir, baseDir, strlen(baseDir));
								memcpy(&outDir[strlen(baseDir)], subOutDir, strlen(subOutDir));
								outDir[strlen(baseDir) + strlen(subOutDir)] = 0x0;
							} else {
								outDir = (char *)malloc(strlen(baseDir) + strlen(subOutDir) + 2);
								snprintf(outDir, strlen(baseDir) + strlen(subOutDir) + 2, "%s/%s", baseDir, subOutDir);
							}

							mkpath(outDir);

							inflateEntry(i, zBlocks, cBlockSize, outFile, outDir);

							free(outDir);
							free(subOutDirc);
							free(outFilec);
							free(_entries[i]._name);
						}
					}
				}

				free(baseDir);
				delete zBlocks;
			}
		} else
			printf("Compression type is not zlib... Aborting.");
	}

	free(dirNamec);
	free(fileNamec);
}

void PSARC::read(const char *arcName, uint32_t start, uint32_t end) {
	read(arcName, start, end, false);
}

void PSARC::readHeader(const char *arcName) {
	read(arcName, 0, 0, true);
}

#undef snprintf
