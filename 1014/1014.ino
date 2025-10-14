const int ButtonPin = 2; 
const int RledPin = 3;  
const int GledPin = 6;  
const int BledPin = 5;  

// --- 模式定義 ---
// 0: 單色模式 (Short Press Mode)
// 1: 呼吸漸變模式 (Breathing Mode)
// 2: 顏色閃爍模式 (Flash Mode)
int currentMode = 0; 

// --- 按鈕狀態相關變數 ---
int currentButtonReading = 0;
bool ButtonPressed = false;      
// *** 新增：追蹤長按是否成立，以避免短按誤觸 ***
bool longPressTriggered = false; 
unsigned long buttonPressStartTime = 0; 
const long LONG_PRESS_THRESHOLD = 1000; // 長按閾值 1 秒

// --- 顏色索引與模式變數 ---
int currentLedColorIndex = 1; // 單色模式下的當前顏色

// --- 呼吸燈效果變數 ---
int brightness = 0;      // LED 亮度 (0-255)
int fadeAmount = 5;      
unsigned long lastFadeTime = 0; 
const int FADE_INTERVAL = 50; // 亮度改變的間隔 (毫秒)

// --- 顏色漸變專用變數 (呼吸模式使用) ---
int colorCycleIndex = 1;      
int nextColorCycleIndex = 2;  
unsigned long colorFadeStartTime = 0; 
const long COLOR_FADE_DURATION = 2000; 
float fadeProgress = 0.0;             

// --- 顏色閃爍專用變數 (閃爍模式使用) ---
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
  // 當按住超過閾值，設置 longPressTriggered 為 true
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
        // 模式 1: 進入呼吸模式，初始化呼吸與漸變參數
        colorCycleIndex = currentLedColorIndex; 
        nextColorCycleIndex = (colorCycleIndex % 7) + 1; 
        colorFadeStartTime = currentTime; 
        brightness = 0;                   
        fadeAmount = 5;                   
        lastFadeTime = currentTime;       
      } else if (currentMode == 2) {
        // 模式 2: 進入閃爍模式，初始化閃爍參數
        flashColorIndex = 1; 
        flashState = true; 
        lastFlashTime = currentTime;
      }
      
    } else {
      // **短按處理：僅在單色模式下切換顏色**
      // 注意：pressDuration < LONG_PRESS_THRESHOLD
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
    handleBreathingMode(currentTime);
  } else if (currentMode == 2) {
    handleFlashMode(currentTime);
  } 
  // 模式 0 (單色模式) 不需額外呼叫函數
}

// --- 模式 1 呼吸漸變處理函數 (不變) ---
void handleBreathingMode(unsigned long currentTime) {
  
  if (currentTime - lastFadeTime >= FADE_INTERVAL) {
    brightness += fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount; 
      if (brightness <= 0) brightness = 0;
      if (brightness >= 255) brightness = 255;
    }
    lastFadeTime = currentTime;
  }
  
  fadeProgress = (float)(currentTime - colorFadeStartTime) / COLOR_FADE_DURATION;
  
  if (fadeProgress >= 1.0) {
    colorCycleIndex = nextColorCycleIndex;
    nextColorCycleIndex = (colorCycleIndex % 7) + 1;
    colorFadeStartTime = currentTime; 
    fadeProgress = 0.0;
  }
  
  crossFadeLedColor(colorCycleIndex, nextColorCycleIndex, brightness, fadeProgress);
}


// --- 模式 2 顏色閃爍處理函數 (不變) ---
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


// --- 輔助函數：設置單一顏色的亮度 (不變) ---
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


// --- 輔助函數：顏色交叉淡化 (不變) ---
void crossFadeLedColor(int colorA_idx, int colorB_idx, int totalBrightness, float progress) {
  
  int R_final = 0;
  int G_final = 0;
  int B_final = 0;
  
  float weightA = 1.0 - progress;
  float weightB = progress;

  for (int i = 0; i < 3; i++) {
    float A_contrib = colorPresets[colorA_idx][i] * weightA;
    float B_contrib = colorPresets[colorB_idx][i] * weightB;
    
    float combined_comp = min(1.0, A_contrib + B_contrib);

    int output_value = (int)(combined_comp * totalBrightness);

    if (i == 0) R_final = output_value;
    else if (i == 1) G_final = output_value;
    else if (i == 2) B_final = output_value;
  }

  analogWrite(RledPin, 255 - R_final);
  analogWrite(GledPin, 255 - G_final);
  analogWrite(BledPin, 255 - B_final);
}