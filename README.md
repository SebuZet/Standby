### Standby for AVR microcontrollers
* Remote control with parser for over 10 protocols such as NEC, RC5, RC5x, RC6, and so on
* Task scheduler
* Simple to learn from

### Hardware
* More information can be found [here](https://ssl.diyaudio.pl/showthread.php/28296-Układ-StandBy-obsługa-przycisku-LEDa-dowolnego-pilota "DIYAudio.pl")

### Supported devices
* ATMega328p

### Supported compilers
* GCC (Atmel Studio 7.0)

### Programming
* More information can be found [here](https://ssl.diyaudio.pl/showthread.php/25577-2-kana%C5%82owy-preamp-na-PGA2320?p=450599&viewfull=1#post450599 "Programming with WinAVR") and [here](https://ssl.diyaudio.pl/showthread.php/25577-2-kana%C5%82owy-preamp-na-PGA2320?p=450071&viewfull=1#post450071 "Programming with Atmel Studio")


### Differences from the original version
* Possibility to use two different RCU codes for power ON and power OFF
* Custom behavior if external (SBC based) player is connected (eg. raspberry pi)
* Forcing power off from SBC (using dedicated GPIO pins)
* Passing RCU command to UART (To be implemented)

### Hardware modifications
* Using pin D1 (TX) for sending RCU commands (via UART)
* Using pin D3 (as input ) for detecting SBC player (connected player has to set D3 to LOW)
* Using pin D4 (as output) for signaling shutdown operation (level HIGH means shutdown is in progress)
* Using pins D5 and D6 (as inputs) for detecting shutdown request (if D5 is in LOW level and D6 in HIGH then power off will be proceed)

### Dependencies
* Using external [IRMP](http://www.mikrocontroller.net/articles/IRMP_-_english) library