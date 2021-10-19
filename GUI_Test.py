from Tkinter import *
# import the needed modules from Tkinter
import tkFont
import tkMessageBox
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
enable_pin = 18; # add the enable pin
coil_A_1_pin = 4 # pink
coil_A_2_pin = 17 # orange
coil_B_1_pin = 23 # blue
coil_B_2_pin = 24 # yellow

# adjust if different
StepCount = 8
Seq = range(0, StepCount)
Seq[0] = [0,1,0,0]
Seq[1] = [0,1,0,1]
Seq[2] = [0,0,0,1]
Seq[3] = [1,0,0,1]
Seq[4] = [1,0,0,0]
Seq[5] = [1,0,1,0]
Seq[6] = [0,0,1,0]
Seq[7] = [0,1,1,0]

GPIO.setup(enable_pin, GPIO.OUT)
GPIO.setup(coil_A_1_pin, GPIO.OUT)
GPIO.setup(coil_A_2_pin, GPIO.OUT)
GPIO.setup(coil_B_1_pin, GPIO.OUT)
GPIO.setup(coil_B_2_pin, GPIO.OUT)

GPIO.output(enable_pin, 1)

# create our main window
win = Tk()

# define the Font
myFont = tkFont.Font(family = 'Helvetica', size = 20, weight = 'bold')

def setStep(w1, w2, w3, w4):
    GPIO.output(coil_A_1_pin, w1)
    GPIO.output(coil_A_2_pin, w2)
    GPIO.output(coil_B_1_pin, w3)
    GPIO.output(coil_B_2_pin, w4)

# define the function to make motor move clockwise
def forward(delay, steps):
    for i in range(steps):
        for j in range(StepCount):
            setStep(Seq[j][0], Seq[j][1], Seq[j][2], Seq[j][3])
            time.sleep(delay)

# define the function to make motor move counterclockwise
def backwards(delay, steps):
    for i in range(steps):
        for j in reversed(range(StepCount)):
            setStep(Seq[j][0], Seq[j][1], Seq[j][2], Seq[j][3])
            time.sleep(delay)

# define the function for the dispensing button to change text
def textChange():
    print("Pill is dispensing")
    result = tkMessageBox.askquestion(title = 'Confirm', message = 'Are you sure?')
   
    # if the result is yes, rotate the motor to dispense pills
    if result == 'yes':
        forward(3 / 1000.0, 64)
        tkMessageBox.showinfo('test', 'Finished')
        # delay = raw_input("Time Delay (ms)?")
        # steps = raw_input("How many steps forward? ")
        # steps = raw_input("How many steps backwards? ")
        # backwards(int(delay) / 1000.0, int(steps))
    # if the result is no, do nothing
    else:
        tkMessageBox.showinfo('test', 'Wrong')
   

win.title("Test GUI")
win.geometry('1000x600')

#Create a button
testButton = Button(win, text = "Dispense", command = textChange, font = myFont, height = 2, width = 6)

# Put button at the top position, other positions: LEFT, RIGHT, BOTTOM
testButton.pack(side = TOP)

mainloop()
