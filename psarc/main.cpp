/*
 * Open PSARC PS3 extractor
 * Copyright (C) 2011 Matthieu Milan
 */

#include "psarc.h"

void usage() {
	printf("Usage: psarc [option] filename\n");
	printf("Options:\n");
	printf("\t-l\t\tCreate a text file that lists the file id, size, and name of every file in the archive.\n");
	printf("\t-x\t\tExtracts all files.\n");
	printf("\t-e START END\tExtracts files with the file id specified by the range between START and END (inclusive).\n");
}


int main(int argc, char *argv[]) {
	PSARC psarc;

	switch (argc) {
		case 2:
			{
				if (strncmp(argv[1], "-v", 2) == 0)
					printf("psarc: version 0.1.2\n");
				else
					usage();
			}
			break;

		case 3:
			{
				const char *arcName = argv[2];
				if (strncmp(argv[1], "-l", 2) == 0)
					psarc.readHeader(arcName);
				else if (strncmp(argv[1], "-x", 2) == 0)
					psarc.read(arcName, 0, 0);
			}
			break;

		case 5:
			{
				if (strncmp(argv[1], "-e", 2) == 0) {
					int argv1 = atoi(argv[2]);
					int argv2 = atoi(argv[3]);
					if (argv1 > 0 && argv2 >= argv1)
						psarc.read(argv[4], argv1, argv2);
				}
			}
			break;

		default:
			usage();
			break;
	}

	return EXIT_SUCCESS;
}
