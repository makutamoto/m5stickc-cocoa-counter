#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME 5
const char COCOA_UUID[] = "0000fd6f-0000-1000-8000-00805f9b34fb";

#define GRAY M5.Lcd.color565(128, 128, 128)

BLEScan *scan;
int nofDevices = -1;
int progressCounter;
SemaphoreHandle_t mutex;

void drawCount(int count) {
  M5.Lcd.setTextPadding(0);
  M5.Lcd.setTextColor(BLACK, GRAY);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("COCOA", 0, 0);

  M5.Lcd.setTextPadding(M5.Lcd.width());
  if(count < 0) {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Searching...", M5.Lcd.width() / 2, M5.Lcd.height() / 2);  
  } else {
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString(String(count), M5.Lcd.width() / 2, M5.Lcd.height() / 2);
  }
}

void search(void *params) {
  while(true) {
    int count = 0;
    BLEScanResults results = scan->start(SCAN_TIME);
    for(int i = 0;i < results.getCount();i++) {
      BLEAdvertisedDevice device = results.getDevice(i);
      if(device.haveServiceUUID()) {
        if(strcmp(device.getServiceUUID().toString().c_str(), COCOA_UUID) == 0) {
          count++;
        }
      }
    }
    scan->clearResults();
    xSemaphoreTake(mutex, 10);
    nofDevices = count;
    progressCounter = 0;
    xSemaphoreGive(mutex);
  }
}

void setup() {
  M5.begin();
  BLEDevice::init("");
  scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(5000);
  scan->setWindow(4999);
  mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(search, "search", 4096, NULL, 1, NULL, 1);
}

void loop() {
  xSemaphoreTake(mutex, 10);
  drawCount(nofDevices);
  M5.Lcd.fillRect(0, M5.Lcd.height() - 5, M5.Lcd.width(), 5, BLACK);
  M5.Lcd.fillRect(0, M5.Lcd.height() - 5, M5.Lcd.width() * progressCounter / (10 * SCAN_TIME), 5, WHITE);
  progressCounter++;
  xSemaphoreGive(mutex);
  delay(100);
}
