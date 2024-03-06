import serial
from datetime import datetime


ser = serial.Serial('/dev/ttyACM0', 9600)

try:
  while True:
    line = ser.readline().decode().strip()
    current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print(f"[{current_time}] {line}")

except KeyboardInterrupt:
  print("Bye !")
  ser.close()
