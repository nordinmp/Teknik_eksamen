import serial
import time

# Opret seriel forbindelse til arduino port
ser = serial.Serial('COM11', 9600, timeout=1)  
time.sleep(2)

# Indtast x og y
x = input("Indtast værdi for x: ")
y = input("Indtast værdi for y: ")

# Send til arduino
ser.write(f"{x} {y}\n".encode())  

# Modtag fra arduino
response = ser.readline().decode().strip()

# Udskriv modtaget besked
print("Modtaget fra Arduino:", response)

ser.close()
