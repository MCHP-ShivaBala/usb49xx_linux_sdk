/*
**********************************************************************************
ADD LICENSE
**********************************************************************************
*   spi_bridging.cpp
*   This file gives the sample code/ test code for using MchpUSB2530 API
*	Interface.
**********************************************************************************
*  $Revision:
*  Description: Sample code for SPI Bridging
**********************************************************************************
* $File:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdbool.h>

#include <cstdlib>

// PT2 SDK API Header file
#include "MchpUSBInterface.h"
#include "USBHubAbstraction.h"

using namespace std;

int main (int argc, char* argv[])
{
	CHAR sztext[3072];
	CHAR chText[512];
	DWORD dwError = 0;
	WORD vendor_id = 0x424 ,product_id= 0x4504;
	BYTE byOperation;
    uint32_t byStartAddr = 0;
	HANDLE hDevice =  INVALID_HANDLE_VALUE;
	UINT8 byLength;

    uint8_t  pbyBuffer[256 * 1024]; //SB
	int32_t wDataLength;
	// BYTE byReadFirmwareData[64 * 1024];
    uint8_t byReadFirmwareData[256 * 1024]; //SB
	string sFirmwareFile;

	uint8_t byBuffer[5] = {0,0,0,0}; //SB
	uint16_t DataLen;
	uint16_t wTotalLen;

    uint8_t byReadBuffer[517];
    uint8_t byWriteBuffer[260] = {0x02};
	char path[20] = {0};
	int hub_count = 0;

    uint16_t NumPageWrites;
    uint8_t RemainderBytes;

	if((0 == strcmp(argv[1],"--help")) || (0 == strcmp(argv[1],"/?")))  //Help
	{
		printf("Operation : Write \n");
		printf("Usage: ./spiBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x01) FirmwareFile \n");
		printf("Example: ./spiBridging 0x424 0x4916 \"1:2\" 0x01 USB49XX_SILICON_CarLife_SPI_V1.27.bin \n \n");
		printf("Operation : Read \n");
		printf("Usage: ./spiBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x00) StartAddress Length \n");
		printf("Example: ./spiBridging 0x424 0x4916 \"1:2\" 0x00 0x00 512 \n\n");
		printf("Operation : Transfer\n");
		printf("Usage: ./spiBridging VID(Hex) PID(Hex) DevicePath(String) Operation(0x03) Command DataLength TotalLength\n");
		printf("Example: ./spiBridging 0x0424 0x4504 8 0x03 0x9f 1 4 \n\n");
		exit(1);

	}
	else if(argc < 6)
	{
		printf("ERROR: Invalid Usage.\n");
		printf("Use --help option for further details \n");
		exit (1);
	}
	else
	{
		vendor_id  =  strtol (argv[1], NULL, 0) ;             //Getting vid and Pid from commandline arguments.
		product_id =  strtol (argv[2], NULL, 0) ;
		strcpy(path,argv[3]);
		byOperation=  strtol (argv[4], NULL, 0) ;

		if(byOperation == 0x00) // Read.
		{
			byStartAddr=  strtol (argv[5], NULL, 0);
			// byLength   =  strtol (argv[6], NULL, 0) ;
            wTotalLen   =  strtol (argv[6], NULL, 0) ;
		}
		else if(byOperation == 0x01) //Write
		{
			sFirmwareFile = argv[5];
			byStartAddr = 0x00;				//SB
		}
		else if(byOperation == 0x03) //Transfer
		{
			//byBuffer = strtol (argv[5],NULL,0);
			byBuffer[0] = strtol (argv[5],NULL,0);	//SB
			DataLen	 = strtol (argv[6],NULL,0);
			wTotalLen = strtol (argv[7],NULL,0);

            byStartAddr = strtol (argv[8], NULL, 0);    //SB
            byBuffer[1] = (byStartAddr & 0xFF0000) >> 16; //SB
            byBuffer[2] = (byStartAddr & 0x00FF00) >> 8; //SB
            byBuffer[3] = byStartAddr & 0x0000FF; //SB
		}
	}

	// Get the version number of the SDK
	if (FALSE == MchpUsbGetVersion(sztext))
	{
		printf ("\nError:SDK Version cannot be obtained,Press any key to exit....");
		exit (1);
	}

	cout << "SDK Version:" <<sztext << endl;
	memset(sztext,0,2048);

	hub_count = MchpGetHubList(sztext);

	if(hub_count < 0)
	{
		printf("No USB hubs found in the system");

	}

	printf("SPI Bridging Demo\n");


	//Return handle to the first instance of VendorID and ProductID matched device.
	hDevice = MchpUsbOpen(vendor_id,product_id,path);

	if(INVALID_HANDLE_VALUE == hDevice)
	{
		printf ("\nError: MchpUsbOpenID Failed:\n");
		exit (1);
	}

	printf ("MchpUsbOpenID successful... \n");

	if(byOperation == 0x00) //Read
	{
		// //Performs read operation from SPI Flash.
		// if(FALSE == MchpUsbSpiFlashRead(hDevice,byStartAddr, &byReadFirmwareData[0],byLength))
		// {
		// 	printf ("\nError: Read Failed:\n");
		// 	exit (1);
		// }
		// for(UINT8 i =0; i< byLength; i++)
		// {
		// 	sprintf(chText,"0x%02x \t",byReadFirmwareData[i] );
		// 	strcat(sztext,chText);
		// }
		// printf("%s \n",sztext);

        //Enable the SPI interface.
        if(FALSE == MchpUsbSpiSetConfig (hDevice,1))
        {
            printf ("\nError: SPI Pass thru enter failed:\n");
            exit (1);
        }

        //performs write operation to the SPI Interface.
        //if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer,DataLen,wTotalLen)) //write
        //{
        //	printf("SPI Transfer write failed \n");
        //	exit (1);
        //}
        byStartAddr = 0;
        wDataLength = 256*1024;
        NumPageWrites = wDataLength / 512;
        RemainderBytes = wDataLength % 512;


        for(uint16_t i=0; i<NumPageWrites; i++)
        {
            byBuffer[0] = 0x0B;
            byBuffer[1] = (byStartAddr & 0xFF0000) >> 16; //SB
            byBuffer[2] = (byStartAddr & 0x00FF00) >> 8; //SB
            byBuffer[3] = byStartAddr & 0x0000FF; //SB

            //performs write operation to the SPI Interface.	//SB
            if(FALSE == MchpUsbSpiTransfer(hDevice,0,byBuffer,4,wTotalLen+5)) //write
            {
                printf("SPI Transfer write failed \n");
                exit (1);
            }

            //performs read operation to the SPI Interface.
            //if(FALSE == MchpUsbSpiTransfer(hDevice,1,(UINT8 *)&byReadBuffer,wTotalLen,wTotalLen))
            //{if(i%16 == 0)
            //		printf("\n");
            //	printf("SPI Transfer read failed \n");
            //	exit (1);
            //}

            //performs read operation to the SPI Interface.	//SB
            if(FALSE == MchpUsbSpiTransfer(hDevice,1,(UINT8 *)&byReadBuffer,DataLen,wTotalLen+5))
            {
                printf("SPI Transfer read failed \n");
                exit (1);
            }

            //if(FALSE == SandiaBlockSpiRead(hDevice, (UINT8*)byBuffer, byReadBuffer, DataLen, wTotalLen))
            //{
            //	printf("SPI Block read failed \n");
            //	exit (1);
            //}

            /*printf("String length sztext: %d\n", strlen(sztext));
            for(UINT8 i =1; i< wTotalLen+1; i++)
            {
            sprintf(chText,"0x%02x",byReadBuffer[i] );
            strcat(sztext,chText);
            printf("String length chText: %d sztext: %d\n", strlen(chText),strlen(sztext));
            //if (strlen(sztext) == 2060)
            //	break;
        }*/

            //printf("%s",sztext);

            /**/
            memcpy((void *)&byReadFirmwareData[i*512], (const void *)&byReadBuffer[1], 512);

            //Check if the flash is BUSY
            byBuffer[0] = 0x05;
            do
            {
                //performs write operation to the SPI Interface.	//SB
                if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,2)) //write
                {
                    printf("SPI Transfer write failed \n");
                    exit (1);
                }

                if(FALSE == MchpUsbSpiTransfer(hDevice,1,(UINT8 *)&byReadBuffer,DataLen,1))
                {
                    printf("SPI Transfer read failed \n");
                    exit (1);
                }
                printf("Reading page %d at addr 0x%06x...SR = %02x\n",i,byStartAddr,byReadBuffer[0]);

            }while(byReadBuffer[0] != 0x00);

            byStartAddr += 512;
        }

        //Disable the SPI interface.
        if(FALSE == MchpUsbSpiSetConfig (hDevice,0))
        {
            printf ("Error: SPI Pass thru enter failed:\n");
            exit (1);
        }

        // for(UINT16 i =1; i<wTotalLen+1; i++)
        // {
        //     printf("0x%02x  ",byReadBuffer[i]);
        //     if(i%16 == 0)
        //     printf("\n");
        // }
        // printf("\n");

        for(int32_t i =0; i<wDataLength; i++)
        {
            printf("0x%02x  ",byReadFirmwareData[i]);
            if(i%16 == 0)
            printf("\n");
        }
        printf("\n");

        // if(writeBinfile(char const *name, uint8_t *buffer, unsigned long fileLen) < 0)
        // {
        //     printf("Failed to create binary file\n");
        // }

	}
	// else if(byOperation == 0x01)//Write
	// {
	// 	//Read the content of the file.
	// 	wDataLength = ReadBinfile(sFirmwareFile.c_str(),pbyBuffer);
	// 	if(wDataLength <=0)
	// 	{
	// 		printf("Failed to Read Content of File\n");
	// 		exit (1);
	// 	}
	// 	//Performs write opeartion to SPI Flash memory.
	// 	if(FALSE == MchpUsbSpiFlashWrite(hDevice,byStartAddr, &pbyBuffer[0],0xfffe))
	// 	{
	// 		printf ("\nError: Write Failed:\n");
	// 		exit (1);
	// 	}
	// }
	else //Transfer
	{
        //Read the bin file into local Buffer
        wDataLength = ReadBinfile(sFirmwareFile.c_str(),pbyBuffer);
        if(wDataLength <=0)
        {
            printf("Failed to Read Content of File\n");
            exit (1);
        }

        //Enable the SPI interface.
        if(FALSE == MchpUsbSpiSetConfig (hDevice,1))
        {
            printf ("\nError: SPI Pass thru enter failed:\n");
            exit (1);
        }

        byBuffer[0] = 0x98;
        if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,1)) //write
        {
            printf("SPI Transfer write failed \n");
            exit (1);
        }

        //WREN
        byBuffer[0] = 0x06;
        //performs write operation to the SPI Interface.	//SB
        if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,1)) //write
        {
            printf("SPI Transfer write failed \n");
            exit (1);
        }

        //Chip Erase
        byBuffer[0] = 0xC7;
        // if(FALSE == MchpUsbSpiTransfer(hDevice,0,byBuffer,4,4)) //write
        if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,1)) //write
        {
            printf("SPI Transfer write failed \n");
            exit (1);
        }

        //Busy wait on the erase operation
        byBuffer[0] = 0x05;
        do
        {
            //performs write operation to the SPI Interface.	//SB
            if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,2)) //write
            {
                printf("SPI Transfer write failed \n");
                exit (1);
            }
            if(FALSE == MchpUsbSpiTransfer(hDevice,1,(UINT8 *)&byReadBuffer,DataLen,1))
            {
                printf("SPI Transfer read failed \n");
                exit (1);
            }
            printf("Erasing the Block...\n");

        }while(byReadBuffer[0] == 0x83);

        NumPageWrites = wDataLength / 256;
        RemainderBytes = wDataLength % 256;


        for (uint16_t i=0; i<=NumPageWrites; i++)
        {

            byWriteBuffer[1] = (byStartAddr & 0xFF0000) >> 16; //SB
            byWriteBuffer[2] = (byStartAddr & 0x00FF00) >> 8; //SB
            byWriteBuffer[3] = byStartAddr & 0x0000FF; //SB

            memcpy((void *)&byWriteBuffer[4], (const void *)&pbyBuffer[i*256], 256);

            //WREN
            byBuffer[0] = 0x06;
            //performs write operation to the SPI Interface.	//SB
            if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,1)) //write
            {
                printf("SPI Transfer write failed \n");
                exit (1);
            }

            if(i == NumPageWrites)
            {
                /*Copying the remaining binary data into write buffer when data lenth < 256 bytes*/
                memcpy((void *)&byWriteBuffer[4], (const void *)&pbyBuffer[i*256], RemainderBytes);

                /*Writing a remaining bytes in the last page*/
                if(FALSE == MchpUsbSpiTransfer(hDevice,0,byWriteBuffer,RemainderBytes+4,RemainderBytes+4)) //write
                {
                    printf("SPI Transfer write failed \n");
                    exit (1);
                }

            }
            else
            {
                /*Copying a page length of binary data into write buffer*/
                memcpy((void *)&byWriteBuffer[4], (const void *)&pbyBuffer[i*256], 256);

                /*Writing a 256 byte page*/
                if(FALSE == MchpUsbSpiTransfer(hDevice,0,byWriteBuffer,256+4,256+4)) //write
                {
                    printf("SPI Transfer write failed \n");
                    exit (1);
                }
            }

            //Check if the flash is BUSY
            byBuffer[0] = 0x05;
            do
            {

                //performs write operation to the SPI Interface.	//SB
                if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,2)) //write
                {
                    printf("SPI Transfer write failed \n");
                    exit (1);
                }

                if(FALSE == MchpUsbSpiTransfer(hDevice,1,(UINT8 *)&byReadBuffer,DataLen,1))
                {
                    printf("SPI Transfer read failed \n");
                    exit (1);
                }
                printf("Writing page %d at addr 0x%06x...SR = %02x\n",i,byStartAddr,byReadBuffer[0]);

            }while(byReadBuffer[0] == 0x83);

            byStartAddr += 256;    //SB
        }

        //WRDI
        byBuffer[0] = 0x04;
        //performs write operation to the SPI Interface.	//SB
        if(FALSE == MchpUsbSpiTransfer(hDevice,0,&byBuffer[0],1,1)) //write
        {
            printf("SPI Transfer write failed \n");
            exit (1);
        }

        //Disable the SPI interface.
        if(FALSE == MchpUsbSpiSetConfig (hDevice,0))
        {
            printf ("Error: SPI Pass thru enter failed:\n");
            exit (1);
        }

	}
	//close device handle
	if(FALSE == MchpUsbClose(hDevice))
	{
		dwError = MchpUsbGetLastErr(hDevice);
		printf ("\nMchpUsbClose:Error Code,%04x\n",(unsigned int)dwError);
		exit (1);
	}
	return 0;
}
