# Wingate Anaerobic Test
This software is compatible with a custom made tachometer (consisting of an Arduino Nano development board and a TCRT5000 optical sensor) mounted on a Monark ergometer bicycle with 16 black markers around the outer edge of the flywheel.
An installer package is available for Windows (XP, 7, 10). The software should also work on Linux (tested on Linux Mint 18.1) and macOS, but you must compile the source code yourself using Qt Creator 4.0.3 with Qt 5.6.2. This older version is used for compatibility with Windows XP, but if that isn't necessary, you are welcome to use the newest version of Qt.

# Installation on Windows XP, 7 and 10
1. Go to the "Releases"-tab on GitHub and download the latest installer executable (eg. "wingate-0.1-setup.exe")
2. Run the executable and follow the instructions on the screen.
3. After installing, go to the installation folder (eg. "C:\Program Files (x86)\Wingate Anaerobic Test")
4. Open the folder "CH341SER-driver" and run SETUP.exe to install the driver for the CH340/CH341 IC on the Arduino board.
5. You're good to go!
