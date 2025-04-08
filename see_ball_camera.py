from imutils.video import VideoStream
import cv2
import imutils
import time
import serial

def setup_trackbars():
    def nothing(x): pass
    cv2.namedWindow("Trackbars")
    cv2.createTrackbar("H Lower", "Trackbars", 0, 179, nothing)
    cv2.createTrackbar("S Lower", "Trackbars", 0, 255, nothing)
    cv2.createTrackbar("V Lower", "Trackbars", 0, 255, nothing)
    cv2.createTrackbar("H Upper", "Trackbars", 179, 179, nothing)
    cv2.createTrackbar("S Upper", "Trackbars", 25, 255, nothing)
    cv2.createTrackbar("V Upper", "Trackbars", 255, 255, nothing)
    cv2.createTrackbar("Max Area", "Trackbars", 400, 2500, nothing)
    cv2.createTrackbar("Erode", "Trackbars", 2, 5, nothing)
    cv2.createTrackbar("Dilate", "Trackbars", 2, 5, nothing)
    cv2.createTrackbar("Max radius", "Trackbars", 24, 250, nothing)

def get_trackbar_values():
    try:
        values = {
            "hL": cv2.getTrackbarPos("H Lower", "Trackbars"),
            "sL": cv2.getTrackbarPos("S Lower", "Trackbars"),
            "vL": cv2.getTrackbarPos("V Lower", "Trackbars"),
            "hU": cv2.getTrackbarPos("H Upper", "Trackbars"),
            "sU": cv2.getTrackbarPos("S Upper", "Trackbars"),
            "vU": cv2.getTrackbarPos("V Upper", "Trackbars"),
            "mA": cv2.getTrackbarPos("Max Area", "Trackbars"),
            "er": max(1, cv2.getTrackbarPos("Erode", "Trackbars")),
            "di": max(1, cv2.getTrackbarPos("Dilate", "Trackbars")),
            "mR": cv2.getTrackbarPos("Max radius", "Trackbars")
        }
        return values
    except:
        return {
            "hL": 0, "sL": 0, "vL": 0,
            "hU": 179, "sU": 255, "vU": 255,
            "mA": 400, "er": 1, "di": 1, "mR": 24
        }

def detect_ball(frame, values):
    try:
        blurred = cv2.GaussianBlur(frame, (11, 11), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

        hL, hU = sorted([values["hL"], values["hU"]])
        sL, sU = sorted([values["sL"], values["sU"]])
        vL, vU = sorted([values["vL"], values["vU"]])

        mask = cv2.inRange(hsv, (hL, sL, vL), (hU, sU, vU))
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

            if M["m00"] != 0 and 5 < radius < values["mR"]:
                return int(x), int(y)
    except Exception as e:
        print(f"[Fejl i detect_ball]: {e}")
    return None

def track_ball():
    print("[INFO] Starter video og seriel kommunikation...")

    try:
        vs = VideoStream(src=1).start()
        print("[INFO] VideoStream startet")
    except Exception as e:
        print(f"[FEJL] Kamera kunne ikke startes: {e}")
        return

    setup_trackbars()
    print("[INFO] Trackbars oprettet")

    try:
        ser = serial.Serial('COM8', 9600, timeout=1)
        print("[INFO] Serial forbindelse åben på COM8")
        time.sleep(2)
    except Exception as e:
        print(f"[FEJL] Kunne ikke åbne COM8: {e}")
        ser = None

    time.sleep(2)

    # === Preview-fase: kamera og sliders virker, men motor ikke startet ===
    print("\n[FORHÅNDSVISNING] Juster sliders og placer bolden.")
    print("[FORHÅNDSVISNING] Tryk ENTER i terminalen for at starte motorstyring...\n")

    while True:
        frame = vs.read()
        if frame is None:
            print("[FEJL] Ingen frame fra kamera")
            continue

        frame = imutils.resize(frame, width=600)
        values = get_trackbar_values()
        ball_pos = detect_ball(frame, values)

        if ball_pos:
            x, y = ball_pos
            cv2.circle(frame, (x, y), 10, (0, 255, 0), 2)
            cv2.circle(frame, (x, y), 2, (0, 0, 255), -1)

        cv2.imshow("Frame", frame)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            vs.stop()
            cv2.destroyAllWindows()
            return

        # Brug klassisk input for ENTER i alle miljøer
        try:
            import msvcrt
            if msvcrt.kbhit() and msvcrt.getch() == b'\r':
                break
        except ImportError:
            input("Tryk ENTER i terminalen for at starte motorstyring...")
            break

    if ser:
        ser.write("START\n".encode())
        print("[INFO] Sendte 'START' til Arduino")

    print("[INFO] Starter hovedloop...")

    while True:
        try:
            frame = vs.read()
            if frame is None:
                print("[FEJL] Ingen frame fra kamera")
                continue

            frame = imutils.resize(frame, width=600)
            values = get_trackbar_values()
            ball_pos = detect_ball(frame, values)

            if ball_pos:
                x, y = ball_pos
                print(f"[KAMERA] Bold fundet på x={x}, y={y}")
                if ser:
                    try:
                        ser.write(f"{x} {y}\n".encode())
                        response = ser.readline().decode().strip()
                        if response:
                            print(f"[ARDUINO] Svar: {response}")
                    except Exception as e:
                        print(f"[FEJL] Serial skrivning: {e}")

                cv2.circle(frame, (x, y), 10, (0, 255, 0), 2)
                cv2.circle(frame, (x, y), 2, (0, 0, 255), -1)

            cv2.imshow("Frame", frame)
            if cv2.waitKey(1) & 0xFF == ord("q"):
                print("[INFO] Afslutter...")
                break

        except Exception as e:
            print(f"[FEJL i hoved-loop]: {e}")
            continue

    vs.stop()
    if ser:
        ser.close()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    print("[INFO] Starter program...")
    track_ball()
