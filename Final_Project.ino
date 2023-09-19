#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "GetFileNameUtils.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // Oled display width 128 pixels
#define SCREEN_HEIGHT 64  // Oled display height 64 pixels

// Digital I/O used
// Set up SPI mode using read data from SD card
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

// Set up Output DAC
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Set up button pause, prev and next audio
#define Play_Pause 16
#define Back 4
#define Next 17

// Set up Oled display connected to I2C (SDA and SCL pinmode)
// Connect to SDA - GPIO21 and SCL - GPIO22
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // The address of SSD1306 is 0x3C

// Create Audtio and Oled display
Audio audio(true);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Init Variables is storage name of media
String fileName;
String state;
uint32_t TimeAudio;
uint32_t TimeCurrentAudio;
float progress = 0;

// Variable using if button click and fix debounce button
int backLastState = HIGH;  // the previous state from the input pin
int backCurrentState;      // the current reading from the input pin
int backLastTime = 0;

int playPauseLastState = HIGH;  // the previous state from the input pin
int playPauseCurrentState;      // the current reading from the input pin
int playPauseLastTime = 0;
int playOrPause = 0;

int nextLastState = HIGH;  // the previous state from the input pin
int nextCurrentState;      // the current reading from the input pin
int nextLastTime = 0;

// Function check if button on click
void checkBackButton();
void checkPlayPauseButton();
void checkNextButton();
void printStateAndFileName();

void setup() {
  // Setup SPI mode using read data from SD card
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);

  // Setup serial baud rate
  Serial.begin(115200);
  SD.begin(SD_CS);

  // Setup button in. Use INPUT_PULLUP to avoid a hovering button situation where there is no defined signal when the button is not pressed
  pinMode(Back, INPUT_PULLUP);
  pinMode(Play_Pause, INPUT_PULLUP);
  pinMode(Next, INPUT_PULLUP);

  //Init display start.
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Setup first play audio
  fileName = "Hero.m4a";
  state = "First Audio..........";
  printStateAndFileName(state);
  audio.setPinout(0, I2S_DOUT, I2S_LRC);
  audio.setVolume(12);  // 0...21
  audio.connecttoFS(SD, fileName.c_str());
}

void loop() {
  audio.loop();
  TimeAudio = audio.getAudioFileDuration();
  TimeCurrentAudio = audio.getAudioCurrentTime();               // Sử dụng hàm để lấy thời gian hiện tại của bài hát
  float progresscurrent = (float)TimeCurrentAudio / TimeAudio;  // Tính tỷ lệ tiến trình phát nhạc
  if (progresscurrent != progress) {
    progress = progresscurrent;
    printStateAndFileName(state);
    if (TimeAudio == 0 && isinf(progresscurrent)) {
      fileName = getNextFileName(SD, fileName);
      state = "Next Audio...........";
      printStateAndFileName(state);
      audio.connecttoFS(SD, fileName.c_str());
    }
  }
  checkBackButton();
  checkPlayPauseButton();
  checkNextButton();
  if (Serial.available()) {  // put streamURL in serial monitor
    audio.stopSong();
    Serial.println(fileName);
    fileName = Serial.readString();
    fileName.trim();
    fileName = fileName + ".m4a";
    Serial.println(fileName);
    state = "Go to................";
    printStateAndFileName(state);
    if (fileName.length() > 1) {
      audio.connecttoFS(SD, fileName.c_str());
    }
    log_i("free heap=%i", ESP.getFreeHeap());
  }
}

void checkBackButton() {
  backCurrentState = digitalRead(Back);
  if (backCurrentState != backLastState && millis() - backLastTime > 500 && backCurrentState == LOW) {
    backLastTime = millis();
    playOrPause = 0;
    fileName = getBackFileName(SD, fileName);
    state = "Back Audio...........";
    printStateAndFileName(state);
    audio.connecttoFS(SD, fileName.c_str());
  }
  // save the the last state
  backLastState = backCurrentState;
}

void checkPlayPauseButton() {
  playPauseCurrentState = digitalRead(Play_Pause);
  if (playPauseCurrentState != playPauseLastState && millis() - playPauseLastTime > 500) {
    // Ghi nhận thời gian nhấn nút cuối cùng
    playPauseLastTime = millis();

    // Cập nhật trạng thái nút trước đó
    playPauseLastState = playPauseCurrentState;
    if (playPauseCurrentState == LOW) {
      if (playOrPause == 0) {
        state = "Pause Audio..........";
        printStateAndFileName(state);
        audio.pauseResume();
        playOrPause = 1;
      } else if (playOrPause == 1) {
        state = "Play Audio...........";
        printStateAndFileName(state);
        audio.pauseResume();
        playOrPause = 0;
      }
    }
  }
}

void checkNextButton() {
  nextCurrentState = digitalRead(Next);
  if (nextCurrentState != nextLastState && millis() - nextLastTime > 500 && nextCurrentState == LOW) {
    nextLastTime = millis();
    playOrPause = 0;
    fileName = getNextFileName(SD, fileName);
    state = "Next Audio...........";
    printStateAndFileName(state);
    audio.connecttoFS(SD, fileName.c_str());
  }
  // save the the last state
  nextLastState = nextCurrentState;
}

void printStateAndFileName(String nameButton) {

  int dotIndex = fileName.lastIndexOf(".");
  String audioName = "Play: " + fileName.substring(0, dotIndex);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, display.height() / 4);
  display.println(nameButton);
  display.setCursor(0, display.height() / 2);
  display.println(audioName);

  int progressBarWidth = display.width();  // Độ rộng thanh kéo
  int progressBarHeight = 4;               // Độ cao thanh kéo
  int progressBarX = 0;
  int progressBarY = display.height() * 9 / 10;
  int progressBarFilledWidth = progressBarWidth * progress;

  // Vẽ khung thanh kéo
  display.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, WHITE);

  // Vẽ phần đã phát
  display.fillRect(progressBarX, progressBarY, progressBarFilledWidth, progressBarHeight, WHITE);
  display.display();
}
