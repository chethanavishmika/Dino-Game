# Google Dino Game using ESP32 & OLED Display

A portable, standalone recreation of the classic Chrome offline **Google Dino Runner Game** built using an **ESP32 Development Board**, a **0.96-inch I2C OLED Display**, and a **Tactile Push Button**.

This project brings the legendary obstacle-jumping game to microcontroller hardware, featuring real-time jump mechanics, score tracking, and smooth graphics rendering on a monochrome screen.

---

## 📸 Features

* **Classic Arcade Gameplay:** Jump over obstacles as the game gradually increases in speed.
* **Interactive Control:** Tactile push button wired to the ESP32 for responsive jump mechanics[cite: 20, 21].
* **Live Score Tracking:** On-screen high score and current score counters rendered in real-time[cite: 20, 21].
* **Compact Hardware Setup:** Minimalist breadboard configuration powered entirely via USB[cite: 20, 21].

---

## 🛠 Hardware Required

| Component | Quantity | Description |
| :--- | :--- | :--- |
| **ESP32 Development Board** | 1 | Wi-Fi & Bluetooth microcontroller module[cite: 20, 21] |
| **0.96" I2C OLED Display** | 1 | Monochrome SSD1306 128x64 display[cite: 20, 21] |
| **Push Button** | 1 | Tactile push button for jumping[cite: 20, 21] |
| **Breadboard & Wires** | - | Jumper cables for power and data links[cite: 20, 21] |

---

## 🔌 Wiring & Pin Mapping

### 1. 0.96" OLED Display to ESP32 (I2C)
| OLED Display Pin | ESP32 Pin | Wire Color (in diagram) | Description |
| :--- | :--- | :--- | :--- |
| **GND** | **GND** | Black[cite: 21] | Ground Rail |
| **VDD** | **3.3V** | Red[cite: 21] | Power Supply |
| **SCK** | **GPIO 22** | Yellow[cite: 21] | I2C Clock Line |
| **SDA** | **GPIO 21** | Blue[cite: 21] | I2C Data Line |

### 2. Push Button to ESP32
| Button Terminal | ESP32 Pin | Wire Color (in diagram) | Description |
| :--- | :--- | :--- | :--- |
| **Terminal 1** | **GPIO 15** | Green[cite: 21] | Jump Trigger Signal (configured with INPUT_PULLUP) |
| **Terminal 2** | **GND** | Black / Green[cite: 21] | Common Ground Rail |

---

## 📚 Required Libraries

Install these libraries via the **Arduino Library Manager** (*Sketch > Include Library > Manage Libraries*):

1. **`Adafruit SSD1306`** by Adafruit
2. **`Adafruit GFX Library`** by Adafruit

---

## 🚀 How to Run

1. Wire your ESP32, OLED display, and Push Button on the breadboard according to the pinout tables above.
2. Open Arduino IDE, install the **Adafruit SSD1306** and **Adafruit GFX libraries**, and select your **ESP32 board**.
3. Upload the sketch code to your ESP32 board.
4. Press the **push button** to make the dinosaur jump over incoming obstacles and start playing!

---

## 📜 License
This project is open-source under the **MIT License**.
  display.display();
  delay(30);
}
