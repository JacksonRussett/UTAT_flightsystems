'''
	This service will record a video after booting has completed, and stop recording when a button has been pressed and pulls up.
	While this device is recording, an LED will light. Once recording is done (when button is pressed and pulls up), the LED will shut off.
	The LED pin will be pin 24, the button will be on attached on pin 23.
	Finally, this script will trigger the RPi to shutdown when pin 4 gets pulled up (pulled up for now).
	This program is intended for use during rocket launches.
'''
from picamera import PiCamera
from time import sleep
import datetime
import RPi.GPIO as GPIO
from os import system

#gpio module setup
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

print("Starting PiCam recording")

#gpio pins setup
led = 24
button = 23
shutdown_trig = 4
GPIO.setup(led, GPIO.OUT)
GPIO.setup(button, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(shutdown_trig, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

framerate = 25
resolution = (1920, 1080)

#get date and time to name the video
now = datetime.datetime.now()
now = str(now)
#replace the space with underscore for naming
now = now.replace(' ', '_')

filename = "/launch_videos/" + now + ".h264"

should_shutdown = False #if if shutdown_trig has been triggered
continue_recording = True #becomes False when shutdown_trig gets triggered or button gets triggered

def shutdown_trig_callback(channel): #when shutdown_trig gets triggered
	global should_shutdown
	should_shutdown = True
	global continue_recording
	continue_recording = False

def button_callback(channel): #when button gets triggered
	global continue_recording
	continue_recording = False

GPIO.add_event_detect(shutdown_trig, GPIO.RISING, callback=shutdown_trig_callback)
GPIO.add_event_detect(button, GPIO.RISING, callback=button_callback)

with PiCamera() as camera:
	#setup camera params
	camera.resolution = resolution
	camera.framerate = framerate
	camera.start_preview() #camera warmup
	sleep(2)

	camera.start_recording(filename)
	GPIO.output(led, GPIO.HIGH)

	while continue_recording: #continue_recording will get changed by button press or shutdown trigger
		sleep(0.1)

	camera.stop_recording()
	GPIO.output(led, GPIO.LOW)

GPIO.remove_event_detect(shutdown_trig)
GPIO.remove_event_detect(button)

system("chmod 777 " + filename) #give all permissions just in case

#convert file to mp4 so its not hassle later
#reason for weird renaming sequence below is because MP4Box acts retarded with long names
system("mv " + filename + " /launch_videos/temp.h264")
system("MP4Box -fps " + str(framerate) + " -add /launch_videos/temp.h264 /launch_videos/temp.mp4")
system("mv /launch_videos/temp.mp4 /launch_videos/" + now + ".mp4")
#remove h264 file so as to not take as much space
system("rm /launch_videos/temp.h264")

#finally, wait for shutdown signal to happen
GPIO.cleanup()

sleep(1) #give a bit of time before shutdown command in case cleanup happening

system("shutdown -h now")
