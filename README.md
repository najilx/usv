# AI Driven Trash Collecting Bot

An advanced unmanned surface vehicle (USV) designed to combat water pollution by autonomously detecting, collecting, and segregating floating debris. This project leverages AI-powered computer vision along with robust mechanical and electronic design to provide both remote control and autonomous operation modes.


## Overview
The AI Driven Trash Collecting Bot project addresses the global challenge of water pollution—specifically, the accumulation of floating waste in confined water bodies. Combining a catamaran-inspired hull for stability with innovative debris collection mechanisms, the USV can operate in both remote-controlled and fully autonomous modes. Computer vision via a YOLO-based object detection model allows the vehicle to precisely detect and navigate towards waste targets, while dual conveyor belts facilitate effective segregation of recyclable and non-recyclable materials.

## Features
- **Dual Operation Modes:**  
  - *Remote Control Mode:* Operated via Bluetooth using a Dabble app, allowing direct user control.
  - *Autonomous Mode:* Utilizes onboard cameras and a YOLO-based detection system to autonomously detect, navigate, and collect waste.
- **Robust Design:**  
  - Catamaran-inspired hull design using PVC for high buoyancy and stability.
  - Differential drive system with four TT gear motors managed by L298N motor drivers.
- **Waste Segregation:**  
  - Dual conveyor belt mechanism to separate recyclable from non-recyclable waste.
- **Real-Time Processing:**  
  - Live video streaming and processing through an ESP32-CAM module in combination with a Python-based client for object detection and command processing.
- **Connectivity:**  
  - ESP32 microcontroller supporting Wi-Fi and Bluetooth for versatile operation and control.

## Architecture
The system follows a client-server model:
- **Client (Python-based system):**
  - Captures and processes live video feeds from the ESP32-CAM.
  - Uses a YOLO-based object detection model to identify waste.
  - Computes navigation commands based on the position of detected waste.
- **Server (ESP32-WROVER):**
  - Receives commands via TCP/IP or Bluetooth.
  - Controls the motors and conveyor belts accordingly.
  - Implements the physical actuation to maneuver the USV and activate the waste collection mechanism.

A simplified data flow is as follows:
1. ESP32-CAM captures live footage.
2. The Python client processes the video, detects waste, and calculates the distance using the Euclidean formula.
3. Appropriate movement commands are sent to the ESP32 server.
4. The server executes motor controls and, when necessary, activates the conveyor belts for debris collection.

## Methodology
The project was implemented in several phases:
1. **Design and Mechanical Engineering:**  
   - Developed a buoyant catamaran hull using PVC pipes.
   - Constructed a bridge deck and integrated the dual conveyor belt system.
2. **Electronics and Embedded Systems:**  
   - Configured the ESP32 microcontroller and ESP32-CAM module.
   - Implemented motor control using L298N drivers and TT gear motors.
3. **Software and AI:**  
   - Developed a YOLO-based object detection model using Python and OpenCV.
   - Created a client-server architecture where the client handles heavy computation (AI) and the server manages real-time control.
4. **Integration and Testing:**  
   - Combined hardware and software modules.
   - Tested remote control (via the Dabble app) and autonomous waste detection/collection operations.
## Usage

### Remote Mode
Simply connect your smartphone running the **Dabble app** to the **ESP32** via **Bluetooth**, and use the virtual joystick to control the USV.

### Autonomous Mode
1. Launch the Python client to initiate waste detection.
2. The client captures a **live video feed** from the ESP32-CAM.
3. Once waste is detected, the client:
   - Calculates the relative position using Euclidean distance.
   - Sends movement commands (FORWARD, LEFT, RIGHT, STOP) to the ESP32 over Wi-Fi.
4. When the USV reaches the target, the **collection mechanism is activated** (dual conveyor belts) to collect and segregate the waste.

---

## Future Scope

- **Field Testing:**  
  Conduct extensive water trials in different conditions to fine-tune buoyancy, navigation, and collection efficiency.

- **Enhanced Object Detection:**  
  Improve detection accuracy with advanced models and integrate additional sensors such as **LIDAR** or **sonar** for better obstacle avoidance.

- **Optimized Power Management:**  
  Research and implement more efficient power usage or alternative sources like **solar panels** to increase runtime.

- **Scalability:**  
  Design systems that coordinate **multiple USVs** for large-scale deployments in lakes, rivers, or reservoirs.

- **Specialized Applications:**  
  Extend functionality to support **oil spill detection**, water quality monitoring, and automated reporting.

---

## Team Members

- **Najil Arfak N N** (VAS21CS076)  
- **Sanjay Biju P** (VAS21CS102)  
- **Mohammed Sinan** (VAS21CS071)  
- **Ramanunni KS** (VAS21CS093)

---

## Acknowledgements

We extend our heartfelt thanks to:

- **Mrs. Prajitha MV**, our project guide, for her continuous support and mentorship.  
- All faculty members of the **Department of Computer Science and Engineering** at **Vidya Academy of Science & Technology**, Thrissur.  
- Our friends and peers who helped us along the journey.  
- Above all, we thank **God Almighty** for the strength and perseverance to complete this project.


