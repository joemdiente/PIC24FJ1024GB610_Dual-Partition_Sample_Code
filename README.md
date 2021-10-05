# PIC24FJ1024GB610 Dual Partition Sample Code



​	This sample code demonstrates how to switch between partitions by writing FBTSEQ Configuration words.

The Boot Sequence Number is decremented during switch.



Setup:

- Explorer 16/32 Development Board
- PIC24FJ1024GB610 Plug-in Module

Connections:

- Connect USB cable to Serial (Baud rate: 9600) for debug messages
- Hold SW4 before reset to swap to Partition 2
- Hold SW3 before reset to swap to Partition 1
- LED3 blinks every 150ms if Partition 1 is mapped to Active partition
- LED9 blinks every 500ms if Partition 2 is mapped to Active partition