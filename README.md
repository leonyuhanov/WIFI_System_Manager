# WIFI_System_Manager
A web based configuration system for wearable applications

1. Upload to ESP8266 with "ERASE ALL FLASH CONTENTS"
2. Wait for FS init on serail monitor
3. Use FS manager to upload the file "index" in the data folder(ill remove the html its just there for ui testing)
2. Upload code again with "Sketch+WIFI" settings only
4. Use FS manager to upload the file "index" in the data folder AGAIN

To load UI:
1. Connect to the ESP network

SSID: WOW-AP
KEY: wowaccesspoint
Web IP:  10.10.10.1

2. Open 10.10.10.1 in your devices browser
2.1 onLoad calls init()
2.2 init() calls configureAnimations() which builds the animation database
2.3 init() calls buildTable() to build the default UI with the default QUE (kind of redundant but will leave for now)
2.4 unit() calls pull() which requests a BINARY data stream in GIF format (it works ok)
2.4.1 pull() stores the data in a byteArray the 1st half are the animation indexes, the second half are the durations
2.4.2 pull() clears UI and rebuilds the UI table


