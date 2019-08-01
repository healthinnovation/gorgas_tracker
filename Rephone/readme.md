# GORGAS tracker scripts
This repository contains the scripts that run the GORGAS tracker. This device is based on [Rephone Geo Kit](https://www.seeedstudio.com/RePhone-Geo-Kit-p-2624.html). Rephone is an open source modular phone that supports different integrated development environments (IDE): Lua, Javascript, Arduino IDE and Eclipse IDE. Our team worked with Arduino IDE.

![RephoneGeoKit](https://github.com/healthinnovation/gorgas_tracker/blob/master/images/RePhone_Geo_Kit.jpg)

## Before you start
We recommend you to visit [Rephone wiki documentation](http://wiki.seeedstudio.com/RePhone/). There you will find information about [Arduino IDE for Rephone](http://wiki.seeedstudio.com/Arduino_IDE_for_RePhone_Kit/) version which contains the libraries you need to upload the code. Also you can go directly to SeeedStudio Github repository and download the [Arduino IDE](https://github.com/Seeed-Studio/Arduino_IDE_for_RePhone).

**Note:** The Arduino IDE for Rephone is only available for Windows XP/Vista/⅞/8.1. Other versions soon. 

## Scripts
Algorithms are based on three tasks: **GPS files reading**, **Power Management**, and **GPS-tracking logs**. GPS files contain the data and medata divided in four plain text files (.txt):
* locations.txt: GPS coordinates, timestamp, number of satellites, battery life status, and quality of the GPS signal
* id.txt: participant ID and location status (to identify and report if a participant move outside that boundary)
* perimeter.txt: centroid and radius of the community (to set a buffer area)
* time.txt: time interval and time for active and sleep modes

### Workflow
- **GPS files reading:** starts with algorithm reading the id file to switch the led (red or green) on as an initial state. Then, the time file is read to set up times for power management and the time interval for tracking.
- **Power management:** configures internal time and date variables from GPS data received. The code is always comparing the internal clock and date with the time and date collected from satellites system. If variables are correct, power management allows the control module to collect location and store data on the locations file. Power mode (time file) is read by the power management to switch data logger off or restore its operations.
- **GPS-tracking logs:** collects participants location during the follow-up and constantly verify if a participant is outside the village boundary. As result update the location status in the id file. 

We developed two script versions:
- Simplified version: Gorgas_Simplificado.ino 
- Extended version: Gorgas_Final.ino

The simplified version of the script is based on functions and to modify the code is easier than the extended version. 

**Note:** You must check [Xadow GSM+BLE](http://wiki.seeedstudio.com/RePhone_Lumi_Kit/#xadow-gsmble) specifications and libraries first, if you want to modify the scripts.
