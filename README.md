# WIFI_System_Manager
A web based configuration system for wearable applications

SET ESP SPIFS size to 2MB in 

1. Upload to ESP8266 with "ERASE ALL FLASH CONTENTS"
2. Wait for SPIFS init(format takes 5-10 Seconds) on serail monitor. the esp will reboot after its created its default file system
3. Use FS manager to upload the file "index" in the data folder(ill remove the html its just there for ui testing)
4. Manualy restart the ESP via reboot push button

To load UI:
1. Connect to the ESP network

SSID: WOW-AP
KEY: wowaccesspoint
Web IP:  10.10.10.1

2. Open 10.10.10.1 in your devices browser
3. onLoad calls init()
4. init() calls configureAnimations() which builds the animation database
5. init() calls buildTable() to build the default UI with the default QUE (kind of redundant but will leave for now)
6. init() calls pull() which requests a BINARY data stream in GIF format (it works ok)
7. pull() stores the data in a byteArray the 1st half are the animation indexes, the second half are the durations
8. pull() clears UI and rebuilds the UI table

PUSH now works as well


