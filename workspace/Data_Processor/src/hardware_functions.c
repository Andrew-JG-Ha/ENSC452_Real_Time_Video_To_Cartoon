/*
 * hardware_functions.c
 *
 * Contains all functions which pertain to setup and use of IP periperals.
 */

#include "hardware_functions.h"
#include "camera.h"

/* ---------------------------------------------------------------------------- *
 * 								gpio_init()		 							*
 * ---------------------------------------------------------------------------- *
 * Initialises the GPIO driver for the push buttons and switches.
 * ---------------------------------------------------------------------------- */
unsigned char gpio_init()
{
	int Status;

	Status = XGpio_Initialize(&BTNInst, BUTTON_SWITCH_ID);
	if(Status != XST_SUCCESS) return XST_FAILURE;

//	// Initialise Push Buttons
//	Status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
//	if(Status != XST_SUCCESS) return XST_FAILURE;

//	XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

	XGpio_SetDataDirection(&BTNInst, SWITCH_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&BTNInst, BUTTON_CHANNEL, 0xFF);

	return XST_SUCCESS;
}

// Interrupt functions

void BTN_Intr_Handler(void *InstancePtr)
{
	btn_value = XGpio_DiscreteRead(&BTNInst, BUTTON_CHANNEL);
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BUTTON_CHANNEL);
	XGpio_InterruptDisable(&BTNInst, SWITCH_CHANNEL);


	usleep(250000); //250ms sleep to effectively debounce signal
	sw_value = XGpio_DiscreteRead(&BTNInst, SWITCH_CHANNEL);

	if (sw_value == 1) {
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			memcpy(SD_BUFFER, (void*) 0x12000000, 1228800);
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
		else if (btn_value == 4) {
			if ((current_HSV_setting % 3 == 0)) {
				HSV_modifiers.hmod -= 0.0625;
			}
			else if ((current_HSV_setting % 3 == 1)) {
				HSV_modifiers.smod -= 0.0625;
			}
			else if ((current_HSV_setting % 3 == 2)) {
				HSV_modifiers.vmod -= 0.0625;
			}
		}
		else if (btn_value == 8) {
			if ((current_HSV_setting % 3 == 0)) {
				HSV_modifiers.hmod += 0.0625;
			}
			else if ((current_HSV_setting % 3 == 1)) {
				HSV_modifiers.smod += 0.0625;
			}
			else if ((current_HSV_setting % 3 == 2)) {
				HSV_modifiers.vmod += 0.0625;
			}
		}
		xil_printf("---------------------------------------------------------\r\n");
		xil_printf("HSV Settings are modifiable and are all currently set to: \r\n");
		printf("HUE modifier is %.4f\n\r", HSV_modifiers.hmod);
		printf("SATURATION modifier is %.4f\n\r", HSV_modifiers.smod);
		printf("VALUE modifier is %.4f\n\r", HSV_modifiers.vmod);
		xil_printf("Press BTNL to decrease the modifier and BTNR to increase the modifier.\r\n");
		xil_printf("Press BTNU to choose a different modifier to change.\r\n");
		if (btn_value == 16) {
			current_HSV_setting++;
			printf("The modifier has been changed to %s.\r\n", HSV_setting[current_HSV_setting % 3]);
		}
		else {
			printf("The current modifier is %s\r\n", HSV_setting[current_HSV_setting % 3]);
		}
		xil_printf("---------------------------------------------------------\r\n\n");
	}
	else if (sw_value == 2) {
		printf("Silly Filter is live!\r\n");
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
	}
	else if (sw_value == 4) {
		printf("Gray Filter is live!\r\n");
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
	}
	else if (sw_value == 8) {
		printf("Sobel Kernel Filter is live!\r\n");
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
	}
	else if (sw_value == 16) {
		printf("Gaussian Smoothing is live!\r\n");
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
	}
	else if (sw_value == 32) {
		printf("User-Defined Filtering is live!\r\n");

		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
		}
		else if (btn_value == 8) {
			if (currentFilterLoc % 10 == 9) {
				// scaling factor
				userDefinedScaling = (userDefinedScaling + 1) >= 8 ? 8 : (userDefinedScaling + 1);
			}
			else {
				userDefinedFilter[currentFilterLoc] = (userDefinedFilter[currentFilterLoc] + 1) >= 16 ? 16 : userDefinedFilter[currentFilterLoc] + 1;
			}

		}
		else if (btn_value == 4) {
			if (currentFilterLoc % 10 == 9) {
				// scaling factor
				userDefinedScaling = (userDefinedScaling - 1) <= 1 ? 1 : (userDefinedScaling - 1);
			}
			else {
				userDefinedFilter[currentFilterLoc] = (userDefinedFilter[currentFilterLoc] - 1) <= -16 ? -16 : userDefinedFilter[currentFilterLoc] - 1;
			}
		}

		printf("---------------------------------------------------------\r\n");
		printf("User-Defined Filter Settings are modifiable and are currently set to: \r\n");
		printf("%-5d %-5d %-5d\n\n", userDefinedFilter[0], userDefinedFilter[1], userDefinedFilter[2]);
		printf("%-5d %-5d %-5d\n\n", userDefinedFilter[3], userDefinedFilter[4], userDefinedFilter[5]);
		printf("%-5d %-5d %-5d\n", userDefinedFilter[6], userDefinedFilter[7], userDefinedFilter[8]);
		printf("Scaling factor: %3d \n", userDefinedScaling);
		printf("Press BTNL to decrease the modifier and BTNR to increase the modifier.\r\n");
		printf("Press BTNU or BTND to choose a filter value to change.\r\n");

		if (btn_value == 16) {
			currentFilterLoc = (currentFilterLoc + 1) >= 9 ? 9 : (currentFilterLoc + 1);
		}
		else if (btn_value == 2) {
			currentFilterLoc = (currentFilterLoc - 1) <= 0 ? 0 : (currentFilterLoc - 1);
		}

		printf("Currently changing: %s.\r\n", filter_settings[currentFilterLoc]);
		printf("---------------------------------------------------------\r\n");

	}
	else {
		if (btn_value == 1) {
			//BTNC
			xil_printf("Taking a photo! Smile!\r\n");
			Xil_DCacheDisable();
			STORE_IMAGE = 1;
//			storeImageInDDR();
//			write_BMP_to_SDCard();
		}
		else if (btn_value == 2) {
			//BTND
				xil_printf("BTND Pressed!\r\n");
			}
		else if (btn_value == 4) {
			//BTNL
			xil_printf("BTNL Pressed!\r\n");
			}
		else if (btn_value == 8) {
			//BTNR
			xil_printf("BTNR Pressed!\r\n");
			}
		else if (btn_value == 16) {
			//BTNU
			xil_printf("BTNU Pressed!\r\n");
		}
	}

    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    (void)XGpio_InterruptClear(&BTNInst, 2);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);
    XGpio_InterruptEnable(&BTNInst, 2);
}

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialization
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 3);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	return XST_SUCCESS;
}
