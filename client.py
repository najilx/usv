import socket
import cv2
import math
import time
import numpy as np
from ultralytics import YOLO

# ---------------- Configuration ----------------
ESP32_IP = "192.168.43.66"           # IP for sending commands
ESP32_PORT = 80
ESP32_CAM_URL = "http://192.168.43.165:81/stream"

FRAME_WIDTH, FRAME_HEIGHT = 640, 480
FRAME_CENTER = (FRAME_WIDTH // 2, FRAME_HEIGHT // 2)
STOP_THRESHOLD = 100
FRAME_SKIP = 5  # Process every 5th frame

# Recyclable Waste Classes
RECYCLABLE_CLASSES = {
    "recyclable", "aluminum can", "cardboard", "glass bottle",
    "paper", "plastic bottle", "plastic bag", "tin", "zip plastic bag"
}

# ---------------- YOLO Model ----------------
model = YOLO("weights/best_model.pt")

# ---------------- Connection Function ----------------
def send_command(command):
    """Open a new connection to ESP32, send command, then close the connection."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(10)
            s.connect((ESP32_IP, ESP32_PORT))
            s.sendall(f"{command}\n".encode('utf-8'))
            print(f"📤 Sent Command: {command}")
            # Brief delay to allow the ESP32 to process the command before closing
            time.sleep(1)
    except Exception as e:
        print(f"⚠️ Error sending command '{command}': {e}")

# ---------------- Utility Functions ----------------
def calculate_distance(p1, p2):
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

def classify_waste(detected_class):
    """Determine if detected waste is recyclable or non-recyclable."""
    if detected_class.lower() in RECYCLABLE_CLASSES:
        return "Recyclable", "Recyclable Belt"
    else:
        
        return "Non-Recyclable", "Non-Recyclable Belt"

# ---------------- Video Processing ----------------
def video_detection_loop():
    """Main loop to capture video, run object detection, and send commands."""
    cap = cv2.VideoCapture(ESP32_CAM_URL)
    if not cap.isOpened():
        print("❌ Error: Cannot access ESP32-CAM")
        return

    frame_count = 0
    last_detected_class = None
    last_action_time = time.time()

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("🔄 Reconnecting to camera...")
            cap.release()
            time.sleep(2)
            cap = cv2.VideoCapture(ESP32_CAM_URL)
            continue

        frame_count += 1
        if frame_count % FRAME_SKIP != 0:
            continue  # Skip frames for performance

        # Preprocess frame
        frame = cv2.resize(frame, (FRAME_WIDTH, FRAME_HEIGHT))
        movement_direction = "FORWARD"
        nearest_distance = float('inf')
        detected_class = None

        # Object detection using YOLO
        results = model.predict(frame, conf=0.5, imgsz=640, device="cpu")
        for result in results:
            for box in result.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                x_center = (x1 + x2) // 2
                y_center = (y1 + y2) // 2
                distance = calculate_distance(FRAME_CENTER, (x_center, y_center))

                # Draw bounding box
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

                if distance < nearest_distance:
                    nearest_distance = distance
                    detected_class = model.names[int(box.cls[0])]
                    deviation_x = x_center - FRAME_CENTER[0]
                    if abs(deviation_x) > 50:
                        movement_direction = "RIGHT" if deviation_x > 0 else "LEFT"
                    else:
                        movement_direction = "FORWARD"

        # When object is close enough, process waste detection
        if detected_class and nearest_distance < STOP_THRESHOLD:
            movement_direction = "STOP"
            detected_waste_type, belt_action = classify_waste(detected_class)
            # Only process if a new waste type is detected or enough time has passed
            if detected_class != last_detected_class or time.time() - last_action_time > 5:
                print(f"♻️ Processing {detected_waste_type} waste ({detected_class})")
                send_command("STOP")
                send_command(belt_action)
                time.sleep(3)  # Wait for sorting to complete
                send_command("FORWARD")
                last_detected_class = detected_class
                last_action_time = time.time()
        else:
            # If no waste is detected or object is not close enough, send movement commands
            send_command(movement_direction)

        # Overlay information on frame
        cv2.putText(frame, f"Move: {movement_direction}", (20, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        if detected_class:
            cv2.putText(frame, f"Detected: {detected_class}", (20, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
            cv2.putText(frame, f"Distance: {int(nearest_distance)}", (20, 120),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)

        cv2.imshow("Waste Detection", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# ---------------- Main ----------------
if __name__ == '__main__':
    video_detection_loop()

