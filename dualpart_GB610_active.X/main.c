/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  PIC24FJ1024GB610
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/memory/flash.h"

#define FCY 8000000UL
#include <libpic30.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// FBTSEQ - Intial Value (Partition 1)
#pragma config BSEQ = 0xAAA             // Relative value defining which partition will be active after device Reset; the partition containing a lower boot number will be active (Enter Hexadecimal value)
#pragma config IBSEQ = 0x555            // The one's complement of BSEQ; must be calculated by the user and written during device programming. (Enter Hexadecimal value)


/* writeWordNVM
 * Writes 24-bit data to program memory.
 * -Copies the page aligned data first to RAM
 * -Then, modify RAM.
 * -Erases page, then copy the whole page back to the program memory.
 */
void writeWordNVM(uint32_t address, uint32_t data)
{
    uint32_t page_address, page_offset;
    uint32_t i;
    uint32_t page_buffer[FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS];
    memset(page_buffer, 0, sizeof(FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS));
    
    // Get flash page aligned address of flash reserved above for this test.
    page_address = FLASH_GetErasePageAddress(address);
    
    //Copy Contents to Memory
    for (i = 0; i < FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS; i++)
    {
        page_buffer[i] = FLASH_ReadWord24(page_address + 2*i);   
    }
    
    //Get Address Offset
    page_offset = FLASH_GetErasePageOffset(address)/2;
    
    //Modify Value
    page_buffer[page_offset] = data;
    
    //Erase Page
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_ErasePage(page_address);
    
    //Write Page Back to Program Memory
    for(i = 0; i < FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS/FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS; i++)
    {
//        printf("[%lu] Writing to Address %#lx \r\n", (long unsigned int) i, page_address + i*FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS);
        FLASH_WriteRow24(page_address + 2*i*FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS, &page_buffer[i*FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS]);
//        printf("done! \r\n");
    }
    
    // Clear Key for NVM Commands so accidental call to flash routines will not corrupt flash
    FLASH_Lock();
    uint32_t temp = 0;
    //Verify Flash
    for(i = 0; i < FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS; i++)
    {
        temp = FLASH_ReadWord24(page_address + 2*i);
        if(page_buffer[i] != temp)
        {
            printf("[%#lx] Verify Failed! RAM: %#lx ROM: %#lx\r\n", page_address + 2*i, page_buffer[i], temp);
        }
    }
}

/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();

    /* Switch to the 2nd Partition if SW4 is low upon boot
     * SW4: Active Low
     */
    if (SW4_GetValue() == 0)
    {
        /* The Active Partition is fixed at 0x0 while
         * the inactive partition is fixed at 0x400000.
         */
        printf("Switch was pressed \r\n");
        uint32_t fbtseq_active = 0x055FFC, fbtseq_inactive = 0x455FFC;
        uint32_t bsn_active = 0, bsn_inactive = 0;
        
        //Check what is the current partition
        if(NVMCONbits.P2ACTIV == 0)
        {
            printf("1st partition is active, 2nd partition is inactive\r\n");
        }
        else if (NVMCONbits.P2ACTIV == 1)
        {
            printf("2nd partition is active, 1st partition is inactive\r\n");
        }
        
        //Copy BSNs (Mask)
        bsn_active = FLASH_ReadWord24(fbtseq_active) & 0xFFF;
        bsn_inactive = FLASH_ReadWord24(fbtseq_inactive) & 0xFFF;
        
        printf("before bsn active: %#lx inactive: %#lx\r\n", bsn_active,bsn_inactive);
        
        //Ensure inactive BSN is lower than active to switch
        while(bsn_active <= bsn_inactive)
        {
            bsn_inactive--;
        };
        
        printf("after bsn active: %#lx inactive: %#lx\r\n", bsn_active,bsn_inactive);
        
        //Compute for inverse (inactive)
        bsn_inactive = ((~bsn_inactive & 0xFFF) << 12) | (bsn_inactive & 0xFFF);
        //Compute for inverse (active)
        bsn_active = ((~bsn_active & 0xFFF) << 12) | (bsn_active & 0xFFF);
        
        //Write to Program Memory (Also write to active to ensure correct FBT and IBT)
        writeWordNVM(fbtseq_inactive, bsn_inactive);
        writeWordNVM(fbtseq_active, bsn_active);
        
        //Read FBTSEQ
        printf("After FBTSEQ Active: %#lx Inactive: %#lx\r\n",
                (long unsigned int) FLASH_ReadWord24(0x055FFC),
                (long unsigned int) FLASH_ReadWord24(0x455FFC)
              );
        
        printf("Reboot to switch\r\n");
        while(1);
    }

    //Checking Only
    printf("FBTSEQ Active: %#lx Inactive: %#lx\r\n",
            (long unsigned int) FLASH_ReadWord24(0x055FFC),
            (long unsigned int) FLASH_ReadWord24(0x455FFC)
          );
    printf("Partition 1 \r\n");
    
    while (1)
    {
        LED3_Toggle();        
        __delay_ms(100);
    }

    return 1;
}
/**
 End of File
*/

