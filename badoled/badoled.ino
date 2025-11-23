#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SD_MMC.h>

#define SDMMC_CLK   39
#define SDMMC_CMD   38
#define SDMMC_D0    40
#define SDMMC_D1    1
#define SDMMC_D2    41
#define SDMMC_D3    42

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

const char* filename = "/frame3.txt";
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Double buffering
uint8_t ImageBuffer1[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
uint8_t ImageBuffer2[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
uint8_t* readBuffer = ImageBuffer1;
uint8_t* displayBuffer = ImageBuffer2;

// Sync primitives
SemaphoreHandle_t bufferSwapSemaphore;
bool frameReady = false;
bool playbackDone = false;
const uint32_t FRAME_TIME_MS = 6; 
uint32_t lastFrameTime = millis();
void displayTask(void* param) {
  while (!playbackDone) {
    uint32_t currentTime = millis();
    uint32_t elapsedTime = currentTime - lastFrameTime;
    

    if (elapsedTime < FRAME_TIME_MS) {
      vTaskDelay((FRAME_TIME_MS - elapsedTime) / portTICK_PERIOD_MS);
    }
    // Tunggu frame baru siap
    if (xSemaphoreTake(bufferSwapSemaphore, portMAX_DELAY)) {
      if (frameReady) {
        // Swap buffer
        uint8_t* temp = displayBuffer;
        displayBuffer = readBuffer;
        readBuffer = temp;
        frameReady = false;
        
        // Tampilkan ke OLED
        display.clearDisplay();
        display.drawBitmap(0, 0, displayBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        display.display();
        lastFrameTime = millis();
      }
      xSemaphoreGive(bufferSwapSemaphore);
    }
    vTaskDelay(1); // Yield ke task lain
  }
  vTaskDelete(NULL);
}

void readTask(void* param) {
  File f = SD_MMC.open(filename, "r");
  if (!f) {
    Serial.println("Gagal buka frame.txt");
    playbackDone = true;
    vTaskDelete(NULL);
    return;
  }

  Serial.println("Mulai baca frame.txt");
  size_t lineNum = 1;
  int fps_track = 0;
  long startMillis = millis();

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    // Parse ke readBuffer
    int numBytes = min((int)line.length() / 2, SCREEN_WIDTH * SCREEN_HEIGHT / 8);
    for (int i = 0; i < numBytes; i++) {
      String byteStr = line.substring(i*2, i*2+2);
      readBuffer[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
    }

    // Tandai frame siap & swap
    if (xSemaphoreTake(bufferSwapSemaphore, portMAX_DELAY)) {
      frameReady = true;
      xSemaphoreGive(bufferSwapSemaphore);
    }

    lineNum++;
    fps_track++;
    if (millis() - startMillis >= 1000) {
      Serial.printf("FPS: %.2f\n", (float)fps_track);
      startMillis = millis();
      fps_track = 0;
    }
    
    vTaskDelay(1); // Yield
  }

  f.close();
  Serial.println("Selesai baca.");
  playbackDone = true;
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED gagal init");
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Init...");
  display.display();

  SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0, SDMMC_D1, SDMMC_D2, SDMMC_D3);
  if (!SD_MMC.begin("/sdcard", false, false, 40000, 5)) {
    Serial.println("SD MMC gagal mount");
    return;
  }

  if (!SD_MMC.exists(filename)) {
    Serial.println("frame.txt tidak ditemukan");
    return;
  }

  // Create semaphore
  bufferSwapSemaphore = xSemaphoreCreateMutex();

  // Create tasks (Core 0 untuk display, Core 1 untuk SD read)
  xTaskCreatePinnedToCore(displayTask, "Display", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(readTask, "Read", 8192, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(100);
}