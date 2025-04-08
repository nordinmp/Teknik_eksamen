from imutils.video import VideoStream
import cv2
import imutils
import time

def setup_trackbars():
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
    
def get_trackbar_values():
    values = {
        "hL": cv2.getTrackbarPos("H Lower", "Trackbars"),
        "sL": cv2.getTrackbarPos("S Lower", "Trackbars"),
        "vL": cv2.getTrackbarPos("V Lower", "Trackbars"),
        "hU": cv2.getTrackbarPos("H Upper", "Trackbars"),
        "sU": cv2.getTrackbarPos("S Upper", "Trackbars"),
        "vU": cv2.getTrackbarPos("V Upper", "Trackbars"),
        "mA": cv2.getTrackbarPos("Max Area", "Trackbars"),
        "er": cv2.getTrackbarPos("Erode", "Trackbars"),
        "di": cv2.getTrackbarPos("Dilate", "Trackbars"),
        "mR": cv2.getTrackbarPos("Max radius", "Trackbars")
    }
    return values

def detect_ball(frame, values):
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    
    whiteLower = (values["hL"], values["sL"], values["vL"])
    whiteUpper = (values["hU"], values["sU"], values["vU"])
    
    mask = cv2.inRange(hsv, whiteLower, whiteUpper)
    mask = cv2.erode(mask, None, iterations=values["er"])
    mask = cv2.dilate(mask, None, iterations=values["di"])
    
    cv2.imshow("HSV Mask", mask)
    
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    cnts = [c for c in cnts if cv2.contourArea(c) > values["mA"]]
    
    if len(cnts) > 0:
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        
        if M["m00"] != 0 and radius > 5 and radius < values["mR"]:
            return int(x), int(y)
    
    return None

def track_ball():
    print("Starting video stream...")
    vs = VideoStream(src=0).start()
    setup_trackbars()
    time.sleep(2.0)
    
    while True:
        frame = vs.read()
        if frame is None:
            break
        
        frame = imutils.resize(frame, width=600)
        values = get_trackbar_values()
        ball_position = detect_ball(frame, values)
        
        if ball_position:
            print(f"Ball detected at: {ball_position}")
        
        cv2.imshow("Frame", frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
    
    vs.stop()
    cv2.destroyAllWindows()
    
    return ball_position


track_ball()