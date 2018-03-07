/*
**********************************************************************************
ADD LICENSE
**********************************************************************************
*  $Revision:
*  Description: This version supports SPI,I2C,UART Bridging and Programming Config file
**********************************************************************************
* $File:  MchpUSBINterface.cpp
*/

#include <stdio.h>
#include <libusb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

//PT2 SDK Header file.
#include "typedef.h"
#include "MchpUSBInterface.h"
#include "USBHubAbstraction.h"

//DLL Exports
#define FALSE								0
#define TRUE								1
#define min(a,b)							(((a) < (b)) ? (a) : (b))
#define MICROCHIP_HUB_VID						0x424

#define VID_MICROCHIP							0x0424
// #define PID_HCE_DEVICE							0x4940

#define CMD_SPI_PASSTHRU_ENTER				0x60  //SB
#define CMD_SPI_PASSTHRU_EXIT				0x62  //SB

//OTP
#define OTP_DATA_START_ADDRESS						0x0002
#define BIG_ENDIAN_WORD(w) 						((((w)&0xFF)<<8) | (((w)&0xFF00) >> 8))
#define CMD_OTP_RESET                       0x08

#define CONVERT_ENDIAN_DWORD(w)	((((DWORD32)(w)) << 24) | (((DWORD32)(w) & 0xFF00) << 8) | \
								 (((DWORD32)(w) & 0xFF0000) >> 8) | (((DWORD32)(w) & 0xFF000000) >> 24))

#define PT2_LIB_VER							"1.23"

// #define CTRL_RETRIES 							2
#define HUB_STATUS_BYTELEN						3 /* max 3 bytes status = hub + 23 ports */
// #define HUB_DESC_NUM_PORTS_OFFSET					2
#define HUB_SKUs                                6


#define logprint(x, ...) do { \
		printf(__VA_ARGS__); \
		fprintf(x,  __VA_ARGS__); \

//SPI fLASH Specific Macros
#define WRITE_BLOCK_SIZE                    256
#define READ_BLOCK_SIZE                     512
#define READ_JEDEC_ID					    0x9F
#define	WREN							    0x06
#define WRDIS                               0X04
#define	RDSR								0x05
#define ULBPR								0x98
#define CHIP_ERASE							0xC7
#define PAGE_PROG							0x02
#define HS_READ                             0x0B

//OTP
// typedef struct tagOtpCfgChecksumA
// {
// 	uint8_t	abyXORChecksum;
// 	uint16_t	wCfgStartOffset;
// 	uint16_t	wCfgLength;
//
// }OTP_CFG_CHECKSUM;
//
// typedef struct tagOtpCfgChecksumA1
// {
// 	uint8_t	abySignature [3];
// 	OTP_CFG_CHECKSUM otpCfgChecksum;
//
// }OTP_CFG_CHECKSUM_A1, *POTP_CFG_CHECKSUM_A1;

/*-----------------------Helper functions --------------------------*/
int usb_enable_HCE_device(uint8_t hub_index);
static int compare_hubs(const void *p1, const void *p2);
static int usb_get_hubs(PHINFO pHubInfoList);
static int usb_get_hub_list(PCHAR pHubInfoList);
static int usb_open_HCE_device(uint8_t hub_index);
int  usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue) ;
// bool UsbSetBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToSet);
// bool UsbClearBitXdata(int hub_index,WORD wXDataAddress,BYTE byBitToClear);
int Read_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes);
int Write_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes);
int xdata_read(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes);
int xdata_write(HANDLE handle, uint32_t wAddress, uint8_t *data, uint8_t num_bytes);

// //OTP Programming
// unsigned int CalculateNumberofOnes(unsigned int UINTVar);

 // Global variable for tracking the list of hubs
HINFO gasHubInfo [MAX_HUBS];
/* Context Variable used for initializing LibUSB session */
libusb_context *ctx = NULL;
/*List of possible PIDs for USB491X/USB471X HFCs*/
uint16_t PID_HCE_DEVICE[HUB_SKUs] = {0x4940, 0x494A, 0x494B, 0x494C, 0x494E, 0x494F};

/*-----------------------API functions --------------------------*/
BOOL  MchpUsbGetVersion ( PCHAR pchVersionNo )
{
	BOOL bRet = TRUE;

	//Send command to lib to get library version
	sprintf(pchVersionNo,"%s",PT2_LIB_VER);

	return bRet;

}

// Get last error for the specific hub instance.
UINT32 MchpUsbGetLastErr (HANDLE DevID)
{
	DevID = DevID;
	return errno;
}

int MchpGetHubList(PCHAR pchHubcount )
{
	int hub_count =0;
	hub_count  = usb_get_hub_list(pchHubcount);
	return hub_count;
}

//Return handle to the first instance of VendorID &amp; ProductID matched device.
// HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID)
// {
// 	int error = 0, hub_cnt=0, hub_index=0;
// 	int restart_count=5;
// 	bool bhub_found = false;
//
// 	//Get the list of the hubs from the device.
// 	hub_cnt = usb_get_hubs(&gasHubInfo[0]);
// 	do
// 	{
// 		if((gasHubInfo[hub_index].wVID == wVID) && (gasHubInfo[hub_index].wPID == wPID))
// 		{
// 			bhub_found = true;
// 			break;
// 		}
//
// 	}
// 	while(hub_index++ < hub_cnt);
//
// 	if(false == bhub_found)
// 	{
// 		DEBUGPRINT("MCHP_Error_Device_Not_Found \n");
// 		return INVALID_HANDLE_VALUE;
// 	}
//
// 	error = usb_open_HCE_device(hub_index);
// 	if(error < 0)
// 	{
//
// 		//enable 5th Endpoit
// 		error = usb_enable_HCE_device(hub_index);
//
// 		if(error < 0)
// 		{
// 			DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to Enable the device \n");
// 			return INVALID_HANDLE_VALUE;
// 		}
// 		do
// 		{
// 			sleep(2);
// 			error = usb_open_HCE_device(hub_index);
// 			if(error == 0)
// 			{
// 				return hub_index;
// 			}
//
// 		}while(restart_count--);
//
// 		DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to open the device error:%d\n",error);
// 		return INVALID_HANDLE_VALUE;
// 	}
//
//
// 	return hub_index;
// }

//Return handle to the first instance of VendorID &amp; ProductID &amp; port path matched device.
HANDLE  MchpUsbOpen ( UINT16 wVID, UINT16 wPID,char* cDevicePath)
{
    	int error = 0, hub_index=0;
    	int restart_count=5;
    	bool bhub_found = false;

	char *ptr;
	BYTE DeviceID[7];
	int  i=0 ;
	ptr = cDevicePath;


	while(*ptr != '\0')
	{
		if(*ptr != ':')
		{
			DeviceID[i] = *ptr - 48;
			i++;
		}
		ptr++;
	}


	if( 0 == usb_get_hubs(&gasHubInfo[0]))
	{
		DEBUGPRINT("MCHP_Error_libusb_error_no_hub \n");
		return INVALID_HANDLE_VALUE;
	}

    	for(;hub_index < 10;hub_index++)
    	{
		if((gasHubInfo[hub_index].wVID == wVID) && (gasHubInfo[hub_index].wPID == wPID) && (i == gasHubInfo[hub_index].port_max))
		{
			 for (int j=0; j<gasHubInfo[hub_index].port_max; j++)
			{
				if(gasHubInfo[hub_index].port_list[j] != DeviceID[j])
				{
					bhub_found = false;
					break;

				}
				bhub_found = true;

			}
			if(true == bhub_found)
			{
				break;
			}


		}

	}

	if(false == bhub_found)
    	{
     		DEBUGPRINT("MCHP_Error_Device_Not_Found \n");
        	return INVALID_HANDLE_VALUE;
   	}

    	error = usb_open_HCE_device(hub_index);

    	if(error < 0)
    	{
        //enable 5th Endpoit
		error = usb_enable_HCE_device(hub_index);

 	       if(error < 0)
        	{
            		DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to Enable the device \n");
            		return INVALID_HANDLE_VALUE;
        	}
        	do
       		{
            		sleep(2);
            		error = usb_open_HCE_device(hub_index);
            		if(error == 0)
            		{
                 		return hub_index;
            		}

        	}while(restart_count--);

       		DEBUGPRINT("MCHP_Error_Invalid_Device_Handle: Failed to open the device error:%d\n",error);
        	return INVALID_HANDLE_VALUE;
 	}

   	return hub_index;
}


//Close the device handle.
BOOL MchpUsbClose(HANDLE DevID)
{

	if(gasHubInfo[DevID].handle != NULL)
	{
		libusb_close((libusb_device_handle*)gasHubInfo[DevID].handle);
		gasHubInfo[DevID].dev = NULL;
		gasHubInfo[DevID].handle = NULL;
	}
	else
	{
		printf("unknown hub index%d\n", DevID);
		return false;
	}
	libusb_exit(ctx);
	return true;
}

BOOL MchpUsbSpiSetConfig ( HANDLE DevID, INT EnterExit)
{
	int rc = FALSE;
	BOOL bRet = FALSE;
	BYTE byCmd =0;

	if(EnterExit)
	{
		byCmd = CMD_SPI_PASSTHRU_ENTER;
	}
	else
	{
		byCmd = CMD_SPI_PASSTHRU_EXIT;
	}
	rc = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,byCmd,0,0,0,0,500);
	if(rc < 0)
	{
		DEBUGPRINT("MchpUsbSpiSetConfig failed\n");
		bRet = FALSE;
	}
	else
	{
		DEBUGPRINT("MchpUsbSpiSetConfig Passed\n");
		bRet = TRUE;
	}
	return bRet;
}

// BOOL MchpUsbSpiFlashRead(HANDLE DevID,UINT32 StartAddr,UINT8* InputData,UINT32 BytesToRead)
// {
	// BOOL bRet = FALSE;
    //
	// if ((StartAddr + BytesToRead) > MAX_FW_SIZE)
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashRead Failed: BytesToRead (%d) and StartAddr(0x%x) is larger than SPI memory size\n",BytesToRead,StartAddr);
	// 	return bRet;
	// }
	// //boots from SPI
	// bRet = AthensReadSPIFlash(DevID,InputData,StartAddr,(StartAddr + BytesToRead));
    //
	// if(FALSE == bRet)
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashRead Failed \n");
	// }
	// else
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashRead Passed \n");
	// 	bRet = TRUE;
	// }
	// return bRet;
// }

/*New Definition - Pre-req: Hub needs to be open*/
BOOL MchpUsbSpiFlashRead(HANDLE DevID,UINT32 StartAddr,UINT8* InputData,UINT32 BytesToRead)
{
    uint16_t NumPageWrites = 0;
    uint8_t RemainderBytes = 0;
    uint8_t byReadBuffer[READ_BLOCK_SIZE+5];
    uint8_t byBuffer[4] = {0,0,0,0};

    //Enable the SPI interface.
    if(FALSE == MchpUsbSpiSetConfig (DevID,1))
    {
        printf ("\nError: SPI Pass thru enter failed:\n");
        exit (1);
    }

    NumPageWrites = MAX_FW_SIZE / READ_BLOCK_SIZE;
    RemainderBytes = MAX_FW_SIZE % READ_BLOCK_SIZE;

    for(uint16_t i=0; i<NumPageWrites; i++)
    {
        byBuffer[0] = HS_READ;
        byBuffer[1] = (StartAddr & 0xFF0000) >> 16; //SB
        byBuffer[2] = (StartAddr & 0x00FF00) >> 8; //SB
        byBuffer[3] = StartAddr & 0x0000FF; //SB

        //performs write operation to the SPI Interface.	//SB
        if(FALSE == MchpUsbSpiTransfer(DevID,0,byBuffer,4,READ_BLOCK_SIZE+5)) //write
        {
            printf("SPI Transfer write failed \n");
            exit (1);
        }

        //performs read operation to the SPI Interface.	//SB
        if(FALSE == MchpUsbSpiTransfer(DevID,1,(uint8_t *)&byReadBuffer,READ_BLOCK_SIZE,READ_BLOCK_SIZE+5)) //parameter3 is don't care
        {
            printf("SPI Transfer read failed \n");
            exit (1);
        }

        /*Copying the block of data read into local buffer for writing to binary file*/
        memcpy((void *)&InputData[i*512], (const void *)&byReadBuffer[1], 512);

        //Check if the flash is BUSY
        byBuffer[0] = 0x05;
        do
        {
            //performs write operation to the SPI Interface.	//SB
            if(FALSE == MchpUsbSpiTransfer(DevID,0,&byBuffer[0],1,2)) //write
            {
                printf("SPI Transfer write failed \n");
                exit (1);
            }

            if(FALSE == MchpUsbSpiTransfer(DevID,1,(uint8_t *)&byReadBuffer,2,1)) //parameter3 is don't care
            {
                printf("SPI Transfer read failed \n");
                exit (1);
            }
            printf("Reading page %d at addr 0x%06x...SR = %02x\n",i,StartAddr,byReadBuffer[0]);

        }while(byReadBuffer[0] != 0x00);

        StartAddr += READ_BLOCK_SIZE;
    }

    //Disable the SPI interface.
    if(FALSE == MchpUsbSpiSetConfig (DevID,0))
    {
        printf ("Error: SPI Pass thru enter failed:\n");
        exit (1);
    }

    return TRUE;
}



BOOL MchpUsbSpiFlashWrite(HANDLE DevID,UINT32 StartAddr,UINT8* OutputData, UINT32 BytesToWrite)
{
	// BOOL bRet = FALSE;
	// if(nullptr == OutputData)
	// {
	// 	DEBUGPRINT("SPI Write failed: NULL pointer");
	// 	return FALSE;
	// }
    //
	// if ((StartAddr + BytesToWrite) > MAX_FW_SIZE)
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashWrite Failed: BytesToWrite (%d) and StartAddr(0x%x) is larger than SPI memory size\n",BytesToWrite,StartAddr);
	// 	return bRet;
	// }
	// bRet = WriteSPIFlash(DevID,OutputData,5000,TRUE,StartAddr,BytesToWrite);
    //
	// if(FALSE == bRet)
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashWrite failed \n");
	// }
	// else
	// {
	// 	DEBUGPRINT("MchpUsbSpiFlashWrite success \n");
	// 	bRet = TRUE;
	// }
	// //Reset the device.
	// uint8_t byData = 0;
	// xdata_read(DevID, 0x804, &byData, 1);
    //
	// byData|= 0x04;
	// xdata_write(DevID, 0x804, &byData, 1);
    //
	// byData = 0x40;
	// xdata_write(DevID, 0x80A, &byData, 1);
    //
	// return bRet;
}
BOOL MchpUsbSpiTransfer(HANDLE DevID,INT Direction,UINT8* Buffer, UINT16 DataLength,UINT32 TotalLength)
{
	int bRetVal = FALSE;
	if(nullptr == Buffer)
	{
		DEBUGPRINT("SPI Transfer failed: NULL pointer");
		return FALSE;
	}

	if(Direction) //Read
	{
		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0xC1,0x04,0x2310,0xBFD2,Buffer,
								TotalLength,CTRL_TIMEOUT);
	}
	else //Write
	{
		bRetVal = libusb_control_transfer((libusb_device_handle*)gasHubInfo[DevID].handle,0x41,0x61,TotalLength,0,Buffer,
					DataLength,CTRL_TIMEOUT);
	}
	if(bRetVal <0 )
	{
		DEBUGPRINT("SPI Transfer Failed\n");
		return FALSE;
	}
	else
	{
		DEBUGPRINT("SPI Transfer success\n");
		return TRUE;
	}
}

uint8_t ForceBootFromRom(HANDLE handle)
{
    uint8_t bRetVal = FALSE;
    uint8_t abyBuffer[4] = {'D','S','P','I'};

    //Writing the signature to disable SPI ROM
    // bRetVal = xdata_write(handle, 0xBFD227EC, abyBuffer, sizeof(abyBuffer));
    bRetVal = libusb_control_transfer ((libusb_device_handle*)gasHubInfo[handle].handle,
		0x40,
		0x03,
		0x27EC,
		0xBFD2,
		abyBuffer,
		4,
		CTRL_TIMEOUT
	);

    if(FALSE == bRetVal)
    {
        printf("Disable SPI signature write failed\n");
        return bRetVal;
    }

    // //Enable the SPI interface.
    // if(FALSE == MchpUsbSpiSetConfig (handle,1))
    // {
    //     printf ("\nError: SPI Pass thru enter failed:\n");
    //     exit (1);
    // }

    // abyBuffer[0] = 0x66;
    // if(FALSE == MchpUsbSpiTransfer(handle,0,&abyBuffer[0],1,1)) //write
    // {
    //     printf("SPI Transfer write failed \n");
    //     exit (1);
    // }
    //
    // abyBuffer[0] = 0x99;
    // if(FALSE == MchpUsbSpiTransfer(handle,0,&abyBuffer[0],1,1)) //write
    // {
    //     printf("SPI Transfer write failed \n");
    //     exit (1);
    // }
    //
    // //Disable the SPI interface.
    // if(FALSE == MchpUsbSpiSetConfig (handle,0))
    // {
    //     printf ("Error: SPI Pass thru enter failed:\n");
    //     exit (1);
    // }

    //Issuing a Soft RESET
    abyBuffer[0] = CMD_OTP_RESET;
    // xdata_write(handle, 0xBFD1DA1C, abyBuffer, 1);
    bRetVal = libusb_control_transfer ((libusb_device_handle*)gasHubInfo[handle].handle,
		0x40,
		0x03,
		0xDA1C,
		0xBFD1,
		abyBuffer,
		1,
		CTRL_TIMEOUT
	);

    // usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue)
    // abyBuffer[0] = 0x01;
    // abyBuffer[1] = 0x03;
    // bRetVal = usb_send_vsm_command((libusb_device_handle*)gasHubInfo[handle].handle, abyBuffer);
    //
    if(FALSE == bRetVal)
    {
        printf("Force Boot from ROM failed\n");
        return bRetVal;
    }

    bRetVal = libusb_control_transfer ((libusb_device_handle*)gasHubInfo[handle].handle,
		0x41,
		0x29,
		0x0001,
		0x0000,
		0,
		0,
		CTRL_TIMEOUT
	);

    sleep(2);
    return bRetVal;
}

// BOOL MchpProgramFile( HANDLE DevID, PCHAR InputFileName)
// {
// 	BOOL bRet = FALSE;
// 	//Read entire OTP
// 	uint8_t abyBuffer[2048];
// 	uint8_t  pbyBuffer[64];
// 	uint8_t wDataLength;
// 	uint16_t wConfigBytes;
// 	uint16_t wNumberOfOnes;
// 	wDataLength = ReadBinfile(InputFileName,pbyBuffer);
// 	if(0 == wDataLength)
// 	{
// 		DEBUGPRINT("Failed to Read Given Configuration File \n");
// 		return bRet;
// 	}
// 	bRet= Read_OTP(DevID, 0, abyBuffer, 2048);
// 	if(bRet < 0)
// 	{
// 		DEBUGPRINT("Failed to Read OTP Content \n");
// 		return bRet;
// 	}
//
// 	//find whether device boots from SPI or ROM
// 	Get_Hub_Info(DevID, (uint8_t *)&gasHubInfo[DevID].sHubInfo);
//
// 	// Update Number of configuration updated in OTP
// 	//Note that by default 0th byte is 0x00 and 1st byte is 0xff present in the OTP.
// 	//That is why xor is used below command
// 	if(gasHubInfo[DevID].sHubInfo.byFeaturesFlag & 0x01)
// 	{
// 		wConfigBytes = (abyBuffer[0] << 8) | (abyBuffer[1]);
// 		wNumberOfOnes = CalculateNumberofOnes(wConfigBytes);
// 		wNumberOfOnes = (16 - wNumberOfOnes);
// 		wConfigBytes &= ~(1 << wNumberOfOnes);
//
// 		//Update The OTP buffer
// 		abyBuffer[0] = (uint8_t)((wConfigBytes & 0xFF00) >> 8); //MSB
// 		abyBuffer[1] = (uint8_t)(wConfigBytes & 0x00FF); //LSB
// 	}
// 	else
// 	{
// 		wConfigBytes = (abyBuffer[0] << 8) |(abyBuffer[1] ^ 0xFF);
// 		//Calculate number of configuration present in OTP
// 		wNumberOfOnes = CalculateNumberofOnes(wConfigBytes);
// 		//Set the BitMask
// 		wConfigBytes = wConfigBytes | (1 << wNumberOfOnes);
//
// 		//Update the OTP buffer for indicating programming count is incremented by one.
// 		//First two bytes will represent the number of times the OTP is programmed.
// 		abyBuffer[0] = (uint8_t)((wConfigBytes & 0xFF00) >> 8); //MSB
// 		abyBuffer[1] = ((uint8_t)(wConfigBytes & 0x00FF) ^ 0xFF ); //LSB
// 	}
//
//
// 	//This is the logic for finding the OTP configuartion record update and data update.
// 	//Start from lowest index
// 	//By deafult, Data starts at 2 if no header found and record header will point end
// 	//of otp minus the last configuarion data(2048-8).
// 	uint16_t gwOTPDataOffset = OTP_DATA_START_ADDRESS;
// 	uint16_t gwOTPHdrOffset = 2048 - sizeof(OTP_CFG_CHECKSUM_A1);
// 	uint16_t wTmpOTPDataOffset =0, wTmpLenght=0;
// 	OTP_CFG_CHECKSUM_A1 *pstChecksumA1 = NULL;
//
// 	pstChecksumA1 = (OTP_CFG_CHECKSUM_A1 *) &abyBuffer[gwOTPHdrOffset];
//
// 	wTmpOTPDataOffset = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgStartOffset);
// 	wTmpLenght = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgLength);
//
// 	while (('I' == pstChecksumA1->abySignature [0]) && \
// 			('D' == pstChecksumA1->abySignature [1]) && \
// 			('X' == pstChecksumA1->abySignature [2]))
// 	{
// 		if ((wTmpOTPDataOffset > 0x0800) || \
// 			(wTmpLenght > 0x0800))
// 		{
// 			// Though signature matched, still the offset or the length field is
// 			// indicating OTP access more than 2K, which is invalid
// 			// Probably an invlid index record, where the random bytes matched "IDX" pattern.
// 			DEBUGPRINT("Trying to access more than 2k OTP memory\n");
// 			return bRet;
// 		}
//
// 		// Update the data offset as valid header is found
// 		gwOTPDataOffset = (wTmpOTPDataOffset + wTmpLenght);
//
// 		// Move to next header
// 		pstChecksumA1 --;
// 		gwOTPHdrOffset-=sizeof(OTP_CFG_CHECKSUM_A1);
//
// 		wTmpOTPDataOffset = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgStartOffset);
// 		wTmpLenght = BIG_ENDIAN_WORD (pstChecksumA1->otpCfgChecksum.wCfgLength);
// 	}
// 	uint16_t wTotalIndexSize = (2048 - gwOTPHdrOffset);
//
// 	if(wDataLength >= (unsigned int)(2048 - gwOTPDataOffset - wTotalIndexSize))
// 	{
// 		DEBUGPRINT("Error: No more free space available for programming OTP\n");
// 		return bRet;
// 	}
// 	//////////////////////////////////////////////
// 	// Update the OTP buffer for indicating programming count is incremented by one
// 	bRet = Write_OTP(DevID, 0, abyBuffer, 2);
// 	if(bRet < 0)
// 	{
// 		DEBUGPRINT("Failed to write OTP \n");
// 		return bRet;
// 	}
// 	//////////////////////////////////////////////
// 	// Update the otp data with new cfg block
// 	bRet = Write_OTP(DevID, gwOTPDataOffset, pbyBuffer, wDataLength);
// 	if(bRet < 0)
// 	{
// 		DEBUGPRINT ("Failed to write OTP \n");
// 		return bRet;
// 	}
// 	//For comparing after programming.
// 	memcpy (&abyBuffer[gwOTPDataOffset], pbyBuffer, wDataLength);
//
// 	uint8_t byChecksum ;
// 	OTP_CFG_CHECKSUM_A1 stChecksum;
//
// 	// Calculate the checksum
// 	for (int i = 0; i < wDataLength; i++)
// 	{
// 		byChecksum ^= pbyBuffer [i];
// 	}
// 	//OTP_CFG_CHECKSUM_A1 stChecksum;
// 	stChecksum.abySignature [0] = 'I';
// 	stChecksum.abySignature [1] = 'D';
// 	stChecksum.abySignature [2] = 'X';
//
// 	stChecksum.otpCfgChecksum.wCfgStartOffset = BIG_ENDIAN_WORD (gwOTPDataOffset);
// 	stChecksum.otpCfgChecksum.wCfgLength = BIG_ENDIAN_WORD (wDataLength);
// 	stChecksum.otpCfgChecksum.abyXORChecksum = byChecksum;
//
// 	//For comparing after programming.
// 	memcpy (&abyBuffer[gwOTPHdrOffset], &stChecksum, sizeof (OTP_CFG_CHECKSUM_A1));
//
// 	bRet = Write_OTP(DevID, gwOTPHdrOffset, (uint8_t *)&stChecksum, sizeof (OTP_CFG_CHECKSUM_A1));
// 	if(bRet < 0)
// 	{
// 		DEBUGPRINT("Failed to write OTP \n");
// 		return bRet;
// 	}
// 	sleep (2);
//
// 	//Verify OTP
// 	uint8_t abyVerifyBuffer[2048];
// 	bRet = Read_OTP(DevID, 0, abyVerifyBuffer, 2048);
// 	if(bRet < 0)
// 	{
// 		DEBUGPRINT("Failed to Read Config Memory \n");
// 		return bRet;
// 	}
// 	if(0 == memcmp(abyVerifyBuffer, abyBuffer, 2048))
// 	{
// 		printf("OTP wrote successfully\n");
// 		writeBinfile("actual_otp_data.bin",  abyBuffer, 2048);
// 		bRet = TRUE;
// 	}
// 	else
// 	{
// 		printf("Mismatch in OTP read data\n");
// 		writeBinfile("expected_otp_data.bin",  abyBuffer, 2048);
// 		writeBinfile("actual_otp_data.bin",  abyVerifyBuffer, 2048);
// 	}
// 	//Reset the device.
// 	uint8_t byData = 0;
// 	xdata_read(DevID, 0x804, &byData, 1);
//
// 	byData|= 0x04;
// 	xdata_write(DevID, 0x804, &byData, 1);
//
// 	byData = 0x40;
// 	xdata_write(DevID, 0x80A, &byData, 1);
//
// 	return bRet;
// }
/*----------------------- Helper functions -----------------------------------*/
static int usb_get_hubs(PHINFO pHubInfoList)
{
	int cnt = 0, hubcnt = 0, i = 0, error=0;
	libusb_device **devs;
	libusb_device_descriptor desc;
	libusb_device_handle *handle;
	PHINFO pHubListHead = pHubInfoList;	// Pointer to head of the list

	error = libusb_init(&ctx);
	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}

	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0)
	{
		DEBUGPRINT("Failed to get the device list \n");
		return -1;
	}


	for (i = 0; i < cnt; i++)
	{
		int error = 0;
		int value = 0;

		libusb_device *device = devs[i];

		error = libusb_get_device_descriptor(device, &desc);
		if(error != 0)
		{
			DEBUGPRINT("LIBUSB_ERROR: Failed to retrieve device descriptor for device[%d] \n", i);
		}


		if((error ==  0) && (desc.bDeviceClass == LIBUSB_CLASS_HUB))
		{
			uint8_t hub_desc[ 7 /* base descriptor */
							+ 2 /* bitmasks */ * HUB_STATUS_BYTELEN];


		  	error = libusb_open(device, &handle);
			if(error < 0)
			{
				DEBUGPRINT("Cannot open device[%d] \t", i);
				switch(error)
				{
					case LIBUSB_ERROR_NO_MEM:
						DEBUGPRINT("LIBUSB_ERROR_NO_MEM \n");
					break;
					case LIBUSB_ERROR_ACCESS:
						DEBUGPRINT("LIBUSB_ERROR_ACCESS \n");
					break;
					case LIBUSB_ERROR_NO_DEVICE:
						DEBUGPRINT("LIBUSB_ERROR_NO_DEVICE \n");
					break;
					default:
						DEBUGPRINT("UNKNOWN_LIBUSB_ERROR %x\n", error);
					break;
				}
				continue;
			}
		 	memset(hub_desc, 0, 9);

			if(desc.bcdUSB == 0x0300)
			{
				value = 0x2A;
			}
		 	else
			{
		  		value = 0x29;
			}

			error = libusb_control_transfer(handle,
											LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE,
											LIBUSB_REQUEST_GET_DESCRIPTOR,
											value << 8, 0, hub_desc, sizeof hub_desc, CTRL_TIMEOUT
											);

		  	if(error < 0)
			{
				DEBUGPRINT("libusb_control_transfer device[%d]: \t", i);
				switch(error)
				{
					case LIBUSB_ERROR_TIMEOUT:
						DEBUGPRINT("LIBUSB_ERROR_TIMEOUT \n");
					break;
					case LIBUSB_ERROR_PIPE:
						DEBUGPRINT("LIBUSB_ERROR_PIPE \n");
					break;
					case LIBUSB_ERROR_NO_DEVICE:
						DEBUGPRINT("LIBUSB_ERROR_NO_DEVICE \n");
					break;
					default:
						DEBUGPRINT("UNKNOWN_LIBUSB_ERROR \n");
					break;
				}
				libusb_close(handle);
				continue;
			}

			pHubInfoList->port_max = libusb_get_port_numbers(device, pHubInfoList->port_list, 7);

		  	if(pHubInfoList->port_max <= 0)
			{
				libusb_close(handle);
				continue;
			}
			pHubInfoList->byPorts	= hub_desc[3];
			libusb_close(handle);

			pHubInfoList->wVID	 	= desc.idVendor;
			pHubInfoList->wPID 		= desc.idProduct;

			pHubInfoList++;
			hubcnt++;
		}
	}

	libusb_free_device_list(devs, 1);

	qsort(pHubListHead, hubcnt, sizeof(HINFO), compare_hubs);

	return hubcnt;
}

static int usb_get_hub_list(char *pHubInfoList)
{
	int cnt = 0,i = 0, error=0, port_cnt = 0, nportcnt = 0;
	libusb_device **devs;
	//libusb_device_handle *handle;
	libusb_device_descriptor desc;
//	PHINFO pHubListHead = pHubInfoList;	// Pointer to head of the list
	uint8_t port_list[7];

	*pHubInfoList = '\0';

	error = libusb_init(&ctx);

	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}


	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0)
	{
		DEBUGPRINT("Failed to get the device list \n");
		return -1;
	}
	for (i = 0; i < cnt; i++)
	{
		int error = 0;

		libusb_device *device = devs[i];
		error = libusb_get_device_descriptor(device, &desc);

		if(error != 0)
		{
			DEBUGPRINT("LIBUSB_ERROR: Failed to retrieve device descriptor for device[%d] \n", i);
		}

		if((error ==  0) && (desc.bDeviceClass == LIBUSB_CLASS_HUB))
		{
			port_cnt = libusb_get_port_numbers(device, port_list, 7);

			if(port_cnt < 1)
			{
				continue;
			}

			nportcnt++;

			char dbgmsg[50];
			memset(dbgmsg,0,50);

			sprintf(pHubInfoList,"%sVID:PID = %04x:%04x", pHubInfoList, desc.idVendor, desc.idProduct);
			//pirintf("VID:PID = %04x:%04x", desc.idVendor, desc.idProduct);
			for(int j = 0; j < port_cnt; j++)
			{

				sprintf(dbgmsg,"%s:%d",dbgmsg,port_list[j]);

			}
			sprintf(pHubInfoList, "%s, Device Path - %s\n",pHubInfoList, dbgmsg);

		}
	}
	printf("%s\n", pHubInfoList);
	libusb_free_device_list(devs, 1);
	return nportcnt;
}

static int compare_hubs(const void *p1, const void *p2)
{
	PHINFO pHub1, pHub2;

	pHub1 = (PHINFO) p1;
	pHub2 = (PHINFO) p2;


	if((VID_MICROCHIP == pHub1->wVID) && (VID_MICROCHIP == pHub2->wVID))
	{
		return 0; 	//Both Microchip hubs
	}
	else if (VID_MICROCHIP == pHub1->wVID)
	{
		return -1;	//Hub 1 is MCHP
	}
	else if (VID_MICROCHIP == pHub2->wVID)
	{
		return 1;  //Hub 2 is MCHP
	}

	return 0;
}

static int usb_open_HCE_device(uint8_t hub_index)
{
	libusb_device_handle *handle= NULL;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_descriptor desc;

	int dRetval = 0;
	ssize_t devCnt = 0, port_cnt = 0;
	ssize_t i = 0, j = 0, k=0;
	uint8_t port_list[7];

	devCnt = libusb_get_device_list(ctx, &devices);
	if(devCnt < 0)
	{
		DEBUGPRINT("Enumeration failed \n");
		return -1;
	}


	for (i = 0; i < devCnt; i++)
	{
		dev = devices[i];

		dRetval = libusb_get_device_descriptor(dev, &desc);
		if(dRetval < 0)
		{
			DEBUGPRINT("Cannot get the device descriptor \n");
			continue;
		}

        for(k = 0; k < HUB_SKUs; k++)
        {
    		if(PID_HCE_DEVICE[k] == desc.idProduct)
            // if(PID_HCE_DEVICE == desc.idProduct)
    		{
    			dRetval = libusb_open(dev, &handle);
    			if(dRetval < 0)
    			{
    				DEBUGPRINT("HCE Device open failed \n");
    				continue;
    			}

    			port_cnt = libusb_get_port_numbers(dev, port_list, 7);
    			if(port_cnt <= 1)
    			{
    				DEBUGPRINT("Retrieving port numbers failed \n");
    				libusb_close(handle);
    				continue;
    			}

    			//it is comapring against hub port count not HCE
    			if(gasHubInfo[hub_index].port_max != (port_cnt-1))
    			{
    				DEBUGPRINT("Hub port match failed with Hub the Index:%d\n",hub_index);
    				libusb_close(handle);
                                    continue;

    			}

    			//Match with the hub port list
    			for(j = 0; j < gasHubInfo[hub_index].port_max; j++)
    			{
    				if(gasHubInfo[hub_index].port_list[j] != port_list[j])
    				{
    					DEBUGPRINT("Hub port match failed with Hub Index:%d\n",hub_index);
    					dRetval = -1;
    					break;
    				}
    			}

    			if(dRetval == -1)
    			{
    				libusb_close(handle);
    				continue;
    			}

    			printf("HCE Hub index=%d Path- ",hub_index);
    			for(j = 0; j < port_cnt; j++)
    			{
    				printf(":%d", (unsigned int)(port_list[j]));
    			}
    			printf("\n");


    			if(libusb_kernel_driver_active(handle, 0) == 1)
    			{
    				//DEBUGPRINT("Kernel has attached a driver, detaching it \n");
    				if(libusb_detach_kernel_driver(handle, 0) != 0)
    				{
    					DEBUGPRINT("Cannot detach kerenl driver. USB device may not respond \n");
    					libusb_close(handle);
    					break;
    				}
    			}

    			dRetval = libusb_claim_interface(handle, 0);

    			if(dRetval < 0)
    			{
    				DEBUGPRINT("cannot claim intterface \n");
    				dRetval = -1;
    				libusb_close(handle);
    				break;
    			}

    			gasHubInfo[hub_index].dev = devices;
    			gasHubInfo[hub_index].handle = handle;
    			gasHubInfo[hub_index].byHubIndex = hub_index;
    			libusb_free_device_list(devices, 1);
    			return dRetval;
    		}
        }
	}

//	libusb_close(handle);

	libusb_free_device_list(devices, 1);
	return -1;
}

int usb_enable_HCE_device(uint8_t hub_index)
{
	libusb_device_handle *handle;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_descriptor desc;
	uint8_t port_list[7];


	int dRetval = 0;
	ssize_t devCnt = 0, port_cnt;
	ssize_t i = 0, j = 0;

	devCnt = libusb_get_device_list(ctx, &devices);
	if(devCnt <= 0)
	{
		DEBUGPRINT("Enumeration failed \n");
		return -1;
	}

	for (i = 0; i < devCnt; i++)
	{
		dev = devices[i];

		dRetval = libusb_get_device_descriptor(dev, &desc);
		if(dRetval < 0)
		{
			DEBUGPRINT("Cannot get the device descriptor \n");
			libusb_free_device_list(devices, 1);
			return -1;
		}

		if(MICROCHIP_HUB_VID == desc.idVendor)
		{
			dRetval = libusb_open(dev, &handle);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device open failed \n");
				continue;
			}


			port_cnt = libusb_get_port_numbers(dev, port_list, 7);
                        if(port_cnt < 1)
                        {
                                DEBUGPRINT("Retrieving port numbers failed \n");
				libusb_close(handle);
                                continue;
                        }

			//It is comaparing agaist hub port count not HCE
			if(gasHubInfo[hub_index].port_max != (port_cnt))
                        {
                                DEBUGPRINT("Hub port match failed with Hub Index:%d\n",hub_index);
                                libusb_close(handle);
                                continue;

                        }

                        for(j = 0; j < gasHubInfo[hub_index].port_max; j++)
                        {
                                if(gasHubInfo[hub_index].port_list[j] != port_list[j])
                                {
                                        DEBUGPRINT("Hub port match failed \n");
                                        dRetval = -1;
                                        break;
                                }
                        }

                        if(dRetval == -1)
                        {
                        	libusb_close(handle);
				continue;
                        }

			printf("HCE capable hub found at Hub index=%d Path- ", hub_index);

                        for(i = 0; i < port_cnt; i++)
                        {
                                printf(":%d", (unsigned int)(port_list[i]));
                        }
                        printf("\n");


			if(libusb_kernel_driver_active(handle, 0) == 1)
			{

				if(libusb_detach_kernel_driver(handle, 0) != 0)
				{
					DEBUGPRINT("Cannot detach kerenl driver. USB device may not respond \n");
					libusb_close(handle);
					break;
				}
			}

			dRetval = libusb_claim_interface(handle, 0);

			if(dRetval < 0)
			{
				DEBUGPRINT("cannot claim intterface \n");
				libusb_close(handle);
				break;
			}

			uint16_t val = 0x0001;
			dRetval = usb_send_vsm_command(handle,(uint8_t*)&val);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device: VSM command 0x0001 failed \n");
				libusb_close(handle);
				break;
			}

			val = 0x0201;
			dRetval = usb_send_vsm_command(handle,(uint8_t*)&val);
			if(dRetval < 0)
			{
				DEBUGPRINT("HCE Device: VSM command 0x0201 failed \n");
			}
			sleep(2);

			libusb_close(handle);
			libusb_free_device_list(devices, 1);
			return dRetval;
		}
	}

	libusb_free_device_list(devices, 1);
	return -1;
}

int  usb_send_vsm_command(struct libusb_device_handle *handle, uint8_t * byValue)
{
	int rc = 0;

	rc = libusb_control_transfer(	handle,
					0x40,
					0x02,
					0,
					0,
					byValue,
					2,
					CTRL_TIMEOUT
				);
	return rc;
}

int Read_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x01;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Read OTP failed %d\n",bRetVal);
		return bRetVal;
	}
	return bRetVal;
}

int Write_OTP(HANDLE handle, uint16_t wAddress, uint8_t *data, uint16_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x00;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	if(bRetVal< 0)
	{
		DEBUGPRINT("Execute write OTP command failed %d\n",bRetVal);
		return bRetVal;
	}
	return bRetVal;
}

int xdata_read(HANDLE handle, uint16_t wAddress, uint8_t *data, uint8_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest 	= 0x04;
	UsbCtlPkt.wValue 	= wAddress;
	UsbCtlPkt.wIndex 	= 0;
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_read_data (&UsbCtlPkt);
	return bRetVal;
}

int xdata_write(HANDLE handle, uint32_t wAddress, uint8_t *data, uint8_t num_bytes)
{
	int bRetVal = FALSE;
	USB_CTL_PKT UsbCtlPkt;

	UsbCtlPkt.handle 	= (libusb_device_handle*)gasHubInfo[handle].handle;
	UsbCtlPkt.byRequest = 0x03;
	UsbCtlPkt.wValue 	= (wAddress & 0xFFFF);
	UsbCtlPkt.wIndex 	= ((wAddress & 0xFFFF0000) >> 16);
	UsbCtlPkt.byBuffer 	= data;
	UsbCtlPkt.wLength 	= num_bytes;

	bRetVal = usb_HCE_write_data (&UsbCtlPkt);
	return bRetVal;
}

unsigned int CalculateNumberofOnes(unsigned int UINTVar)
{
	unsigned int N0OfOnes = 0;
	do
	{
		if(0x0000 == UINTVar) // variable if zero then return 0
			break;
		// Now counts 1's
		while(UINTVar)
		{
			N0OfOnes++;
			UINTVar &= (UINTVar -1);
		}
	}while(false);

	return N0OfOnes;
}

//Return handle to the first instance of VendorID &amp; ProductID matched device.
//VID and PID of Hub Feature Controller - by dealut vid:pid - 0x424:0x2530
//This API shouldn't use if there is multiple HUBS with HFC(vid:pid - 0x424:0x2530)
HANDLE  MchpUsbOpenHFC (UINT16 wVID, UINT16 wPID)
{
	int error = 0;
	libusb_device_handle *handle;

	error = libusb_init(&ctx);
	if(error < 0)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Initialization LibUSB failed\n");
		return -1;
	}


	handle = libusb_open_device_with_vid_pid(ctx, wVID, wPID);
	if(NULL == handle)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: Failed to open the HFC vid:pid - 0x%04x:0x%04x\n", wVID, wPID);
		return -1;
	}

	/*Check if kenel driver attached*/
	if(libusb_kernel_driver_active(handle, 0))
   	{
		error = libusb_detach_kernel_driver(handle, 0); // detach driver

		if(0 > error)
		{
			DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: libusb_detach_kernel_driver failed\n");
			libusb_close(handle);
			return -1;
		}

   	}
   	error = libusb_claim_interface(handle, 0);
   	if(0 > error)
	{
		DEBUGPRINT("MCHP_Error_LibUSBAPI_Fail: libusb_claim_interface failed\n");
		libusb_close(handle);
		return -1;
	}

	gasHubInfo[0].handle = handle;
	gasHubInfo[0].byHubIndex = 0;

	return 0;
}
