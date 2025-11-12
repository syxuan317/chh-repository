const int ButtonPin = 2; 
const int RledPin = 9;  
const int GledPin = 10;  
const int BledPin = 11;  

// --- 模式定義 ---
// 0: 單色模式 (Short Press Mode)
// 1: 快速顏色閃爍模式 (Fast Flash Mode)
// 2: 顏色閃爍模式 (Sequential Flash Mode)
int currentMode = 0; 

// --- 按鈕狀態相關變數 ---
int currentButtonReading = 0;
bool ButtonPressed = false;      
bool longPressTriggered = false; 
unsigned long buttonPressStartTime = 0; 
const long LONG_PRESS_THRESHOLD = 1000; // 長按閾值 1 秒

// --- 顏色索引與模式變數 ---
int currentLedColorIndex = 1; // 單色模式下的當前顏色

// --- 模式 1：快速閃爍專用變數 (取代呼吸) ---
unsigned long lastMode1FlashTime = 0;
const int MODE1_FLASH_ON_TIME = 50;  // 快速閃爍 - 亮 50ms
const int MODE1_FLASH_OFF_TIME = 50; // 快速閃爍 - 滅 50ms
bool mode1FlashState = true;         // 模式 1 的閃爍狀態：true=亮, false=滅


// --- 顏色漸變專用變數 (模式 1 仍使用) ---
int colorCycleIndex = 1;      
int nextColorCycleIndex = 2;  
unsigned long colorFadeStartTime = 0; 
const long COLOR_FADE_DURATION = 2000; // 顏色漸變週期 2 秒
float fadeProgress = 0.0;             

// --- 模式 2：顏色閃爍專用變數 ---
unsigned long lastFlashTime = 0;
const int FLASH_ON_TIME = 100; // 閃爍開啟時間 (毫秒)
const int FLASH_OFF_TIME = 100; // 閃爍關閉時間 (毫秒)
bool flashState = false; 
int flashColorIndex = 1; 


// 顏色數據表：定義 1-7 顏色組合的 R, G, B 組成
int colorPresets[8][3] = {
  // R G B
  {0, 0, 0}, // 0: 關閉 (未使用)
  {1, 0, 0}, // 1: 紅色
  {0, 1, 0}, // 2: 綠色
  {1, 1, 0}, // 3: 黃色
  {0, 0, 1}, // 4: 藍色
  {1, 0, 1}, // 5: 品紅色
  {0, 1, 1}, // 6: 青色
  {1, 1, 1}  // 7: 白色 (全亮)
};

// --- 函数原型声明 (解决 setSolidColor/crossFadeLedColor 未定义的问题) ---
void setSolidColor(int ledColorIndex, int brightnessValue);
void crossFadeLedColor(int colorA_idx, int colorB_idx, int totalBrightness, float progress);
void handleBreathingMode(unsigned long currentTime);
void handleFlashMode(unsigned long currentTime);
// ------------------------------------------

void setup() {
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP); // 按鈕接 GND，LOW 為按下
  
  // 初始化 LED 為第一個顏色 (紅燈，全亮)
  setSolidColor(currentLedColorIndex, 255);
}

void loop() {
  unsigned long currentTime = millis();
  currentButtonReading = digitalRead(ButtonPin); 

  // --- 1. 按下按鈕的起始處理 ---
  if (currentButtonReading == LOW && !ButtonPressed) {
    buttonPressStartTime = currentTime; 
    ButtonPressed = true;
    longPressTriggered = false; // 每次按下都重置長按狀態
  } 

  // --- 2. 持續按住時的長按偵測 ---
  if (currentButtonReading == LOW && ButtonPressed && !longPressTriggered &&
      (currentTime - buttonPressStartTime >= LONG_PRESS_THRESHOLD)) {
    
    longPressTriggered = true; // 標記為長按事件
  }

  // --- 3. 放開按鈕的最終處理 ---
  if (currentButtonReading == HIGH && ButtonPressed) {
    
    if (longPressTriggered) {
      // **長按處理：切換模式**
      
      currentMode++;
      if (currentMode > 2) { // 0 -> 1 -> 2 -> 0 循環
        currentMode = 0; 
      }
      
      // 模式切換時的初始化
      if (currentMode == 0) {
        // 模式 0: 回到單色模式，顯示當前顏色
        setSolidColor(currentLedColorIndex, 255); 
      } else if (currentMode == 1) {
        // 模式 1: 進入快速閃爍模式，初始化閃爍與漸變參數
        colorCycleIndex = currentLedColorIndex; 
        nextColorCycleIndex = (colorCycleIndex % 7) + 1; 
        colorFadeStartTime = currentTime; 
        mode1FlashState = true;          // 從亮開始
        lastMode1FlashTime = currentTime; // 記錄閃爍開始時間
      } else if (currentMode == 2) {
        // 模式 2: 進入閃爍模式，初始化閃爍參數
        flashColorIndex = 1; 
        flashState = true; 
        lastFlashTime = currentTime;
      }
      
    } else {
      // **短按處理：僅在單色模式下切換顏色**
      if (currentMode == 0) {
        currentLedColorIndex++;
        if (currentLedColorIndex > 7) { 
          currentLedColorIndex = 1; 
        }
        setSolidColor(currentLedColorIndex, 255); 
      } 
    }
    
    // 清理狀態
    ButtonPressed = false; 
    longPressTriggered = false;
  }

  // --- 4. 根據當前模式執行效果 ---
  if (currentMode == 1) {
    handleBreathingMode(currentTime); // 模式 1 (現在是快速閃爍)
  } else if (currentMode == 2) {
    handleFlashMode(currentTime); // 模式 2 (固定顏色閃爍)
  } 
}

// --- 模式 1 快速閃爍和顏色漸變處理函數 (原 handleBreathingMode) ---
void handleBreathingMode(unsigned long currentTime) {
  
  // 1. 顏色漸變邏輯
  // 計算顏色漸變進度
  fadeProgress = (float)(currentTime - colorFadeStartTime) / COLOR_FADE_DURATION;
  
  // 檢查是否完成一個顏色週期
  if (fadeProgress >= 1.0) {
    colorCycleIndex = nextColorCycleIndex;
    nextColorCycleIndex = (colorCycleIndex % 7) + 1;
    colorFadeStartTime = currentTime; 
    fadeProgress = 0.0;
  }
  
  // 2. 快速閃爍邏輯 (50ms 亮 / 50ms 滅)
  if (mode1FlashState) {
    // 狀態為亮 (ON)
    if (currentTime - lastMode1FlashTime >= MODE1_FLASH_ON_TIME) {
      mode1FlashState = false; // 亮週期結束，切換到滅
      lastMode1FlashTime = currentTime;
      // 滅時，設置亮度為 0
      crossFadeLedColor(colorCycleIndex, nextColorCycleIndex, 0, fadeProgress); 
    } else {
      // 亮時，設置亮度為 255
      crossFadeLedColor(colorCycleIndex, nextColorCycleIndex, 255, fadeProgress);
    }
  } else {
    // 狀態為滅 (OFF)
    if (currentTime - lastMode1FlashTime >= MODE1_FLASH_OFF_TIME) {
      mode1FlashState = true; // 滅週期結束，切換到亮
      lastMode1FlashTime = currentTime;
    }
    // 滅狀態時，LED 保持 0 亮度
  }
}


// --- 模式 2 顏色閃爍處理函數 ---
void handleFlashMode(unsigned long currentTime) {
  if (flashState) {
    // 狀態為亮 (ON)
    if (currentTime - lastFlashTime >= FLASH_ON_TIME) {
      flashState = false; 
      lastFlashTime = currentTime;
      setSolidColor(0, 0); // 設置為全滅 (使用 index 0)
    } else {
      setSolidColor(flashColorIndex, 255);
    }
  } else {
    // 狀態為滅 (OFF)
    if (currentTime - lastFlashTime >= FLASH_OFF_TIME) {
      flashState = true; 
      lastFlashTime = currentTime;
      
      // 準備下一個閃爍顏色
      flashColorIndex++;
      if (flashColorIndex > 7) { 
        flashColorIndex = 1; 
      }
    }
  }
}


// --- 輔助函數：設置單一顏色的亮度 ---
void setSolidColor(int ledColorIndex, int brightnessValue) {
  int R_comp = colorPresets[ledColorIndex][0];
  int G_comp = colorPresets[ledColorIndex][1];
  int B_comp = colorPresets[ledColorIndex][2];
  
  int R_out = R_comp * brightnessValue;
  int G_out = G_comp * brightnessValue;
  int B_out = B_comp * brightnessValue;
  
  // 共陽極 LED 反轉輸出 (255 - 亮度)
  analogWrite(RledPin, 255 - R_out);
  analogWrite(GledPin, 255 - G_out);
  analogWrite(BledPin, 255 - B_out);
}


// --- 輔助函數：顏色交叉淡化 ---
void crossFadeLedColor(int colorA_idx, int colorB_idx, int totalBrightness, float progress) {
  
  int R_final = 0;
  int G_final = 0;
  int B_final = 0;
  
  float weightA = 1.0 - progress;
  float weightB = progress;

  for (int i = 0; i < 3; i++) {
    float A_contrib = colorPresets[colorA_idx][i] * weightA;
    float B_contrib = colorPresets[colorB_idx][i] * weightB;
    
    // 確保混合後的單個分量不超過最大值 1.0
    float combined_comp = min(1.0, A_contrib + B_contrib);

    // 乘以總亮度
    int output_value = (int)(combined_comp * totalBrightness);

    if (i == 0) R_final = output_value;
    else if (i == 1) G_final = output_value;
    else if (i == 2) B_final = output_value;
  }
  // 共陽極 LED 反轉輸出
  analogWrite(RledPin, 255 - R_final);
  analogWrite(GledPin, 255 - G_final);
  analogWrite(BledPin, 255 - B_final);
}