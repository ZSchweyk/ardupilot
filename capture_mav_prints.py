from pymavlink import mavutil
import time

# 1. Connect to the flight controller
# Replace 'udpin:localhost:14550' with your actual connection string (e.g., '/dev/ttyUSB0' or 'COM3')
connection = mavutil.mavlink_connection('/dev/ttyACM0', baud=115200)

print("Announcing GCS to Flight Controller...")
for _ in range(5):
    connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_GCS, 
                                  mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)
    time.sleep(0.5)

connection.wait_heartbeat()
print("Connected!")

# 3. Continuous loop to listen for status text
while True:
    msg = connection.recv_match(blocking=True)
    if msg:
        if msg.get_type() == 'STATUSTEXT':
            print(f"TEXT: {msg.text}")
        # This will print EVERY message type to prove data is flowing
        # print(f"Received: {msg.get_type()}") 

