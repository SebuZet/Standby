## Instructions for AVR programming
* All informations are extracted from [here](https://ssl.diyaudio.pl/showthread.php/25577-2-kana%C5%82owy-preamp-na-PGA2320?p=450599&viewfull=1#post450599 "Programming with WinAVR")

## Requirements for Windows
* Install WinAVR
* Drivers for programmer

## Commands
### Init device

avrdude -c usbasp -p m32 -F

Example output:
> avrdude: AVR device initialized and ready to accept instructions
> Reading | ################################################## | 100% 0.03s
> avrdude: Device signature = ...
> avrdude: safemode: Fuses OK
> avrdude done.  Thank you.

### Programming fuse-bits

avrdude -c usbasp -p m32 -F -U lfuse:w:0xff:m -U hfuse:w:0xd9:m

Example output:
> avrdude: AVR device initialized and ready to accept instructions
> 
> Reading | ################################################## | 100% 0.04s
> 
> avrdude: Device signature = 0x1e9514
> avrdude: Expected signature for ATMEGA328P is 1E 95 0F
> avrdude: reading input file "0xff"
> avrdude: writing lfuse (1 bytes):
> 
> Writing | ################################################## | 100% 0.01s
> 
> avrdude: 1 bytes of lfuse written
> avrdude: verifying lfuse memory against 0xff:
> avrdude: load data lfuse data from input file 0xff:
> avrdude: input file 0xff contains 1 bytes
> avrdude: reading on-chip lfuse data:
> 
> Reading | ################################################## | 100% 0.01s
> 
> avrdude: verifying ...
> avrdude: 1 bytes of lfuse verified
> avrdude: reading input file "0xd9"
> avrdude: writing hfuse (1 bytes):
> 
> Writing | ################################################## | 100% 0.01s
> 
> avrdude: 1 bytes of hfuse written
> avrdude: verifying hfuse memory against 0xd9:
> avrdude: load data hfuse data from input file 0xd9:
> avrdude: input file 0xd9 contains 1 bytes
> avrdude: reading on-chip hfuse data:
> 
> Reading | ################################################## | 100% 0.01s
> 
> avrdude: verifying ...
> avrdude: 1 bytes of hfuse verified
> 
> avrdude: safemode: Fuses OK
> 
> avrdude done.  Thank you.

### Programming firmware

avrdude -c usbasp -p m32 -F -U flash:w:standby.hex

Example output:
> avrdude: AVR device initialized and ready to accept instructions
> 
> Reading | ################################################## | 100% 0.04s
> 
> avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
>          To disable this feature, specify the -D option.
> avrdude: erasing chip
> avrdude: reading input file "standby.hex"
> avrdude: input file standby.hex auto detected as Intel Hex
> avrdude: writing flash (17368 bytes):
> 
> Writing | ################################################## | 100% 129.31s
> 
> avrdude: 17368 bytes of flash written
> avrdude: verifying flash memory against standby.hex:
> avrdude: load data flash data from input file standby.hex:
> avrdude: input file standby.hex auto detected as Intel Hex
> avrdude: input file standby.hex contains 17368 bytes
> avrdude: reading on-chip flash data:
> 
> Reading | ################################################## | 100% 98.34s
> 
> avrdude: verifying ...
> avrdude: 17368 bytes of flash verified
> 
> avrdude: safemode: Fuses OK
> 
> avrdude done.  Thank you.
