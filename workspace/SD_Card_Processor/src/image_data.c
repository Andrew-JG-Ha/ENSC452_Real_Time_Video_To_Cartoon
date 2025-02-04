#include "image_data.h"

void storeImageInDDR() {
	rawDataStorage = (u8*)*SD_BUFFER;
//	rawDataStorage = (u8*) 0x07000000;
//	memcpy(rawDataStorage, (u8*)*VGA_BUFFER,1228800);
	imageStorage = (u8*) 0x06000000;

	u8 BMP_IMAGE_HEADER[] = {
		    // Bitmap File Header (14 bytes)
		    0x42, 0x4D,             // BM (bitmap identifier)
		    0x82, 0x00, 0x00, 0x00, // File size in bytes (70 bytes including headers and pixel data)
		    0x00, 0x00,             // Reserved (unused)
		    0x00, 0x00,             // Reserved (unused)
		    0x7A, 0x00, 0x00, 0x00, // Offset to start of pixel data (122 bytes including headers)

		    // DIB Header (40 bytes)
		    0x6C, 0x00, 0x00, 0x00, // DIB header size (40 bytes)
		    0x80, 0x02, 0x00, 0x00, // Image width (640 pixels)
		    0xE0, 0x01, 0x00, 0x00, // Image height (480 pixels)
		    0x01, 0x00,             // Number of color planes (must be 1)
		    0x10, 0x00,             // Bits per pixel (16 bits per pixel)
		    0x03, 0x00, 0x00, 0x00, // Compression method (BI_BITFIELDS)
		    0x7A, 0x60, 0x09, 0x00, // Size of raw image data
		    0xC4, 0x0E, 0x00, 0x00, // Horizontal resolution
		    0xC4, 0x0E, 0x00, 0x00, // Vertical resolution
		    0x00, 0x00, 0x00, 0x00, // Number of colors in the palette (not used)
		    0x00, 0x00, 0x00, 0x00, // Number of important colors (not used)

		    // Color Masks for RGBA4444 (16 bytes)
		    0x00, 0x0F, 0x00, 0x00, // Red mask (4 bytes)
		    0x00, 0xF0, 0x00, 0x00, // Green mask (4 bytes)
		    0x0F, 0x00, 0x00, 0x00, // Blue mask (4 bytes)
		    0xF0, 0x00, 0x00, 0x00, // Alpha mask (4 bytes)

			//52 bytes of 0x00 for some reason
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
	};

	BMP_HEADER_SIZE = sizeof(BMP_IMAGE_HEADER);
	BMP_DATA_SIZE = 640*480*2; // Factor of 2 due to 2 bytes per pixel in the BMP image
	for (int i=0; i < BMP_HEADER_SIZE; i++) {
		imageStorage[i] = BMP_IMAGE_HEADER[i];
	}

	u8 alpha = 0xF0;
	RGB rgb_data;
	HSV hsv_data;

	for (int j = 0; j < BMP_DATA_SIZE/2; j++) {
		//Isolate blue, green, and red components
		rgb_data.b = rawDataStorage[j*4 + 2] >> 4;
		rgb_data.g = rawDataStorage[j*4 + 1] >> 4;
		rgb_data.r = rawDataStorage[j*4] >> 4;

		imageStorage[BMP_HEADER_SIZE + j*2] = (u8) (alpha | rgb_data.b);
		imageStorage[BMP_HEADER_SIZE + j*2 + 1] = (u8) ((rgb_data.g << 4) | rgb_data.r);
	}
}

void write_BMP_to_SDCard() {
	xil_printf("Started writing image to SD Card.\r\n");

	FRESULT Result;
	UINT NumBytesWritten;
	BYTE work[FF_MAX_SS];
	u32 FileSize = BMP_HEADER_SIZE + BMP_DATA_SIZE;
	TCHAR *path = "0:/";

	// Force mount the default drive on the SD Card and check if it is ready to work
	Result = f_mount(&Fatfs, path, 0);
	if (Result != FR_OK) {
		xil_printf("Error mounting SD Card.\r\n");
		xil_printf("Result holds %d for debugging purposes.\r\n", Result);
		return;
	}

	Result = f_mkfs(path, FM_FAT32, 0, work, sizeof(work));
	if (Result != FR_OK) {
		xil_printf("Error initializing FAT32 System Parameters on SD Card.\r\n");
		xil_printf("Result holds %d for debugging purposes.\r\n", Result);
		return;
	}

	SD_File = (char * ) FileName;

//	generate_filename(SD_File);

	//Create a file to write to
	Result = f_open(&File, SD_File , FA_WRITE | FA_CREATE_ALWAYS | FA_READ);
	if (Result != FR_OK) {
			xil_printf("Error creating file on SD Card.\r\n");
			xil_printf("Result holds %d for debugging purposes.\r\n", Result);
			return;
	}

	//Set the file read/write pointer to the beginning of file
	Result = f_lseek(&File, 0);
	if (Result != FR_OK) {
		xil_printf("Error setting file pointer for writing to beginning of file on SD Card.\r\n");
		xil_printf("Result holds %d for debugging purposes.\r\n", Result);
		return;
	}

	//Write bmp data to file
	Result = f_write(&File, (const void*) imageStorage, FileSize, &NumBytesWritten);
	if (Result != FR_OK) {
			xil_printf("Error writing to file on SD Card.\r\n");
			xil_printf("Result holds %d for debugging purposes.\r\n", Result);
			f_close(&File);
			return;
	}

	//Close the file
    Result = f_close(&File);
    if (Result != FR_OK) {
        xil_printf("Error closing file on SD Card.\r\n");
        xil_printf("Result holds %d for debugging purposes.\r\n", Result);
        return;
    }

	// Force unmount the default drive on the SD Card and check if it is ready to work
	Result = f_mount(0, path, 0);
	if (Result != FR_OK) {
		xil_printf("Error unmounting SD Card.\r\n");
		xil_printf("Result holds %d for debugging purposes.\r\n", Result);
		return;
	}
	xil_printf("Image stored to SD Card!\r\n");
}

void generate_filename(char *filename) {
    sprintf(filename, "Image_%d.bmp", imageNumber);
    imageNumber++;
}
