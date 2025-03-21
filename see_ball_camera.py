from collections import deque
from imutils.video import VideoStream
import numpy as np
import cv2
import imutils
import time

def nothing(x):
    pass

# Create a window for trackbars
cv2.namedWindow("Trackbars")
cv2.createTrackbar("H Lower", "Trackbars", 0, 179, nothing)
cv2.createTrackbar("S Lower", "Trackbars", 0, 255, nothing)
cv2.createTrackbar("V Lower", "Trackbars", 0, 255, nothing)
cv2.createTrackbar("H Upper", "Trackbars", 179, 179, nothing)
cv2.createTrackbar("S Upper", "Trackbars", 25, 255, nothing)
cv2.createTrackbar("V Upper", "Trackbars", 255, 255, nothing)
cv2.createTrackbar("Max Area", "Trackbars", 400, 2500, nothing)
cv2.createTrackbar("Erode", "Trackbars", 5, 5, nothing)
cv2.createTrackbar("Dilate", "Trackbars", 5, 5, nothing)
cv2.createTrackbar("Max radius", "Trackbars", 25, 250, nothing)

# Initialize video stream
print("Starting video stream...")
vs = VideoStream(src=0).start()

pts = deque(maxlen=64)

while True:
    # Read the frame
    frame = vs.read()
    if frame is None:
        break
    
    frame = imutils.resize(frame, width=600)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    
    # Get values from trackbars
    hL = cv2.getTrackbarPos("H Lower", "Trackbars")
    sL = cv2.getTrackbarPos("S Lower", "Trackbars")
    vL = cv2.getTrackbarPos("V Lower", "Trackbars")
    hU = cv2.getTrackbarPos("H Upper", "Trackbars")
    sU = cv2.getTrackbarPos("S Upper", "Trackbars")
    vU = cv2.getTrackbarPos("V Upper", "Trackbars")
    mA = cv2.getTrackbarPos("Max Area", "Trackbars")
    er = cv2.getTrackbarPos("Erode", "Trackbars")
    di = cv2.getTrackbarPos("Dilate", "Trackbars")
    mR = cv2.getTrackbarPos("Max radius", "Trackbars")

    whiteLower = (hL, sL, vL)
    whiteUpper = (hU, sU, vU)
    
    mask = cv2.inRange(hsv, whiteLower, whiteUpper)
    # Improve mask processing
    mask = cv2.erode(mask, None, iterations=er)  # More erosion to remove black spots
    mask = cv2.dilate(mask, None, iterations=di)  # Expand the remaining shape)
    
    # Debugging visualization
    cv2.imshow("HSV Mask", mask)
    
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    # Filter out small contours
    min_area = mA  # Change this based on the ball size
    cnts = [c for c in cnts if cv2.contourArea(c) > min_area]

    
    if len(cnts) > 0:
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        
        if M["m00"] != 0:
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
        
        if radius > 5 and radius < mR:  # Adjusted threshold
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)
            print(f"Ball detected at: ({x}, {y}), Radius: {radius}")
    
    pts.appendleft(center)
    
    for i in range(1, len(pts)):
        if pts[i - 1] is None or pts[i] is None:
            continue
        thickness = int(np.sqrt(64 / float(i + 1)) * 2.5)
        cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
    
    cv2.imshow("Frame", frame)
    
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break

vs.stop()
cv2.destroyAllWindows()
