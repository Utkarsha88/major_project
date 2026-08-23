# 🍌 IOT-Based Chemical and Ripeness Detection in Fruits using Machine Learning.

> **An IoT + Machine Learning system for non-destructive fruit quality assessment using environmental, gas-response, and color information.**

This project combines **embedded sensing, IoT data acquisition, remote data transmission, and machine learning** to analyze fruit quality from sensor measurements.

The system uses an **ESP32** connected to a **BME680 environmental/gas sensor** and a **TCS3200 RGB color sensor**. Sensor readings are collected through a controlled measurement cycle, processed on the ESP32, and transmitted to a backend API for further analysis.

The machine learning pipeline uses **XGBoost with SMOTE** to classify three quality-related targets:

* 🍌 **Ripeness:** Unripe, Ripe, Overripe, Spoiled
* 🎨 **Color:** Green, Yellow, Dark
* 🧪 **Chemical Used:** Yes / No

The current ML dataset contains **4,182 samples and 13 columns**, combining RGB measurements, environmental measurements, gas-response features, and target labels.

---

## 📌 Project Overview

Fruit quality is commonly assessed through visual inspection, physical testing, or laboratory-based analysis. These approaches can be subjective, time-consuming, or expensive.

This project explores a sensor-based alternative by combining:

**Physical sensing → Data acquisition → Feature extraction → IoT transmission → Machine Learning → Fruit quality classification**

The system is designed as a prototype research platform for studying whether relatively low-cost sensors can provide useful signals for fruit quality assessment.

---

## 🎯 Objectives

The major objectives of the project are:

1. Develop an embedded sensing system using ESP32.
2. Measure fruit/environment-related parameters using multiple sensors.
3. Establish a baseline gas response before fruit measurement.
4. Capture RGB characteristics using a TCS3200 color sensor.
5. Collect environmental and gas-related measurements using BME680.
6. Generate derived features such as gas difference and VOC percentage.
7. Transmit collected sensor data to a remote backend.
8. Build machine learning models for fruit-quality classification.
9. Handle class imbalance using SMOTE.
10. Evaluate the models using train/test comparison, cross-validation, classification reports, confusion matrices, and learning curves.
11. Investigate feature importance to understand which measurements contribute most to the predictions.

---

# 🏗️ System Architecture

```text
                    ┌─────────────────────┐
                    │      Fruit Sample   │
                    └──────────┬──────────┘
                               │
                ┌──────────────┴──────────────┐
                │                             │
        ┌───────▼────────┐          ┌────────▼────────┐
        │ TCS3200 Sensor │          │   BME680 Sensor │
        │                │          │                 │
        │ RGB / Color    │          │ Temperature     │
        │ Measurement    │          │ Humidity        │
        └───────┬────────┘          │ Pressure        │
                │                   │ Gas Resistance  │
                │                   └────────┬────────┘
                │                            │
                └─────────────┬──────────────┘
                              │
                       ┌──────▼──────┐
                       │    ESP32    │
                       │             │
                       │ Acquisition │
                       │ Filtering   │
                       │ Baseline    │
                       │ Feature Calc│
                       └──────┬──────┘
                              │
                         Wi-Fi / HTTP
                              │
                       ┌──────▼──────┐
                       │ Backend API │
                       └──────┬──────┘
                              │
                       Sensor Dataset
                              │
                    ┌─────────▼─────────┐
                    │ Machine Learning  │
                    │                   │
                    │ SMOTE → XGBoost  │
                    └─────────┬─────────┘
                              │
              ┌───────────────┼────────────────┐
              │               │                │
       ┌──────▼──────┐ ┌──────▼──────┐ ┌──────▼──────┐
       │  Ripeness   │ │    Color    │ │  Chemical   │
       │ 4 Classes   │ │ 3 Classes   │ │  Binary     │
       └─────────────┘ └─────────────┘ └─────────────┘
```

---

# 🔧 Hardware

| Component                     | Purpose                                                     |
| ----------------------------- | ----------------------------------------------------------- |
| **ESP32 DevKit**              | Main microcontroller and Wi-Fi connectivity                 |
| **BME680**                    | Temperature, humidity, pressure, and gas-resistance sensing |
| **TCS3200**                   | RGB/color measurement                                       |
| Push Button                   | Starts a new measurement session                            |
| Connecting wires / breadboard | Hardware interconnection                                    |
| Fruit sample                  | Measurement subject                                         |

The current PlatformIO project targets the **ESP32 DOIT DevKit V1** using the Arduino framework.

---

# 📡 Sensors

## BME680

The BME680 provides:

* Temperature
* Relative humidity
* Atmospheric pressure
* Gas resistance

The firmware applies sensor oversampling and gas-heater configuration before collecting measurements.

Gas resistance is particularly important because the system uses changes in gas response relative to a baseline to derive additional features.

---

## TCS3200

The TCS3200 color sensor is used to obtain RGB measurements.

The firmware:

1. Selects the required color filter.
2. Reads the sensor frequency.
3. Takes multiple readings.
4. Averages the measurements.
5. Maps the readings into RGB values.

The current firmware uses five readings per color measurement and performs color calibration using predefined minimum and maximum values.

---

# 🧠 Measurement Workflow

The ESP32 firmware implements a state-based measurement process.

```text
IDLE
 │
 │ Button Press
 ▼
BASELINE
 │
 │ 4 minutes
 ▼
WAIT FOR FRUIT
 │
 │ 15 seconds
 ▼
STABILIZATION
 │
 │ 30 seconds
 ▼
MONITORING
 │
 │ 7 minutes
 ▼
COMPLETE
 │
 │ 10 seconds
 ▼
IDLE
```

### 1. Baseline

Before placing the fruit, the system records the surrounding gas response.

The baseline is collected for approximately **4 minutes**, with samples taken every second.

The final baseline is calculated as the average gas response:

```text
Baseline Gas =
    Sum of baseline gas readings
    ──────────────────────────────
       Number of readings
```

### 2. Fruit Placement

After baseline collection, the system provides a **15-second window** for placing the fruit.

### 3. Stabilization

The system waits approximately **30 seconds** for the sensor environment to stabilize.

### 4. Monitoring

The fruit is monitored for approximately **7 minutes**.

During this period, the firmware calculates the gas-response difference and VOC-related feature.

### 5. Data Transmission

At the end of the monitoring period, the collected samples are serialized as JSON and transmitted to the configured backend API over HTTP.

These timings and state transitions are implemented directly in the current ESP32 firmware.

---

# 📊 Data Features

The dataset currently contains:

```text
Red
Green
Blue
Temperature
Humidity
Pressure
GasResistance
Difference
VOC%
Baseline
Ripeness
Color
Chemical Used
```

The ML pipeline uses different subsets of these measurements for each prediction task.

### Ripeness Model

Features:

```text
Humidity
GasResistance
Difference
Red
Green
Blue
```

### Color Model

Features:

```text
Red
Green
Blue
```

### Chemical Classification Model

Features:

```text
Temperature
Humidity
Pressure
GasResistance
Difference
VOC%
Red
Green
Blue
```

The current notebook reports **4,182 total samples** and separates the data into training and testing sets using a stratified split with a 35% test set.

---

# 🤖 Machine Learning Pipeline

The current ML implementation uses:

```text
Dataset
   │
   ▼
Label Encoding
   │
   ▼
Train / Test Split
   │
   ▼
SMOTE
   │
   ▼
XGBoost
   │
   ▼
Prediction
   │
   ├── Ripeness
   ├── Color
   └── Chemical Used
```

## Why XGBoost?

XGBoost was selected because it performs well on structured/tabular datasets and can model nonlinear relationships between sensor measurements and quality labels.

The current configuration uses a relatively constrained model to reduce memorization:

```text
n_estimators      = 40
learning_rate     = 0.05
max_depth         = 2
subsample         = 0.8
colsample_bytree  = 0.8
min_child_weight  = 3
reg_lambda        = 1.5
```

The notebook specifically evolved the model to reduce overfitting on the available dataset.

---

# ⚖️ Handling Class Imbalance

The project uses **SMOTE (Synthetic Minority Over-sampling Technique)**.

An important part of the implementation is that SMOTE is placed **inside an `imblearn` Pipeline** rather than being applied to the complete dataset before cross-validation.

```text
Training Fold
     │
     ▼
   SMOTE
     │
     ▼
 XGBoost
```

This prevents synthetic samples generated from validation data from leaking into the training process.

The notebook explicitly uses this approach during 5-fold stratified cross-validation.

---

# 📈 Model Performance

The current notebook reports the following test-set results.

| Task             | Test Accuracy | Macro F1 |
| ---------------- | ------------: | -------: |
| 🍌 Ripeness      |       **98%** | **0.99** |
| 🎨 Color         |       **96%** | **0.97** |
| 🧪 Chemical Used |       **97%** | **0.95** |

### Ripeness

Four classes are predicted:

* Unripe
* Ripe
* Overripe
* Spoiled

Reported test accuracy:

```text
98%
```

Class-level F1 scores range from approximately **0.97 to 1.00**.

### Color

Three classes are predicted:

* Dark
* Green
* Yellow

Reported test accuracy:

```text
96%
```

The Green class has a reported precision of 0.90 and recall of 1.00, while Yellow has precision 1.00 and recall 0.93.

### Chemical Used

Binary classification:

```text
No
Yes
```

Reported test accuracy:

```text
97%
```

The model achieves a reported F1-score of 0.98 for `No` and 0.92 for `Yes`.

> **Important:** `Chemical Used` is a dataset classification label. It should not be interpreted as direct chemical identification, laboratory chemical analysis, or proof of food safety.

---

# 🔬 Cross-Validation

The project performs **5-fold Stratified Cross-Validation**.

Current reported results:

| Task     | Mean CV Accuracy | Standard Deviation |
| -------- | ---------------: | -----------------: |
| Ripeness |       **98.45%** |             ±0.64% |
| Color    |       **95.77%** |             ±3.05% |
| Chemical |       **96.17%** |             ±0.29% |

The notebook flags the very high ripeness score for additional investigation rather than blindly treating it as proof of real-world performance. This is a good practice for a research-oriented ML project.

---

# 🧪 Overfitting Analysis

The project does not rely only on accuracy.

It explicitly compares training and testing accuracy:

```text
Ripeness   Train: 98.64%   Test: 98.29%
Color      Train: 96.28%   Test: 96.17%
Chemical   Train: 95.92%   Test: 96.65%
```

The resulting gaps are very small, indicating good agreement between training and testing performance on the current dataset.

The notebook also includes:

* Learning curves
* Confusion matrices
* Feature importance
* XGBoost training curves
* Validation loss curves
* Live prediction output

These provide additional diagnostics beyond a single accuracy number.

---

# 🔎 Feature Importance

The project calculates XGBoost feature importance separately for:

* Ripeness
* Color
* Chemical classification

This helps investigate which sensor measurements contribute most strongly to each prediction task.

The feature-importance analysis is generated directly from the trained XGBoost models.

---

# ⚡ Embedded Software

The firmware is written in **C++ for ESP32**.

Main responsibilities include:

```text
Wi-Fi connection
      ↓
BME680 acquisition
      ↓
TCS3200 acquisition
      ↓
Signal smoothing
      ↓
Baseline calculation
      ↓
Feature calculation
      ↓
Sample collection
      ↓
JSON serialization
      ↓
HTTP POST
      ↓
Backend
```

The firmware also performs gas smoothing using an exponential moving average:

```text
Smoothed Gas =
    0.9 × Previous Smoothed Gas
    +
    0.1 × Current Gas
```

It additionally calculates the gas-response change:

```text
Gas Rate of Change =
    Current Smoothed Gas - Previous Gas
```

and during monitoring:

```text
Difference =
    Baseline Gas - Smoothed Gas

VOC% =
    Difference / Baseline Gas
```

The current firmware implements these calculations and stores sensor samples before transmitting them.

---

# 📁 Repository Structure

```text
major_project/
│
├── MajorProjectBaishakh/
│   │
│   ├── src/
│   │   └── main.cpp
│   │
│   ├── include/
│   │   └── README
│   │
│   ├── lib/
│   │   └── README
│   │
│   ├── test/
│   │   └── README
│   │
│   ├── myfolder/
│   │   ├── New_final_Code.cpp
│   │   ├── brandnew.cpp
│   │   ├── brandnew_v2.cpp
│   │   ├── final_code.cpp
│   │   ├── long_code.cpp
│   │   ├── main.cpp
│   │   ├── myfile_v.cpp
│   │   ├── new.cpp
│   │   ├── refinedcode.cpp
│   │   ├── robert.cpp
│   │   └── Untitled4 (2).ipynb
│   │
│   ├── BMEtestcode.cpp
│   ├── TSC3200testcode.cpp
│   ├── Myfile.cpp
│   └── platformio.ini
│
└── README.md
```

The repository currently contains the main firmware, sensor test programs, PlatformIO configuration, and the ML notebook.

---

# 🛠️ Software Requirements

## Embedded Development

* VS Code
* PlatformIO
* ESP32 Platform
* Arduino Framework

The PlatformIO configuration currently uses:

```text
Platform: espressif32
Board: ESP32 DOIT DevKit V1
Framework: Arduino
Monitor Speed: 115200
```

Required libraries include:

```text
Adafruit BME680 Library
Adafruit Unified Sensor
```

## Machine Learning

Recommended Python environment:

```text
Python 3.10+
```

Main Python libraries:

```text
pandas
numpy
matplotlib
seaborn
scikit-learn
imbalanced-learn
xgboost
joblib
```

The current notebook is designed to run in Google Colab and includes a dataset upload workflow.

---

# 🚀 Running the ESP32 Firmware

### 1. Clone the repository

```bash
git clone <repository-url>
cd major_project/MajorProjectBaishakh
```

### 2. Open the project

Open the `MajorProjectBaishakh` directory in VS Code with PlatformIO installed.

### 3. Configure credentials

Before flashing the firmware, configure:

```cpp
Wi-Fi SSID
Wi-Fi password
Backend API endpoint
API authentication key
```

**Do not commit real credentials or API keys to Git.**

Use environment-specific or secure configuration wherever possible.

### 4. Connect the ESP32

Connect the ESP32 through USB.

### 5. Build and upload

From PlatformIO:

```text
Build
Upload
Monitor
```

Serial communication runs at:

```text
115200 baud
```

### 6. Start a measurement

After the system is ready:

1. Remove the fruit.
2. Press the start button.
3. Allow baseline collection.
4. Place the fruit when instructed.
5. Allow stabilization.
6. Allow monitoring to complete.
7. Sensor samples are transmitted to the backend.

---

# 🧪 Running the ML Pipeline

Open:

```text
MajorProjectBaishakh/myfolder/Untitled4 (2).ipynb
```

Then run the notebook sequentially.

The pipeline performs:

```text
1. Install/import dependencies
2. Load dataset
3. Inspect dataset
4. Encode target labels
5. Select features
6. Split train/test data
7. Build SMOTE + XGBoost pipelines
8. Perform 5-fold cross-validation
9. Diagnose overfitting
10. Evaluate test set
11. Generate confusion matrices
12. Calculate feature importance
13. Perform live prediction
14. Generate training/validation curves
```

---

# 📋 Dataset Schema

| Column          | Description                       |
| --------------- | --------------------------------- |
| `Red`           | RGB red measurement               |
| `Green`         | RGB green measurement             |
| `Blue`          | RGB blue measurement              |
| `Temperature`   | BME680 temperature                |
| `Humidity`      | BME680 relative humidity          |
| `Pressure`      | BME680 atmospheric pressure       |
| `GasResistance` | BME680 gas resistance             |
| `Difference`    | Gas response relative to baseline |
| `VOC%`          | Derived gas-response percentage   |
| `Baseline`      | Baseline gas measurement          |
| `Ripeness`      | Fruit ripeness label              |
| `Color`         | Fruit color label                 |
| `Chemical Used` | Binary dataset label              |

The current dataset contains **4,182 records and 13 columns**.

---

# 🔐 Security Notice

The embedded firmware currently contains configuration values directly in source code.

For a production or public deployment:

* Never commit Wi-Fi passwords.
* Never commit API keys.
* Rotate any credentials that have already been exposed.
* Use secure configuration files or environment-specific build settings.
* Avoid hardcoding production endpoints where possible.
* Use HTTPS for communication with the backend.
* Validate incoming sensor data on the server.
* Authenticate devices using revocable credentials.

This is especially important because the repository is public.

---

# ⚠️ Research Limitations

Although the current results are strong, they should not be interpreted as proof that the system is ready for commercial food-safety deployment.

Important limitations include:

### Dataset size and diversity

The current dataset contains 4,182 samples, but sensor readings collected from the same experimental setup may not represent all real-world fruit conditions.

### Controlled environment

Temperature, humidity, sensor placement, fruit variety, storage conditions, and measurement geometry can affect sensor readings.

### Potential dataset simplicity

The ripeness model achieves very high performance, so additional testing with independently collected samples is important.

### Chemical classification

The `Chemical Used` target represents a binary dataset label. It is not equivalent to identifying a specific chemical compound or performing laboratory-grade chemical analysis.

### Generalization

Real-world validation should include:

* Different fruit varieties
* Different fruit sizes
* Different storage conditions
* Different environmental conditions
* Different measurement sessions
* Different batches of fruit
* Completely unseen samples

---

# 🔬 Recommended Future Work

The project can be extended in several directions.

## Hardware

* Add ESP32-CAM image acquisition.
* Add firmness measurement using load cell + HX711.
* Improve TCS3200 calibration.
* Improve sensor enclosure and measurement chamber.
* Add controlled airflow.
* Improve sensor positioning and repeatability.

## Machine Learning

* Collect more independent samples.
* Perform leave-one-batch-out validation.
* Test Random Forest, SVM, LightGBM, CatBoost, and neural networks.
* Perform hyperparameter optimization.
* Investigate feature selection.
* Add probability calibration.
* Evaluate robustness under environmental changes.
* Test on completely unseen fruit batches.

## Research Validation

A stronger experimental design would separate:

```text
Training Fruit
       ↓
Validation Fruit
       ↓
Completely Unseen Test Fruit
```

rather than allowing measurements from the same fruit or measurement session to appear across training and testing subsets.

This would provide a more realistic estimate of deployment performance.

---

# 📚 Experimental ML Philosophy

A major focus of this project is not simply achieving a high accuracy score.

The ML workflow explicitly attempts to answer:

> **"Does the model actually generalize, or is it simply memorizing the available sensor data?"**

For this reason, the project includes:

```text
SMOTE inside CV
       +
Stratified 5-Fold CV
       +
Train/Test comparison
       +
Learning curves
       +
Confusion matrices
       +
Feature importance
       +
Independent test evaluation
```

This makes the ML component more suitable for experimentation and research rather than treating accuracy as the only metric.

---

# 📊 Example Prediction

The notebook includes a live prediction example producing:

```text
Ripeness : Unripe
Color    : Green
```

along with class probabilities for the prediction.

The final system can therefore be extended toward a user-facing interface such as:

```text
┌─────────────────────────────────┐
│        FRUIT QUALITY CHECK      │
├─────────────────────────────────┤
│                                 │
│  Ripeness :      RIPE           │
│  Color    :      YELLOW         │
│  Chemical :      NO              │
│                                 │
│  Confidence:      96.4%          │
│                                 │
└─────────────────────────────────┘
```

---

# 🌱 Project Vision

The long-term goal is to develop a low-cost, sensor-assisted fruit-quality assessment platform that can combine multiple physical signals instead of relying exclusively on visual inspection.

The broader concept is:

```text
                    MULTIMODAL
                  FRUIT QUALITY
                   ASSESSMENT
                       │
          ┌────────────┼────────────┐
          │            │            │
       Color        Gas/VOC      Environment
          │            │            │
          └────────────┼────────────┘
                       │
                  Machine Learning
                       │
              ┌────────┴────────┐
              │                 │
         Classification     Confidence
              │                 │
              └────────┬────────┘
                       │
                  Quality Report
```

---

# 👨‍💻 Project

**Major Project**

Developed as an undergraduate engineering project exploring the intersection of:

* Embedded Systems
* IoT
* Sensor Fusion
* Machine Learning
* Data Analysis
* Edge Data Acquisition
* Cloud/API Communication

---

# 📄 License

This project is currently intended primarily for **academic and research purposes**.

If you plan to reuse, distribute, or commercialize the system, please contact the project author regarding licensing and attribution.

---

# ⭐ Acknowledgements

This project makes use of open-source technologies and libraries including:

* ESP32 Arduino ecosystem
* PlatformIO
* Adafruit BME680 library
* Python
* pandas
* NumPy
* scikit-learn
* imbalanced-learn
* XGBoost
* Matplotlib
* Seaborn

---

## Status

```text
Hardware Prototype       ✅
Sensor Acquisition       ✅
ESP32 Firmware           ✅
Baseline Measurement     ✅
Gas Feature Extraction   ✅
RGB Acquisition          ✅
Backend Transmission     ✅
Dataset                  ✅
ML Pipeline              ✅
SMOTE + XGBoost          ✅
Cross Validation         ✅
Overfitting Analysis     ✅
Feature Importance       ✅
Live Prediction          ✅
```

> **Built as a research-oriented prototype for intelligent, sensor-based fruit quality assessment.**
