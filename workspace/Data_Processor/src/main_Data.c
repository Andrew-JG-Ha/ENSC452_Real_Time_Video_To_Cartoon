#include <stdio.h>
#include "xaxivdma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xscugic.h"
#include "xil_types.h"
#include "platform.h"
#include "xil_printf.h"
#include "camera.h"
#include "editors.h"
#include "xpseudo_asm.h"
#include "xil_mmu.h"
#include "hardware_functions.h"

// https://www.youtube.com/watch?v=IGdr4QRrOcs

#define COMM_VAL (*(volatile unsigned long *)(0xFFFF0000))
#define VGA_BUFFER (*(volatile int **)(0xFFFF00FF))
#define SD_BUFFER (*(volatile int **)(0xFFFF01FF))

#define packagedHSize 320

#define HSizeInput 640 / 2 * 4
#define HStrideLength 640 / 2 * 4
#define VSizeInput 480 * 4

#define sev() __asm__("sev")
#define ARM1_STARTADR 0xFFFFFFF0
#define ARM1_BASEADDR 0x100000


int* writeBuffer1 = (int*) CAMERA_BASE_ADDR;
int* writeBuffer2 = (int*) CAMERA_BASE_ADDR + 4*320*480;
int* writeBuffer3 = (int*) CAMERA_BASE_ADDR + 2* 4*320*480;

int* vgaBuffer = (int*) 0x12000000;
int* grayBuffer = (int*) GRAYSCALE_BASE_ADDR;
int* rgbBuffer = (int*) RGB_BASE_ADDR;
int* sobelBuffer = (int*) SOBEL_BASE_ADDR;
int* hsvBuffer = (int*) HSV_BASE_ADDR;

HSV_mods HSV_modifiers = {1.0, 1.0, 1.0};
int userDefinedFilter[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
int userDefinedScaling = 1;

int sobelKernel[9] = {1, 0, -1, 2, 0, -2, 1, 0 ,-1};
int gaussianKernel[9] = {6, 12, 6, 12, 24, 12, 6, 12, 6};
int sharpenKernel[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
int ridgeKernel[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};

int main() {
	init_platform();
	Xil_DCacheDisable(); // Disable data cache
	Xil_Out32(ARM1_STARTADR, ARM1_BASEADDR); // Set correct base address for ARM1
	dmb(); // Wait for ARM1 to be set up

	COMM_VAL = 0;
	STORE_IMAGE = 0;

	sev(); // Wake up ARM1

	Xil_DCacheFlush(); //Flush the data cache before doing anything

	/* Initialize GPIO peripheral */
	gpio_init();

	int status;
	// Initialize interrupt controller
	status = IntcInitFunction(INTC_DEVICE_ID, &BTNInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	VGA_BUFFER = &vgaBuffer;

	XAxiVdma vdma;  // VDMA instance

	// Look up the VDMA configuration
	XAxiVdma_Config *vdma_config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);
	if (!vdma_config) {
	    xil_printf("VDMA device not found\r\n");
	}

	// Initialize the VDMA instance
	status = XAxiVdma_CfgInitialize(&vdma, vdma_config, vdma_config->BaseAddress);
	if (status != XST_SUCCESS) {
	    xil_printf("VDMA initialization failed\r\n");
	}

	// Configure DMA setup parameters
	XAxiVdma_DmaSetup dma_setup;
	dma_setup.VertSizeInput = VSizeInput;  // Vertical size of input frame
	dma_setup.HoriSizeInput = HSizeInput;  // Horizontal size of input frame
	dma_setup.Stride = HStrideLength;         // Stride (bytes per line)
	dma_setup.FrameDelay = 0;       // No frame delay
	dma_setup.EnableCircularBuf = 1; // Enable circular buffering
	dma_setup.FixedFrameStoreAddr = 0; // No fixed frame store address

	// Set up buffer addresses in DDR memory
	UINTPTR frame_buffer_addr[1];
	frame_buffer_addr[0] = &(writeBuffer1[0]);
	frame_buffer_addr[1] = &(writeBuffer2[0]);
	frame_buffer_addr[2] = &(writeBuffer3[0]);
	// Initialize frame_buffer_addr array with addresses of DDR memory buffers

	// Configure the VDMA for write operation
	status = XAxiVdma_DmaConfig(&vdma, XAXIVDMA_WRITE, &dma_setup);
	if (status != XST_SUCCESS) {
	    xil_printf("VDMA configuration failed\r\n");
	}

	// Set buffer addresses for writing
	status = XAxiVdma_DmaSetBufferAddr(&vdma, XAXIVDMA_WRITE, frame_buffer_addr);
	if (status != XST_SUCCESS) {
	    xil_printf("VDMA buffer address setup failed\r\n");
	}

	// Start the VDMA operation
	status = XAxiVdma_DmaStart(&vdma, XAXIVDMA_WRITE);
	if (status != XST_SUCCESS) {
	    xil_printf("VDMA start failed\r\n");
	}

	XAxiVdma_DmaStop(&vdma, XAXIVDMA_WRITE);

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x30, 0x83);
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xAC, &(writeBuffer1[0]));
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB0, &(writeBuffer2[0]));
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB4, &(writeBuffer3[0]));

	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA8, HStrideLength);
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA4, HSizeInput);
	Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA0, VSizeInput);

//    int data;
//    for (int i = 0; i < 500; i++) {
//    	data = writeBuffer1[i];
//    	printf("Data = 0x%08x, Memory Location = 0x%08x\n", data, (&writeBuffer1[i]));
//    }

//    for (int i = 0; i < 1000000; i++) {
//    	if (i % 10000 == 0) {
//        	data = writeBuffer[100];
//        	printf("Data = 0x%08x, Memory Location = 0x%08x\n", data, (&writeBuffer[100]));
//    	}
//    }

	Xil_DCacheEnable();
	printf("Start Up ...");
	for (int i = 0; i < 5000000; i++) {
		int index = 0;
		index = index + 1;
	}
	printf("....");

	Xil_DCacheFlush();
	printf("..");
	for (int i = 0; i < 640*480; i++) {
		vgaBuffer[i] = 0x00F0F0F0;
	}

	printf("...\n Start Up Done!\n");

    while(1) {
    	Xil_DCacheFlush();
//   	unpackCameraData(writeBuffer1, rgbBuffer, grayBuffer);
//    	applyKernelRGB(rgbBuffer, sobelBuffer, gaussianKernel, 4);
//    	applyKernelRGB(sobelBuffer, sobelBuffer2, gaussianKernel, 4);
//    	applyKernelRGB(sobelBuffer2, vgaBuffer, gaussianKernel, 4);
    	switch (STORE_IMAGE) {
    		case 0:
            	if (sw_value == 1) { // HSV adjustment
            		memcpy(STORAGE_BUFFER, writeBuffer1, CAM_DATA_BUFFER_SIZE*4);
            		raw_to_HSV_frame(STORAGE_BUFFER, vgaBuffer);
            	}
            	else if(sw_value == 2) { // Silly
            		raw_to_HSV_frame(writeBuffer1, hsvBuffer);
            		unpackGray(writeBuffer1, grayBuffer);
            		applyKernelGrey(grayBuffer, sobelBuffer, sobelKernel);
            		applySillyFilter(vgaBuffer, hsvBuffer, sobelBuffer);
            	}
            	else if(sw_value == 4) { // Grayscale
            		unpackGray(writeBuffer1, vgaBuffer);
            	}
            	else if(sw_value == 8) { // Sobel Kernel
            		unpackGray(writeBuffer1, grayBuffer);
            		applyKernelGrey(grayBuffer, vgaBuffer, sobelKernel);
            	}
            	else if(sw_value == 16) { // Gaussian Blur
            		unpackRGB(writeBuffer1, rgbBuffer);
            		applyKernelRGB(rgbBuffer, vgaBuffer, gaussianKernel, 8);
            	}
            	else if (sw_value == 32) { // user defined
            		unpackRGB(writeBuffer1, rgbBuffer);
            		applyKernelRGB(rgbBuffer, vgaBuffer, userDefinedFilter, userDefinedScaling);
            	}
            	else { // default RGB
            		unpackRGB(writeBuffer1, vgaBuffer);
            	}
    			break;
    		case 2:
        		Xil_DCacheEnable();
        		STORE_IMAGE = 0;
    			break;
    		default:
    			break;
    	}
//    	printf("ARM0 Online\r\n");
//    	unpackGray(writeBuffer1, vgaBuffer);

//    	xil_printf("Data at mem location: 0x%08x is 0x%08x\r\n", &(writeBuffer1[400]), writeBuffer1[400]);
//    	memcpy(vgaBuffer, writeBuffer1, 640*480*4);
//    	printf("unpacked and printed!\n");
    }

    cleanup_platform();
    return 0;
}
