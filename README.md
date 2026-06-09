# Autonomous Self-Balancing Robotics Platform (Thesis Project)

An advanced, heterogeneous dual-processor two-wheeled inverted pendulum (balancing robot) designed to showcase professional-grade systems integration, deterministic real-time scheduling, and modular hardware architectures. 

The project decouples time-critical locomotion controls from high-level cognitive edge AI tasks using an industrial **CAN bus backbone**. It also features a custom-designed **multi-board PCB ecosystem**, a plug-and-play **hardware expansion bay**, and a custom-built **distributed fault isolation/fail-safe system**.

---

## 🏗️ System Architecture & Domain Separation

The robot is engineered with absolute domain separation to guarantee real-time stability while processing heavy data-driven applications:

### 1. The Real-Time Locomotion Domain (STM32)
* **Hardware:** WeAct STM32F412RET6 (ARM Cortex-M4 @ 100MHz).
* **Software:** Bare-metal C running on **FreeRTOS**.
* **Locomotion:** Dual JGA25-370 12V DC geared motors equipped with quadrature Hall-effect encoders driven by a high-efficiency MOSFET **TB6612FNG** controller.
* **Sensing:** MPU6050 6-Axis IMU (via $I^2C$) and dual HC-SR04 ultrasonic distance sensors (using hardware timer input-capture interrupts to avoid blocking code).
* **Responsibilities:** Executes a deterministic **250Hz (4ms) PID control loop** and an optimized **Complementary Sensor Fusion filter** to maintain vertical stability.

### 2. The High-Level Application Brain (Raspberry Pi 4)
* **Hardware:** Raspberry Pi 4 (Embedded Headless Linux).
* **Software:** Multi-threaded, non-blocking **Python** application stack.
* **Edge AI Integration:** 100% offline local speech processing pipeline. Features Speech-to-Text (STT) via quantized `Whisper.cpp` and a local Small Language Model (LLM via `Ollama`) executing context-aware intent parsing and vocal generation via Text-to-Speech (TTS).
* **User Interface:** A 4-inch HDMI diagnostic touchscreen dashboard and an SPI-driven MAX7219 8x8 LED matrix array rendering emotional eye animations synced dynamically with LLM output tokens.

### 3. The Spinal Cord (CAN Bus Backbone)
* **Protocol:** Standard CAN V2.0B network running at 500kbps / 1Mbps.
* **Hardware Interfacing:** Managed via dual external MCP2515 controllers over SPI. 
* **Linux Integration:** Leverages native Linux **SocketCAN** drivers to bridge hardware inputs natively to the OS network layer.

---

## 🛠️ Custom Hardware Architecture & Modular Design

To eliminate breadboard wire clutter and minimize electromagnetic interference (EMI) from the high-current motors, the system is segmented across three custom-routed PCBs:

1. **Power Distribution Board (PCB #1):** Interfaces with a 3S LiPo battery pack via an XT30 panel connector. Features high-current 12V traces ($\approx 1.5\text{ mm}$) and dedicated 5V/3A step-down buck regulators. It isolates the Raspberry Pi's CAN controller from inductive motor noise.
2. **STM32 Locomotion Shield (PCB #2):** A custom daughterboard hosting the STM32F412, IMU, ultrasonic sensor headers, and the MOSFET motor driver.
3. **Modular Backpack Expansion Bay (Cartridge Slot):** A hot-swappable physical bay using a spring-loaded **4-pin pogo pin interface** (`VCC`, `GND`, `CAN_H`, `CAN_L`). It allows self-contained modules powered by low-cost **RISC-V microcontrollers** (CH32V series) to gather peripheral environmental data and stream it autonomously to the main network.

---

## 🔬 Thesis Focus: Distributed Fail-Safe & Diagnostic Isolation

The core research contribution of this thesis focuses on multi-tiered system reliability and deterministic fault detection without relying on high-overhead mathematics.

### 1. Dual-Direction Heartbeat Protocol (The Dead Man's Switch)
The Python background architecture broadcasts a periodic 1-byte keep-alive frame (`CAN ID 0x700`, payload `0xAA`) onto the bus every 20ms. The STM32 catches this frame inside a dedicated hardware interrupt, resetting an internal counter. 
* If the Linux OS experiences a kernel panic, memory exhaustion, or freezes during an AI inference, the STM32 misses the heartbeat for $>100\text{ ms}$ (5 dropped frames) and declares a critical fault.
* **Graceful Multi-Stage Shutdown Sequence:** Instead of cutting power instantly (which induces a violent kinetic crash), the STM32 executes an automated, staged safe-recovery routine:
  * **Stage 1 (0–200ms):** Command PID target velocities to zero to brake the chassis and stabilize in place.
  * **Stage 2 (200–500ms):** Slowly adjust the internal tilt setpoints forward by a few degrees, forcing a controlled, low-velocity descent onto its bumpers.
  * **Stage 3 (500ms+):** Pull the TB6612FNG standby pin `LOW` to completely isolate the motor windings from power.

### 2. Hierarchical 1-Byte Error Bitmask Registry
Instead of clogging the CAN bus network with lengthy text diagnostics or creating dozens of disparate CAN frames, low-level `if` statement error checks on the STM32 toggle single bits inside a unified `uint8_t` registry:
* `Bit 0 (0x01)`: MPU6050 IMU Failure ($I^2C$ Timeout)
* `Bit 1 (0x02)`: Left Ultrasonic Timeout
* `Bit 2 (0x04)`: Right Ultrasonic Timeout
* `Bit 3 (0x08)`: Motor Driver Thermal/Current Trip
* `Bit 5 (0x20)`: 3S LiPo Battery Critically Low ($<9.9\text{V}$)

This byte is pushed over `CAN ID 0x050` (System Diagnostics). The Pi’s Python background thread ingests the frame, performs fast bitwise `AND` (`&`) extractions, and dynamically routes alerts, debugging text, and visual error graphics to the 4" LCD interface.

---

## 📂 Repository File Structure

```text
