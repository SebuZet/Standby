#!/bin/python 
# Simple script for requesting shutdown operation
# After putting pin D5 to LOW and D6 to HIGH levels shutdown device will start power off procedure
# 
# author: Sebastian Zaorski 

import RPi.GPIO as GPIO  
import os  
import configparser, os

# reads configuration file
config = configparser.ConfigParser()
config.read_file(open('standby.ini'))

# check if 'Communication' section exist
if config.has_section('Communication'):

   if config.getboolean('Communication', 'Enabled'):
      PinOE = config.getint('Communication', 'PinOE')

      if config.has_section('Shutdown'):
         PinLow = config.getint('Shutdown', 'PinLow')
         PinHigh = config.getint('Shutdown', 'PinHigh')
         print ('Requesting shut down with pins ' + str(PinLow) + ' (LOW) and ' + str(PinHigh) + ' (HIGH). Pin OE: ' + str(PinOE))

         # Use the Broadcom SOC Pin numbers 
         GPIO.setmode(GPIO.BCM)  

         # setup pins
         GPIO.setup(PinLow, GPIO.OUT)
         GPIO.output(PinLow, GPIO.LOW)
         GPIO.setup(PinHigh, GPIO.OUT)
         GPIO.output(PinHigh, GPIO.HIGH)
         
         # enable communication (just in case)
         GPIO.setup(PinOE, GPIO.OUT)
         GPIO.output(PinOE, GPIO.HIGH)
      else:
         print ('ERROR: \'Shutdown\' section doesn\'t exist in configuration file.')

   else:
      print ('ERROR: \'Communication\' with Standby device disabled. Skipping request.')

else:
   print ('ERROR: \'Communication\' section doesn\'t exist in configuration file.')