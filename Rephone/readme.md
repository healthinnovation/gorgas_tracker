# GORGAS tracker scripts
This repository contains the scripts that run the GORGAS tracker. This device is based on [Rephone Geo Kit](https://www.seeedstudio.com/RePhone-Geo-Kit-p-2624.html). Rephone is an open source modular phone that supports different integrated development environments (IDE): Lua, Javascript, Arduino IDE and Eclipse IDE. Our team worked with Arduino IDE.

![RephoneGeoKit](https://github.com/healthinnovation/gorgas_tracker/blob/master/images/RePhone_Geo_Kit.jpg)

## Before you start
We recommend you to visit [Rephone wiki documentation](http://wiki.seeedstudio.com/RePhone/). There you will find information about [Arduino IDE for Rephone](http://wiki.seeedstudio.com/Arduino_IDE_for_RePhone_Kit/) version which contains the libraries you need to upload the code. Also you can go directly to SeeedStudio Github repository and download the [Arduino IDE](https://github.com/Seeed-Studio/Arduino_IDE_for_RePhone).

**Note:** If you are a Linux or iOS user, we have bad news. This Arduino IDE is only available for Windows XP/Vista/⅞/8.1 

## Development
GORGAS tracker algorithm workflow has three stages: GPS files reading (“id.txt”, “perimeter.txt” and “time.txt”.), power management and tracking.
- GPS files reading stage starts with algorithm reading the id file (ID participant and exposure status) to switch a green led (less exposure) or red led (high exposure) on as an initial state. Then time file is read to set up times for power management and the epoch length for tracking. Latitude, longitude, and radius of less exposure are extracted from perimeter file to upload any new configuration.
- Power management stage configures internal time and date variables from GPS data received. The code is always comparing the internal clock and date with the time and date collected from satellites system. If variables are correct, power management allows the control module to collect location and store data on the locations file. Power mode (time file) is read by the power management to switch data logger off or restore its operations.
- GPS-tracker collects participants location at the tracking stage. It is constantly verifying if a participant is outside the village. If a participant is detected outside the village, exposure status is modified and control module switch red LED on. Previous data is always compared with the new ones so the tracker knows if a wrong data was stored. If the device doesn’t receive correct data, it sends a request to satellites system ten times in a row until getting a correct one. Blue LED is turned on when a wrong data was stored.

We developed two script versions:

- Simplified version: Gorgas_Simplificado.ino 
- Extended version: Gorgas_Final.ino

The simplified version of the script is based on functions and to modify the code is easier than the extended version. 

**Note:** You must check [Xadow GSM+BLE](http://wiki.seeedstudio.com/RePhone_Lumi_Kit/#xadow-gsmble) specifications and libraries first, if you want to modify the scripts.
