<!-- Navigable Item -->
<a id="readme-top"></a>

<!-- General Information -->
<div align="center">
  <h1 align="center">Intelligent Wireless Sensor Networks</h1>
  <img width="750" src="https://github.com/DaanF1/IWSN-CarReader/blob/main/Images/Screenshot_end-product.png"/>
  <img width="85" src="https://github.com/DaanF1/IWSN-CarReader/blob/main/Images/Screenshot_LoRa_gateway.png"/>
</div>

<!-- Divider -->
___
<!-- Table Of Contents -->
<details open>
 <summary><strong>📚 Table of Contents</strong></summary>
  
- [About This Repository](#about-this-repository)<br>
   - [Wiring Diagram](#wiring-diagram)<br>
- [Walkthrough](#Walkthrough)<br>
   - [Reading Car Data](#reading-car-data)<br>
   - [The Things Network And Datacake](#the-things-network-and-datacake)<br>
</details>

<!-- Divider -->
___
<!-- Badges -->
[![GitHub Release](https://img.shields.io/github/v/release/DaanF1/IWSN-CarReader?style=for-the-badge&labelColor=%23000000)](https://github.com/DaanF1/IWSN-CarReader/releases)

<!-- About This Repository -->
# About This Repository
This repository contains a custom made CAN-bus reader for a Volkswagen Polo. Collected data from the [CAN-bus](https://nl.wikipedia.org/wiki/Controller_Area_Network) is sent to [The Things Network](https://www.thethingsnetwork.org/) via [LoRa(WAN)](https://www.kpn.com/zakelijk/internet-of-things/lora-netwerk), from where the realtime-data is displayed using a [Datacake](https://datacake.co/) dashboard. <br>
<br>
This project was built using the [Arduino IDE](https://www.arduino.cc/en/software/), together with the [ESP32-TWAI-CAN](https://docs.arduino.cc/libraries/esp32-twai-can/) and [LoRaE5](https://github.com/SylvainMontagny/LoRaE5) libraries. <br>
<br>

<!-- Wiring Diagram -->
## Wiring Diagram
The Wiring Diagram below shows the pin layout of the project, as well as the used components.
<div align="center">
  <img width="800" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/IWSN-Hardware.png"/>
</div>
<div align="center">
  <img width="400" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Auto-opstelling-v2.png"/>
</div>

<!-- Divider -->
___
<!-- Walkthrough -->
# Walkthrough
The project started by making a custom breadboard to fit the ESP32 WROOM Devkit V1:
<div align="center">
  <img width="500" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Screenshot_custom_breadboard.png"/>
</div>
<br>

After this, I made the connections from the OBD2-port adapter to the breadboard using a buck-converter to power the ESP32:
<div align="center">
  <img width="400" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Screenshot_buck_converter_test.png"/>
</div>
<br>

<!-- Reading Car Data -->
## Reading Car Data
To continue, I had to research which data I wanted to read from the car. This was found under the [OBD2 PIDs](https://en.wikipedia.org/wiki/OBD-II_PIDs):
<div align="center">
  <img width="500" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Screenshot_OBD2_PIDs.png"/>
</div>
<br>

I wanted to read the Engine Speed, Vehicle Speed, Mass Air Flow and Fuel Level Input. Based on this data the user could see how efficient they are driving. <br>
The data is read from the CAN-bus using CAN-frames:
<div align="center">
  <img width="400" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Screenshot_CAN_frame_reading.png"/>
</div>
<br>

<!-- The Things Network And Datacake -->
## The Things Network And Datacake
Setting up on The Things Network required registering the LoRa-e5 module. After this, a webhook was used to transport the data (using a payload) to the Datacake dashboard. Here the data can be seen (first image has an error at the beginning):
<div align="center">
  <img width="700" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Datacake-Dashboard-Autorit.png"/>
</div>
<div align="center">
  <img width="800" src="https://github.com/DaanF1/IWSN_CarReader/blob/main/Images/Autoritten-IWSN.png"/>
</div>

<!-- Back To Top -->
<p align="right"><a href="#readme-top">Back To Top</a></p>
