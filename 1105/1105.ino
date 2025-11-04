// --- I. 硬體定義 ---
const int ButtonPin = 2; 
const int RledPin = 3;  // 只保留紅燈的腳位

// --- II. 模式定義 ---
// 0: 紅色呼吸模式 (Red Breathing Mode)
// 1: 紅色閃爍模式 (Red Flash Mode)
int currentMode = 0; 

// --- III. 按鈕狀態變數 (保留長短按判斷) ---
int currentButtonReading = 0;
bool ButtonPressed = false;      
bool longPressTriggered = false; 
unsigned long buttonPressStartTime = 0; 
const long LONG_PRESS_THRESHOLD = 1000; // 長按閾值 1 秒

// --- IV. 呼吸燈變數 (Mode 0) ---
int brightness = 0;      // LED 亮度 (0-255)
int fadeAmount = 5;      
unsigned long lastFadeTime = 0; 
const int FADE_INTERVAL = 30; // 亮度改變的間隔 (毫秒)

// --- V. 閃爍燈變數 (Mode 1) ---
unsigned long lastFlashTime = 0;
const int FLASH_ON_TIME = 150;  // 閃爍開啟時間 (毫秒)
const int FLASH_OFF_TIME = 150; // 閃爍關閉時間 (毫秒)
bool flashState = false; 


void setup() {
  // 只初始化紅燈腳位 (假設為共陽極 RGB 的 R 腳位)
  pinMode(RledPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP); // 按鈕接 GND，LOW 為按下
  
  // 由於是共陽極，高電位(255)代表關閉。
  // 初始化紅燈為滅，準備進入呼吸模式
  analogWrite(RledPin, 255); 
}

void loop() {
  unsigned long currentTime = millis();
  currentButtonReading = digitalRead(ButtonPin); 

  // --- 1. 按下按鈕的起始處理 ---
  if (currentButtonReading == LOW && !ButtonPressed) {
    buttonPressStartTime = currentTime; 
    ButtonPressed = true;
    longPressTriggered = false; 
  } 

  // --- 2. 持續按住時的長按偵測 ---
  if (currentButtonReading == LOW && ButtonPressed && !longPressTriggered &&
      (currentTime - buttonPressStartTime >= LONG_PRESS_THRESHOLD)) {
    longPressTriggered = true; 
  }

  // --- 3. 放開按鈕的最終處理 (長按切換模式) ---
  if (currentButtonReading == HIGH && ButtonPressed) {
    
    if (longPressTriggered) {
      // **長按處理：切換模式 (0 -> 1 -> 0)**
      currentMode++;
      if (currentMode > 1) { 
        currentMode = 0; 
      }
      
      // 模式切換時的初始化
      if (currentMode == 0) {
        // 模式 0: 紅色呼吸模式，初始化參數
        brightness = 0;                   
        fadeAmount = 5;                   
        lastFadeTime = currentTime; 
      } else if (currentMode == 1) {
        // 模式 1: 紅色閃爍模式，初始化參數
        flashState = true; // 預設亮起
        lastFlashTime = currentTime;
      }
      
    } else {
      // **短按處理：無動作** (保持程式碼簡潔，不執行任何功能)
    }
    
    // 清理狀態
    ButtonPressed = false; 
    longPressTriggered = false;
  }

  // --- 4. 根據當前模式執行效果 ---
  if (currentMode == 0) {
    handleRedBreathingMode(currentTime);
  } else if (currentMode == 1) {
    handleRedFlashMode(currentTime);
  } 
}

// --- 模式 0 紅色呼吸處理函數 ---
void handleRedBreathingMode(unsigned long currentTime) {
  
  if (currentTime - lastFadeTime >= FADE_INTERVAL) {
    brightness += fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount; 
      if (brightness <= 0) brightness = 0;
      if (brightness >= 255) brightness = 255;
    }
    lastFadeTime = currentTime;
    
    // 共陽極邏輯: 亮度越高，寫入值越低
    analogWrite(RledPin, 255 - brightness); 
  }
}


// --- 模式 1 紅色閃爍處理函數 ---
void handleRedFlashMode(unsigned long currentTime) {
  
  if (flashState) {
    // 狀態為亮 (ON)
    if (currentTime - lastFlashTime >= FLASH_ON_TIME) {
      flashState = false; // 切換到滅
      lastFlashTime = currentTime;
      analogWrite(RledPin, 255); // 滅 (共陽極)
    } else {
      analogWrite(RledPin, 0); // 亮 (共陽極)
    }
  } else {
    // 狀態為滅 (OFF)
    if (currentTime - lastFlashTime >= FLASH_OFF_TIME) {
      flashState = true; // 切換到亮
      lastFlashTime = currentTime;
    }
  }
}
