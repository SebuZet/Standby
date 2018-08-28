#!/bin/python 
# Simple script for communication with standby device
# After boot up SBC has to set LOW level on pin 'Connected' to start communication with Standby device
# SBC should monitor 'Shutdown' pin. After detecting HIGH signal SBC should power down itself
# author: Sebastian Zaorski 
import RPi.GPIO as GPIO  
import time, os, sys, configparser

# reads configuration file
config = configparser.ConfigParser()
config.read_file(open(os.path.dirname(sys.argv[0]) + '/standby.ini'))

# Shutdown function 
def Shutdown(channel):  
   print("Shutdown request found. Shutting down the system!")
   os.system("sudo shutdown -h now")  

# check if 'Communication' section exist
if config.has_section('Communication'):

   if config.getboolean('Communication', 'Enabled'):
      PinOE = config.getint('Communication', 'PinOE')

      if config.has_section('Monitor'):

         PinConnected = config.getint('Monitor', 'PinConnected')
         PinShutdown = config.getint('Monitor', 'PinShutdown')

         print ('Starting communication with Standby device with pins ' + str(PinConnected) + ' (connected) and ' + str(PinShutdown) + ' (shutdown). Pin OE: ' + str(PinOE))
         # Use the Broadcom SOC Pin numbers 
         GPIO.setmode(GPIO.BCM)  
         # setup shutdown monitoring pin
         GPIO.setup(PinShutdown, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)  
         # setup connection PIN
         GPIO.setup(PinConnected, GPIO.OUT)
         GPIO.output(PinConnected, GPIO.LOW)
         # enable communication
         GPIO.setup(PinOE, GPIO.OUT)
         GPIO.output(PinOE, GPIO.HIGH)
         time.sleep(1)
	 # Add our function to execute when the button pressed event happens 
         GPIO.add_event_detect(PinShutdown, GPIO.RISING, callback = Shutdown, bouncetime = 500)  
         # Now wait! 
         while 1:  
            time.sleep(1) 
      else:
         print ('ERROR: \'Monitor\' section doesn\'t exist in configuration file.')

   else:
      print ('ERROR: \'Communication\' with Standby device disabled. Skipping request.')

else:
   print ('ERROR: \'Communication\' section doesn\'t exist in configuration file.')
