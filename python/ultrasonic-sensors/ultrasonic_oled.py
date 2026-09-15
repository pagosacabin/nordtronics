import board
import digitalio
import adafruit_ssd1306
from PIL import Image, ImageDraw, ImageFont
from gpiozero import DistanceSensor
from time import sleep
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


while True:
    
    ultrasonic_dis = round(ultrasonic.distance * 100, 2)
    ultrasonic_dis = str(ultrasonic_dis)
    ultrasonic_dis = "Distance: " + ultrasonic_dis + " cm"

    #print(ultrasonic_dis)

# Draw some shapes
#/usr/share/fonts/truetype/freefont/FreeMono.ttf
    draw.rectangle((0, 0, oled.width-2, oled.height-2), outline=255, fill=0)

    draw.text((5, 11), ultrasonic_dis, font=ImageFont.truetype("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 10), fill=255)


# Display the image
    oled.image(image)
    oled.show()

    sleep(1)
    # Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
    image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image.
    draw = ImageDraw.Draw(image)

    oled.image(image)
    oled.show()