# VideoToTFTDisplay
VIdeo file frame to TFT Display Pixels buffer converter

How to use:

1. copy the video you want to play to TFT Display, even better if its already just black and white
2. edit video filename at line 4 FrameExtractor.py and run
3. It will generate frame folder and fill it up with every frame
4. start FrameConverter.py to convert it to TXT file

   every single line on that txt will contain 1 frame of data, every 1 byte will contain 8 pixel data as stated in adafruit TFT display buffer format

5. copy the file generated into SD CARD! make sure its formatted as FAT32!
6. open badoled.ino and flash to ESP32s3! if you use ESP32, use this pin instead:

#define SDMMC_CLK   14
#define SDMMC_CMD   15
#define SDMMC_D0    2
#define SDMMC_D1    4
#define SDMMC_D2    12
#define SDMMC_D3    13

DO NOT use PULL UP resistor on pin 12 on ESP32! otherwise it will never able to boot

7. connect TFT display (128*64 px) to SDA and SCL pins, on ESP32s3 its SDA:8 SCL:9, ESP32 SDA:21 SCL:22
