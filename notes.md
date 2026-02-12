# Instructions

Follow the steps I took below to properly flash the bootloader and built FC code onto the BlitzMiniF745.  
1. Build the appropriate bootloader (and code). This needs to be done only the very first time since the FC comes with a BetaFlight bootloader.
    ```
    ./waf configure --board BlitzMiniF745
    ./waf copter
    ```
    This should generate a .hex file in `build/BlitzMiniF745/bin` named `arducopter_with_bl.hex`.  
2. Download and install BetaFlight version 10.8 (10.9 and 10.10 have [bugs in them](https://github.com/betaflight/betaflight-configurator/issues/3833)). Run BetaFlight. While the FC is powered off, hold down the Boot button, and power it on via USB. You should see "DFU" mode at the top right hand corner, instead of a COM port. This is good because it means that the FC is in the appropriate mode to flash a new bootloader onto it.  

    If you don't see this "DFU" mode, you may need to download this [ImpulseRC_Driver_Fixer.exe](https://github.com/ImpulseRC/ImpulseRC_Driver_Fixer/blob/main/ImpulseRC_Driver_Fixer.exe) and run it while holding down the Boot button on the FC...

    Once successful, in BetaFlight click the "Firmware Flasher" section on the left pane and make sure `No reboot sequence` and `Full chip erase` are both checked on. Also, enable `Manual baud rate` if necessary and set it to `115200` as this is the default for the FC. At the bottom right, click `Load Firmware [Local]`,  select the `build/BlitzMiniF745/bin/arducopter_with_bl.hex` file, and click `Flash Firmware`. You should see a progress bar that says "Erasing", then "Flashing", then "Verifying". Once the log says "Programming SUCCESSFUL", unplug the USB cable and plug it back in (without holding the Boot button). Your PC should not recognize a new "ArduPilot" or "MAVLink" device in Device Manager.
3. Download and install MissionPlanner. After running, you should be able to successfully connect to the board.
4. To view debug logs from the code while in WSL2, install [USBIPD-WIN](https://github.com/dorssel/usbipd-win.git) on Windows by running `winget install usbipd`. You should then be able to list your connected USB devices, including the FC, with `usbipd list` in powershell as administrator. Find the `BUSID` for the FC, and run the following to bind and attach the device to wsl...  
    ```
    usbipd bind --busid <busid>
    usbipd attach --wsl --busid <busid>
    ```  
    Then go to your WSL/Ubuntu terminal and verify the FC is present with `lsusb`. Check the device name with `ls /dev/ttyACM*` and fix permissions to give yourself access with `sudo chmod 666 /dev/ttyACM0`. Then, in the main ardupilot code directory, run `mavproxy.py --master=/dev/ttyACM0`. You should see stuff printing... type `status` and hit enter to see specific FC metrics.
5. In the future, since the bootloader was already flashed and is compatible with Ardupilot code, you only need to run `./waf copter --upload` to build and upload code to the FC.


Here's the gemini thread I've been querying for reference: https://share.google/aimode/h0LhjRqGtX33M5OnZ

