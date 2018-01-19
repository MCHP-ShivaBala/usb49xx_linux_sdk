/*******************************************************************************
  Protouch2 SDK header file

  Company:
    Microchip Technology Inc.

  File Name:
    MchpUSBInterface.h

  Summary:
    Protouch2 SDK Header File

  Description:
    Protouch2 SDK LIB Header file
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
*/
#pragma once

#include "typedef.h"

// *****************************************************************************
// *****************************************************************************
// Section: DLL Exports
// *****************************************************************************
// *****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif


#define MY_MAX_PATH   						1024*8
#define MAX_HUBS 						20
#define CTRL_TIMEOUT 						(5*1000) /* milliseconds */

#define DEBUG
#ifdef DEBUG
#define DEBUGPRINT(...) printf("DEBUGINFO: " __VA_ARGS__)
#else
#define DEBUGPRINT(...)
#endif

#pragma pack(1)
typedef struct tagControlPacket
{
	libusb_device_handle*	handle;
	uint8_t				byRequest;
	uint16_t				wValue;
	uint16_t				wIndex;
	uint8_t*				byBuffer;
	uint16_t				wLength;
} USB_CTL_PKT, *PUSB_CTL_PKT;

typedef enum tag_family
{
	HUB_FAMILY_INDEX_USB2530 = 0,
	HUB_FAMILY_INDEX_USB274X = 1,
	HUB_FAMILY_INDEX_EC2 = 2,
	HUB_FAMILY_INDEX_UNKNOWN = 3
} hub_family_index;

typedef	struct _ASIC
{
	WORD	wInternalFWRevision;
	BYTE	byDeviceRevision;
	WORD	wASICType;
	BYTE	byFeaturesFlag;

} ASICINFO;

typedef enum {SMBus, USB} IntfMode;

typedef enum
{
    USB_DEVICE_INSTANCE_NOT_INITIALIZED = 0,
    USB_DEVICE_INSTANCE_INITIALIZED,
    USB_DEVICE_INSTANCE_OPENED,
    USB_DEVICE_INSTANCE_CLOSED

}USB_DEVICE_INSTANCE_STATE;

typedef struct tagHubInfo
{
	WORD	wVID;			/*!< USB Vendor ID of the hub*/
	WORD	wPID;			/*!< USB Product ID of the hub*/
	WORD	wDID;			/*!< USB device ID of the hub*/
	BYTE	byPorts;		/*!< Number of downstream ports in the hub*/
	WORD wSKUNAME;		/*!< Get SKU name to find number of downstream port*/
	IntfMode Interface;

	BYTE	port_list[7];		/*!< Port number list from root*/
	BYTE	port_max;		/*!< Number of items in the path*/
	void*	hThisHub;	/*!< Device Handle to this hub on Windows*/
	void*	hWinUsbHandle;//usb device handle- hubhandle->usbdevice handle->winusb handle
	void*	hParentHub;	/*!< Device Handle to the parent hub on Windows*/
	int iLocation;
	char deviceName[MY_MAX_PATH];

	ASICINFO sHubInfo;

	////libusb
	void* handle;
	void* filehandle;/*Winusb Dev path handle */
	void **devs;
	void *dev;
	BYTE byHubIndex;
	bool gbUpdateSPI_OTP;
	BYTE gbyRunningfromSPI;
	hub_family_index enHubFamily;/* Family of the Hub, find from sku*/
	USB_DEVICE_INSTANCE_STATE Usb_Device_State;
	DWORD dwError; /* Error occured in last operation*/

} HINFO, DEVICE_INFO, *PHINFO, *PDEVICE_INFO;


// Global variable for tracking the list of hubs
extern HINFO gasHubInfo [MAX_HUBS];

// *****************************************************************************
#define INVALID_HANDLE_VALUE 0xFF
#define Error_Success        	0x0000
#define ReadBackFailed		0x1001
#define SpiPassThruWriteFailed  0x1003
#define SpiPassThruEnterFailed  0x1004
#define SpiNoDevice	        0x1005
#define SpiFlashWrongDeviceID   0x100A
#define SpiFWCompareFailed	0x100B
#define SPISRAMProgFailed 	0x100E


/**************************************************************************************************
  Function:
        BOOL  MchpUsbGetVersion ( PCHAR pchVersionNo );

  Summary:
    Get version no of the DLL.
  Description:
    This API will get the version no of the DLL
  Conditions:
    None.
  Input:
    pchVersionNo -  Pointer to the buffer where the version number of the
                    DLL will be stored.
  Return:
    None.
  Example:
    <code>
    CHAR sztext[2048];
    if (FALSE == MchpUsbGetVersion(sztext))
    {
        printf ("nPress any key to exit....");
        exit (1);
    }
    //Print version number here
    cout \<\< sztext \<\< endl;
    </code>
  Remarks:
    None
  **************************************************************************************************/
 BOOL  MchpUsbGetVersion ( PCHAR pchVersionNo );
/**************************************************************************************************
  Function:
            UINT32 MchpUsbGetLastErr (HANDLE DevID);

  Summary:
    Get last error for the specific hub instance.
  Description:
    This API will get last error occurred when handling other API's in this
    library.
  Conditions:
    None.
  Input:
    DevID -  Handle to the device \- Return value of
             MchpUsbOpenID.
  Return:
    Linux Error codes.
  Example:
    <code>

	dwError = MchpUsbGetLastErr(hDevice);

    //Print error here
    cout \<\< dwError \<\< endl;
    </code>
  Remarks:
    None
  **************************************************************************************************/

 UINT32 MchpUsbGetLastErr (HANDLE DevID);

/*********************************************************************************************************
 Function:
            int MchpGetHubList(PCHAR pchHubcount );

  Summary:
    Get list of the usb devices connected to the system.
  Description:
    This API will get Get list of the usb devices connected to the system.
  Conditions:
    None.
  Input:
    HubInfo -  Pointer to the buffer which has minimal information about usb
               hubs with hub_index.
  Return:
     No.of usb Hubs connected to system
  Example:
    <code>

	hub_count = MchpGetHubList(sztext);

    //Print error here
    cout \<\< hub_count \<\< endl;
    </code>
  Remarks:
    None
  **************************************************************************************************/

int MchpGetHubList(PCHAR HubInfo );

/*********************************************************************************************************

  Function:
    HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID);

 Summary:
    Open the device handle.

  Description:
    This API will return handle to the first instance of the HUB VID & PID matched device.

  Conditions:
    - None.

  Input:
    wVID -    Vendor ID(VID) of the Hub.
	wPID -    Product ID(PID) of the Hub.
  Return:
    HANDLE of the Vendor ID and Product ID matched hub - for success

    INVALID_HANDLE_VALUE (Call GetMchpUsbLastErr for more details) - for
    failure

  Example:
    <code>


    CHAR sztext[2048];

    HANDLE hDevice = INVALID_HANDLE_VALUE;

    UINT32 dwError;


    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");
    </code>
  Remarks:
    None
  *********************************************************************************************************/

 HANDLE  MchpUsbOpenID ( UINT16 wVID, UINT16 wPID);

/******************************************************************************************************
  Function:

    HANDLE  MchpUsbOpen ( UINT16 wVID, UINT16 wPID,char* cDevicePath);

 Summary:
    Open the device handle.


  Description:
    This API will return handle to the first instance of the HUB VID , PID & Device path matched device.

  Conditions:

    - None.

  Input:
    		wVID 	-    Vendor ID(VID) of the Hub.
			wPID 	-    Product ID(PID) of the Hub.
	 cDevicePath    -    Path of the Hub.

  Return:
    HANDLE of the Vendor ID , Product ID and DevicePath matched hub - for success

    INVALID_HANDLE_VALUE (Call GetMchpUsbLastErr for more details) - for
    failure


  Example:
    <code>



    CHAR sztext[2048];

    HANDLE hDevice = INVALID_HANDLE_VALUE;

    UINT32 dwError;

	char path[20] = {7,2};


    hDevice = MchpUsbOpenID(0x424, 0x1234,path);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");
    </code>
  Remarks:
    None
  *********************************************************************************************************/

 HANDLE  MchpUsbOpen ( UINT16 wVID, UINT16 wPID,char* cDevicePath);

/******************************************************************************************************
  Function:

    HANDLE  MchpUsbOpenHFC ( UINT16 wVID, UINT16 wPID);

 Summary:
    Open the device handle.


  Description:
    This API will return handle to the HUB with wVID ,wPID matched device.

  Conditions:

    - This API shouldn't use if there is multiple HUBS with HFC(vid:pid - 0x424:0x2530)enabled.

  Input:
    		wVID 	-    Vendor ID(VID) of the Hub.
			wPID 	-    Product ID(PID) of the Hub.


  Return:
    HANDLE of the Vendor ID , Product ID and DevicePath matched hub - for success

    INVALID_HANDLE_VALUE (Call GetMchpUsbLastErr for more details) - for
    failure


  Example:
    <code>

    HANDLE hDevice = INVALID_HANDLE_VALUE;
    UINT32 dwError;

    hDevice = MchpUsbOpenHFC(0x424, 0x2530);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }

    printf("Device Opened successfullyn");
    </code>
  Remarks:
    None
  *********************************************************************************************************/

 HANDLE  MchpUsbOpenHFC ( UINT16 wVID, UINT16 wPID);

/******************************************************************************************************
  Function:
            BOOL MchpUsbClose(HANDLE DevID);

  Summary:
    Close the device handle.
  Description:
    This API will close the handle for device specified in the call.
  Input:
    DevID -  Handle to the device \- Return value of
             MchpUsbOpenID.
  Conditions:
    MchpUsbOpenID should be called before calling this API
  Return:
    TRUE - for Success;

    FALSE - for Failure
  Example:
    <code>

    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfully\n");

    if (FALSE == MchpUsbClose(hDevice))
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04x",dwError);
        exit (1);
    }
    </code>
  Remarks:
    None
  ******************************************************************************************************/

 BOOL MchpUsbClose(HANDLE DevID);

 /***********************************************************************************************************
  Function:
    BOOL MchpUsbSpiSetConfig ( HANDLE DevID, INT EnterExit)

  Summary:
    This API enables/disables the SPI interface.

  Description:
    This API enables/disables the SPI interface. If SPI control register is
    not edited by the user then this function would put SPI in default mode
    i.e, mode0 and dual_out_en = 0. Speed is dependant totally on the strap
    options.

    A INT variable EnterExit is used to identify if it is pass thru enter
    or exit.

  Conditions:
    MchpUsbOpenID should be called before calling this API

   Input:
    DevID -      Handle to the device<p />
    EnterExit -  Pass thru Enter or exit option<p />1 \: Pass thru Enter;<p />0
                 \: Pass thru Exit;
  Return:
    TRUE - for Success;

    FALSE - (Call GetMchpUsbLastErr for more details) - for failure

  Example:
    <code>
    CHAR sztext[2048];


    HANDLE hDevice =  INVALID_HANDLE_VALUE;

    UINT32 dwError;


    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");

	    //Enter into SPI Pass thru
    if (FALSE == MchpUsbSpiSetConfig(hDevice,1))
    {
		dwError = MchpUsbGetLastErr(hDevice);
		printf("MchpUsbSpiSetConfig Failed- %04x\n",(unsigned int)dwError);
		exit (1);
    }
	</code>
 ***************************************************************************************************************************/
	BOOL MchpUsbSpiSetConfig ( HANDLE DevID, INT EnterExit);

/***************************************************************************************************************************
  Function:
	BOOL MchpUsbSpiFlashRead(HANDLE DevID,UINT32 StartAddr,UINT8* InputData,UINT32 BytesToRead)

  Summary:
    This API performs read operation from SPI Flash.

  Description:
    This API reads bytes of data mentioned in the BytesToRead parameter
    from the SPI Flash memory region of the device starting at address
    mentioned in the StartAddr parameter. Before reading from SPI Flash,it
    will check for correct device Handle and Proper buffer length.

  Conditions:
    MchpUsbOpenID should be called before calling this API

  Input:
    DevID -        Handle to the device<p />
    StartAddr -    Start Address of the SPI Flash from where read operation
                   starts.
    InputData -    Pointer to the Buffer which contains the data to be read.
    BytesToRead -  No of Bytes to be read.
  Return:
    TRUE - for Success;

    FALSE - (Call GetMchpUsbLastErr for more details) - for failure

  Example:
    <code>
	CHAR sztext[2048];


    HANDLE hDevice =  INVALID_HANDLE_VALUE;

    UINT32 dwError;


    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");
	BYTE byReadFirmwareData[64 * 1024];
	if(FALSE == MchpUsbSpiFlashRead(hDevice,0x0000, &byReadFirmwareData[0],0x0064))
	{
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("\nError: Read Failed %04x\n",dwError);
		exit (1);
	}
	</code>
 ***************************************************************************************************************************/
	BOOL MchpUsbSpiFlashRead(HANDLE DevID,UINT32 StartAddr,UINT8* InputData,UINT32 BytesToRead);
/***************************************************************************************************************************
  Function:
    BOOL MchpUsbSpiFlashWrite(HANDLE DevID,UINT32 StartAddr,UINT8* OutputData, UINT32 BytesToWrite)

  Summary:
    This API performs write opeartion to SPI Flash memory.

  Description:
    This API writes bytes of data as mentioned in the BytesToWrite
    parameter to the SPI Flash memory region from memory location as
    specified in StartAddr. Before Writing to SPI Flash,it will check for
    correct device Handle and Proper buffer length.

  Conditions:
    MchpUsbOpenID should be called before calling this API

  Input:
    DevID -         Handle to the device<p />
    StartAddr -     Start Address of the SPI Flash from where write operation
                    starts.
    OutputData -    Pointer to the Buffer which contains the data to be
                    written. Cannot be a constant
    BytesToWrite -  No of Bytes to be written.
  Return:
    TRUE - for Success;

    FALSE - (Call GetMchpUsbLastErr for more details) - for failure

  Example
    <code>
	CHAR sztext[2048];
	uint8_t  pbyBuffer[128 * 1024];

    HANDLE hDevice =  INVALID_HANDLE_VALUE;

    UINT32 dwError;

    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");

	ReadBinfile("spi_firmware.bin",pbyBuffer);
	if(FALSE == MchpUsbSpiFlashWrite(hDevice,0x00, &pbyBuffer[0],0xfffe))
	{
		printf ("\nError: Write Failed:\n");
		exit (1);
	}

	</code>
 ***************************************************************************************************************************/
	BOOL MchpUsbSpiFlashWrite(HANDLE DevID,UINT32 StartAddr,UINT8* OutputData, UINT32 BytesToWrite);
/**********************************************************************************************************************
  Function:
    BOOL MchpUsbSpiTransfer(HANDLE DevID,INT Direction,UINT8* Buffer, UINT16 DataLength,UINT32 TotalLength);

  Summary:
    This API performs read/write operation to the SPI Interface.

  Description:
    This API is the low level SPI pass thru command read/write. All
    commands to the SPI interface are directed as SPI Pass thru write, SPI
    pass thru read is nothing but a XDATA read from a specified offset
    where the response is stored.

  Conditions:
    MchpUsbOpenID should be called before calling this API

  Input:
    DevID -         Handle to the device<p />
    Direction -     This bit will indicate if it is a Pass thru read or
                    write. Read = 1; Write = 0.
    Buffer -        Buffer containing the command/ data to be sent to the
                    device in case of SPI pass thru write. In case of pass
                    thru read this buffer is used to store the data recieved
                    from the device.<p />
    DataLength -    This field is the size of USB command OUT packet being
                    sent to the firmware.<p />
    wTotalLength -  The wTotalLength is utilized to mention the number of
                    bytes the SPI flash will return for the pass thru
                    command.
  Return:
    TRUE - for Success;

    FALSE - (Call GetMchpUsbLastErr for more details) - for failure

  Example
    <code>
	CHAR sztext[2048];
	uint8_t  pbyBuffer[128 * 1024];

    HANDLE hDevice =  INVALID_HANDLE_VALUE;

    UINT32 dwError;

    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");

	if (FALSE == MchpUsbSpiSetConfig(hDevice,1))
	{
		printf ("MchpUsbSpiSetConfig failed");
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("Error,%04xn",dwError);
		exit (1);
	}
	UINT8 bySPIBuffer[4];
	UINT8 byOpcodeGetJEDECID = 0x9f;
	//Write 0x9f to get JEDEC ID, Datalen is 1
	//Totally 4 bytes will be retrived as jedec id, give total length as 4
	if(FALSE == MchpUsbSpiTransfer(hDevice,0,byOpcodeGetJEDECID,1,4))
	{
		printf ("MchpUsbSpiTransfer failed");
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("Error,%04xn",dwError);
		exit (1);
	}
	//Read 4 bytes of JEDEC ID
  	 if(FALSE == libMchpUsbSpiTransfer(hDevice,1,bySPIBuffer[0],4,4))
	{
		printf ("MchpUsbSpiTransfer failed");
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("Error,%04xn",dwError);
		exit (1);
	}
	if (FALSE == MchpUsbSpiSetConfig(hDevice,0))
	{
		printf ("MchpUsbSpiSetConfig failed");
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("Error,%04xn",dwError);
		exit (1);
	}
    </code>

 ***************************************************************************************************************************/
	BOOL MchpUsbSpiTransfer(HANDLE DevID,INT Direction,UINT8* Buffer, UINT16 DataLength,UINT32 TotalLength);

/******************************************************************************************************
  Function:
    BOOL MchpProgramFile( HANDLE DevID, PCHAR InputFileName);

  Summary:
    Program configuration file to the selected device ID

  Description:
    This API will program the configuration file given as argument to the
    selected device ID.

  Conditions:
    MchpUsbOpenID should be called before calling this API

  Input:
    DevID -          Handle to the device
    InputFileName -  \Input configuration file to be programmed into the
                     device

  Example
    <code>
	CHAR sztext[2048];

	uint8_t  pbyBuffer[128 * 1024];


    HANDLE hDevice =  INVALID_HANDLE_VALUE;

    UINT32 dwError;


    hDevice = MchpUsbOpenID(0x424, 0x1234);
    if(INVALID_HANDLE_VALUE == hDevice)
    {
        dwError = MchpUsbGetLastErr(hDevice);
        printf ("Error,%04xn",dwError);
        exit (1);
    }
    printf("Device Opened successfullyn");

    if(FALSE == MchpProgramFile(hDevice ,"MYcONFIG.BIN"))
    {
	printf("Programming Failed \n");
	dwError = MchpUsbGetLastErr(hDevice);
	printf ("Error,%04xn",dwError);
	exit (1);
    }
   </code>

 ***************************************************************************************************************************/
// BOOL MchpProgramFile( HANDLE DevID, PCHAR InputFileName);


BOOL SandiaBlockSpiRead(HANDLE DevID,UINT8* CmdBuffer, UINT8* DataBuffer, UINT16 DataLength,UINT32 TotalLength);


#ifdef __cplusplus
}
#endif
