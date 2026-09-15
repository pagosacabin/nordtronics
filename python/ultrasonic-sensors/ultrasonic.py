
from gpiozero import DistanceSensor
from time import sleep
ultrasonic = DistanceSensor(echo=5 , trigger=17, max_distance=2 )

def hello():
    print("Hello")

def goodbye():
    print("Goodbye")

while True:
    print(ultrasonic.distance)
    ultrasonic.when_in_range = hello
    ultrasonic.when_out_of_range = goodbye

    sleep(1)

