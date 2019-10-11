/*
 * visi_genie_file_access.c
 *
 *  Created on: Sep 13, 2019
 *      Author: cruzj
 */

#include "visi_genie_file_access.h"


void genieFileBegin(uint8_t index) {
	magicFileIndex = index;
	magicFileBusy = false;
}

void genieFileAccessIsReady() {
	magicFileBusy = false;
}

void genieFileDir(char * filename) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_DIR;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileRead(char * filename) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_READ;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileWriteBytes(char * filename, uint8_t * data, uint8_t count) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_WRITE;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;
	memcpy(magicFileBuffer + len, data, count);
	len += count;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileWriteStr(char * filename, char * str) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_WRITE;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;
	memcpy(magicFileBuffer + len, str, strlen(str));
	len += strlen(str);

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileAppendBytes(char * filename, uint8_t * data, uint8_t count) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_APPEND;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;
	memcpy(magicFileBuffer + len, data, count);
	len += count;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileAppendStr(char * filename, char * str) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_APPEND;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;
	memcpy(magicFileBuffer + len, str, strlen(str));
	len += strlen(str);

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileErase(char * filename) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_ERASE;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

void genieFileSize(char * filename) {
	magicFileBusy = true;
	magicFileBuffer[0] = GENIEM_FILE_SIZE;
	uint8_t len = 1;
	memcpy(magicFileBuffer + len, (uint8_t*)filename, strlen(filename) + 1);
	len += strlen(filename) + 1;

	genieWriteMagicBytes(magicFileIndex, magicFileBuffer, len);

	while (magicFileBusy == true) {
		genieDoEvents();
	}
}

