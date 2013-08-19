//------------------------------------------------------------------------------
// Module Name: UCfgRX
//
// Module Description:  Intit Users EEPROM, intit RX with API setting written into EEPROM
//
//
// Copyright � 2002-2005, Silicon Image, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Silicon Image, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------
#include <stdio.h>

#include "UCfgRX.h"
#include "UEEPROM.h"
#include "SiIRX_API.h"
#include "SiIRXAPIDefs.h"

#include "SiIVidIn.h"

//------------------------------------------------------------------------------
// Function Name: siiGetPCB_Id()
// Function Description:  this function reads Board Id from EEPROM
//
// Accepts: none
// Returns: BYTE, Board Id
// Globals: none
//------------------------------------------------------------------------------
BYTE siiGetPCB_Id( void ){
BYTE bData;

    siiBlockReadEEPROM ( SII_PCB_ID_ADDR, 1 , &bData );
	return bData;
}
//------------------------------------------------------------------------------
// Function Name: InitEEPROMWithDefaults
// Function Description:
//------------------------------------------------------------------------------
void InitEEPROMWithDefaults( void ){
BYTE abData[5];

    abData[0] = RX_API_ID_L;
    abData[1] = RX_API_ID_H;
    siiBlockWriteEEPROM ( SII_EEPROM_ID_ADDR , 2, abData);    // write EEPROM ID
    abData[0] = SiI_CP9125;
//    abData[0] = SiI_CP9135;

    siiBlockWriteEEPROM ( SII_PCB_ID_ADDR , 1, abData);    // write PCB ID

//YMA				HBR					DSD				PCM
    abData[0] = MClock_256Fs<<4 | MClock_512Fs<<2 | MClock_256Fs | SiI_RX_InvertOutputVidClock;
    abData[1] = SiI_RX_CD_24BPP;
    siiBlockWriteEEPROM ( SII_RX_INIT_SYS_ADDR ,2 , abData);

    abData[0] = SiI_RX_VInCh1;
    siiBlockWriteEEPROM ( SII_RX_VIDEO_INPUT ,1 , abData);

    abData[0] = SiI_RX_P_RGB;
    abData[1] = SiI_RX_SS_SeparateSync;
    abData[2] = SiI_RX_SC_NoInv;
    abData[3] = SiI_RX_AVC_NoPedestal;
    siiBlockWriteEEPROM ( SII_RX_VIDEO_OUTPUT_F ,4 , abData);

    abData[0] = (BYTE)( SiI_RX_AOut_Default & 0xFF );
    abData[1] = (BYTE)( ( SiI_RX_AOut_Default >> 8 ) & 0xFF );
    abData[2] = (BYTE)( SiI_RX_AOut_I2S_I2SDefault & 0xFF );
    abData[3] = (BYTE)(( SiI_RX_AOut_I2S_I2SDefault >> 8 ) & 0xFF );
    abData[4] = 0x00;
	abData[5] = 0x22;//YMA 2 added default value for DSDHBR format
    siiBlockWriteEEPROM ( SII_RX_AUDIO_OUTPUT_F, 6 , abData); //YMA 2 change 5 -> 6

}

//------------------------------------------------------------------------------
// Function Name: InitTX
// Function Description:  this functio takes TX out of Power Down mode
//------------------------------------------------------------------------------
static void InitTX ( BYTE bBoardID ){

   
    
    if ( bBoardID == SiI_CP9000 ) {
        hlWriteByte_8BA( 0x72, 0x08, 0x35 );
    }
    else if (( bBoardID == SiI_FPGA_IP11 )||( bBoardID == SiI_CP9135|| bBoardID ==  SiI_CP9125)||( bBoardID == SiI_CP9133)) {

	 printf("InitTX\n");
        hlWriteByte_8BA(0x70, 0x08, 0x37);
        hlWriteByte_8BA(0x70, 0x09, 0x11);
    }
    
}
//------------------------------------------------------------------------------
// Function Name: PrintMasterClock
// Function Description:  This function prints output master clock
//------------------------------------------------------------------------------
static void PrintMasterClock( BYTE bMasterClock ){

    switch ( bMasterClock ){
        case MClock_128Fs: printf ("128 * Fs"); break;
        case MClock_256Fs: printf ("256 * Fs"); break;
        case MClock_384Fs: printf ("384 * Fs"); break;
        case MClock_512Fs: printf ("512 * Fs"); break;
    }
}
//------------------------------------------------------------------------------
// Function Name: PrintOutputClorDepth
// Function Description:  This function prints output color depth
//------------------------------------------------------------------------------
static void PrintOutputColorDepth( BYTE bOutColorDepth ){

    switch ( bOutColorDepth ){
        case SiI_RX_CD_24BPP: printf ("24 bit per pixel"); break;
        case SiI_RX_CD_30BPP: printf ("30 bit per pixel"); break;
        case SiI_RX_CD_36BPP: printf ("36 bit per pixel"); break;
    }
}
//------------------------------------------------------------------------------
// Function Name: PrintIntilizeSystemWith
// Function Description:  This function prints out parameters of intilization RX API instance
//------------------------------------------------------------------------------
static void PrintIntilizeSystemWith( BYTE * pbInitData ){

    printf ("\n RX API intance intilized with:");
    printf ("\nPCM Master Clock output: " );
    PrintMasterClock( pbInitData[0] & 0x03 );
    printf ("\nDSD Master Clock output: " );
    PrintMasterClock( (pbInitData[0] >> 2) & 0x03 );
    printf ("\nHBR Master Clock output: " );
    PrintMasterClock( (pbInitData[0] >> 4) & 0x03 );
    printf ("\nFPGA support: ");
    if ( pbInitData[0] & SiI_RX_FPGA )
        printf ("yes");
    else
        printf ("no");
    printf ("\nVideo Output clock:");
    if ( pbInitData[0] & SiI_RX_InvertOutputVidClock )
        printf ("yes");
    else
        printf ("no");
    printf ("\nOutput Color Depth: ");
    PrintOutputColorDepth ( pbInitData[1] & 0x03 );
}

//------------------------------------------------------------------------------
// Function Name: siiRXAPIConfig
// Function Description:
//
//------------------------------------------------------------------------------
static BYTE siiRXAPIConfig( void ){
BYTE abData[6];
BYTE bError;
WORD wDevId;

    siiBlockReadEEPROM ( SII_RX_INIT_SYS_ADDR ,2 , abData);

    bError = SiI_RX_InitializeSystem( abData );

    if ( ! ( bError & SiI_EC_Mask) ) {
//VG added for printout system config
		PrintIntilizeSystemWith( abData );

        SiI_RX_GetAPI_Info( abData );
        wDevId = abData[4] | ( abData[5] << 8 );

	siiBlockReadEEPROM ( SII_RX_VIDEO_INPUT ,1 , abData);

        if ( wDevId == SiI9011 ) {
            abData[0] = SiI_RX_VInCh1;
        }

//YMA added to avoid reinit system again in the SetVideoInput() at start up.
// not need to reinitialize the system as the changing channel should do
//	bError = SiI_RX_SetVideoInput( abData[0] );
SiI_Ctrl.bVidInChannel = abData[0]; 
bError = siiInitVideoInput(SiI_Ctrl.bVidInChannel);
//YMA end of modify

        if ( !(bError & SiI_EC_Mask) ) {
            siiBlockReadEEPROM ( SII_RX_VIDEO_OUTPUT_F ,4 , abData);
            if ( ( wDevId == SiI9011 ) || ( wDevId == SiI9023 ) || ( wDevId == SiI9033 ) || 
                   (SiI_Ctrl.bDevId == RX_SiI9133 )|| (SiI_Ctrl.bDevId == RX_SiI9135||SiI_Ctrl.bDevId == RX_SiI9125 )
				 || ( SiI_Ctrl.bDevId == RX_SiIIP11 ) ) {
                abData[3] = SiI_RX_AVC_Digital_Output; // set video digital output because no analog                                                           // video output suuport
            }
            bError = SiI_RX_SetVideoOutputFormat ( abData[0],
                                                   abData[1],
                                                   abData[2],
                                                   abData[3]
                                                 );
            if (  !(bError & SiI_EC_Mask) ) {
//                siiBlockReadEEPROM ( SII_RX_AUDIO_OUTPUT_F ,5 , abData);   //0x30
                siiBlockReadEEPROM ( SII_RX_AUDIO_OUTPUT_F ,6 , abData);   //YMA 2 added DSDHBR format byte
                bError = SiI_RX_SetAudioOutputFormat ( (WORD) ( abData[0] | (abData[1] << 8)),
                                                       (WORD) ( abData[2] | (abData[3] << 8)),
                                                                abData[4], abData[5]//YMA 2 added DSDHBR format byte
                                                     );

            }
        }
    }
    return ( bError  & SiI_EC_Mask) ;

}
//------------------------------------------------------------------------------
// Function Name: siiResoreSavedRXConfiguration
// Function Description:
//
// Accepts: none
// Returns: none
// Globals: none
//------------------------------------------------------------------------------
BYTE siiRXConfig( void ){
BYTE abData[2];
BYTE bError;


    printf ( " \n Init application...\n" );

    bError = siiFindEEPROM();
    if ( !bError ) {

        bError = siiBlockReadEEPROM ( SII_EEPROM_ID_ADDR, 2 , abData );
        if( !bError ) {
            printf ( "\n Get EEPROM settings\n" );
            if( ( abData[0] != RX_API_ID_L ) || ( abData[1] != RX_API_ID_H )){
                InitEEPROMWithDefaults();
            }
            bError = siiRXAPIConfig();

            if( !bError  ) {
                siiBlockReadEEPROM ( SII_PCB_ID_ADDR , 1, abData);    // write PCB ID
                if ( ( abData[0] == SiI_CP9000 ) || ( abData[0] == SiI_FPGA_IP11 ) ) 
                    InitTX( abData[0] );
            }
        }

    }
    else {
        bError = TRUE;
        printf ( "No Ack from configuration EEPROM, use API to init RX\n");

    }
    return bError;

}
