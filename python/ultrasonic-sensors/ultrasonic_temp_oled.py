import csv
import board
import digitalio
import adafruit_ssd1306
from PIL import Image, ImageDraw, ImageFont
from gpiozero import DistanceSensor
from datetime import datetime

import os
import glob
import time

os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

base_dir = '/sys/bus/w1/devices/'
device_folder = glob.glob(base_dir + '28*')[0]
device_file = device_folder + '/w1_slave'


ultrasonic = DistanceSensor(echo=5 , trigger=17, max_distance=2 )

# Define the I2C pins used by the OLED display
i2c = board.I2C()
oled = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, addr=0x3c)

# Clear display
oled.fill(0)
oled.show()

# Create a blank image for drawing
image = Image.new("1", (oled.width, oled.height))
draw = ImageDraw.Draw(image)

def read_temp_raw():
    f = open(device_file, 'r')
    lines = f.readlines()
    f.close()
    return lines

def read_temp():
    lines = read_temp_raw()
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = read_temp_raw()
    equals_pos = lines[1].find('t=')
    if equals_pos != -1:
        temp_string = lines[1][equals_pos+2:]
        temp_c = float(temp_string) / 1000.0
        temp_f = temp_c * 9.0 / 5.0 + 32.0
        return temp_c, temp_f
    
#define data to write
data = [
         [ Timestamp, Distance, Temp-C, Temp-F],
         [ datetime.now().strftime("%Y-%m-%d %H:%M"),ultrasonic.distance, temp_c, temp_f]
]

while True:
    # Create blank image for drawing.
    # Make sure to create image with mode '1' for 1-bit color.
    #image = Image.new("1", (oled.width, oled.height))

    # Get drawing object to draw on image.
    #draw = ImageDraw.Draw(image)

    #oled.image(image)
    #oled.show()

    temp_c,temp_f = read_temp()
    temp = (temp_c, temp_f)
    temp = str(temp)
    temp = temp.replace("(", "temp: ")  
    temp = temp.replace(",", " C")
    temp = temp.replace(")", " F")  

    print(temp)
    
    ultrasonic_dis = round(ultrasonic.distance * 100, 2)
    ultrasonic_dis = str(ultrasonic_dis)
    ultrasonic_dis = "Distance: " + ultrasonic_dis + " cm"

    #print(ultrasonic_dis)

# Draw some shapes
#/usr/share/fonts/truetype/freefont/FreeMono.ttf
    draw.rectangle((0, 0, oled.width-1, oled.height-1), outline=255, fill=0)

    draw.text((5, 5), ultrasonic_dis, font=ImageFont.truetype("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 10), fill=255)
    draw.text((5, 16), temp, font=ImageFont.truetype("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 10), fill=255)


#open file int write mode
    with open('data.csv', mode='w') as file:
        writer = csv.writer(file)
        writer.writerows(data)

# Display the image
    oled.image(image)
    oled.show()
    time.sleep(1)
