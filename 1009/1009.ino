const int ButtonPin = 2; 
const int RledPin = 3;  
const int GledPin = 6;  
const int BledPin = 5;  

// --- 按鈕狀態相關變數 ---
int currentButtonReading = 0;
bool ButtonPressed = false;      
unsigned long buttonPressStartTime = 0; 
const long LONG_PRESS_THRESHOLD = 1000; // 長按閾值 1 秒

// --- 顏色索引與模式變數 ---
int currentLedColorIndex = 1; // 短按模式下的當前顏色
bool breathingModeActive = false; 

// --- 呼吸燈效果變數 ---
int brightness = 0;      // LED 亮度 (0-255)
int fadeAmount = 5;      
unsigned long lastFadeTime = 0; 
const int FADE_INTERVAL = 50; // 亮度改變的間隔 (毫秒)

// --- 顏色漸變專用變數 ---
int colorCycleIndex = 1;      // 顏色循環中的當前色
int nextColorCycleIndex = 2;  // 顏色循環中的目標色
unsigned long colorFadeStartTime = 0; // 顏色淡化開始時間
const long COLOR_FADE_DURATION = 2000; // 顏色淡化持續時間 (2 秒)
float fadeProgress = 0.0;             // 0.0 到 1.0 的淡化進度


// 顏色數據表：定義 1-7 顏色組合的 R, G, B 組成
// 1: Red (1,0,0), 2: Green (0,1,0), 3: Yellow (1,1,0), 4: Blue (0,0,1), ...
// 我們用三個位元來表示 R, G, B 的 HIGH/LOW 狀態
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

  // --- 1. 按鈕邏輯處理 (與前一個版本相同) ---
  if (currentButtonReading == LOW && !ButtonPressed) {
    buttonPressStartTime = currentTime; 
    ButtonPressed = true;              
  } 

  if (currentButtonReading == LOW && ButtonPressed && !breathingModeActive && 
      (currentTime - buttonPressStartTime >= LONG_PRESS_THRESHOLD)) {
    
    breathingModeActive = true;       
    // 進入模式時，初始化顏色淡化參數
    colorCycleIndex = currentLedColorIndex; // 從當前顏色開始
    nextColorCycleIndex = (colorCycleIndex % 7) + 1; // 下一個目標顏色
    colorFadeStartTime = currentTime; // 記錄淡化開始時間
    brightness = 0;                   
    fadeAmount = 5;                   
    lastFadeTime = currentTime;       
  }

  if (currentButtonReading == HIGH && ButtonPressed) {
    long pressDuration = currentTime - buttonPressStartTime; 

    if (breathingModeActive) {
      // 退出呼吸模式
      breathingModeActive = false;
      setSolidColor(currentLedColorIndex, 255); 
    } else {
      // 短按：切換單一顏色
      if (pressDuration < LONG_PRESS_THRESHOLD) {
        currentLedColorIndex++;
        if (currentLedColorIndex > 7) { 
          currentLedColorIndex = 1; 
        }
        setSolidColor(currentLedColorIndex, 255); 
      } 
    }
    ButtonPressed = false; 
  }

  // --- 2. 呼吸燈模式下的效果處理 ---
  if (breathingModeActive) {
    
    // 2.1 處理亮度淡入淡出 (呼吸效果)
    if (currentTime - lastFadeTime >= FADE_INTERVAL) {
      brightness += fadeAmount;
      if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount; 
        if (brightness <= 0) brightness = 0;
        if (brightness >= 255) brightness = 255;
      }
      lastFadeTime = currentTime;
    }
    
    // 2.2 處理顏色平滑漸變
    // 計算淡化進度 (0.0 到 1.0)
    fadeProgress = (float)(currentTime - colorFadeStartTime) / COLOR_FADE_DURATION;
    
    // 如果淡化完成，切換到下一個顏色循環
    if (fadeProgress >= 1.0) {
      colorCycleIndex = nextColorCycleIndex;
      nextColorCycleIndex = (colorCycleIndex % 7) + 1;
      colorFadeStartTime = currentTime; // 重新計時
      fadeProgress = 0.0;
    }
    
    // 將當前亮度和淡化進度應用到 LED
    crossFadeLedColor(colorCycleIndex, nextColorCycleIndex, brightness, fadeProgress);
  } 
}

// --- 輔助函數：設置單一顏色的亮度 (用於短按或退出模式) ---
void setSolidColor(int ledColorIndex, int brightnessValue) {
  // 獲取顏色組件 (0 或 1)
  int R_comp = colorPresets[ledColorIndex][0];
  int G_comp = colorPresets[ledColorIndex][1];
  int B_comp = colorPresets[ledColorIndex][2];
  
  // 計算最終輸出亮度：如果組件為 1，則輸出 brightnessValue；否則輸出 0
  int R_out = R_comp * brightnessValue;
  int G_out = G_comp * brightnessValue;
  int B_out = B_comp * brightnessValue;
  
  // 共陽極 LED 反轉輸出 (255 - 亮度)
  analogWrite(RledPin, 255 - R_out);
  analogWrite(GledPin, 255 - G_out);
  analogWrite(BledPin, 255 - B_out);
}


// --- 輔助函數：顏色交叉淡化 (用於呼吸模式) ---
void crossFadeLedColor(int colorA_idx, int colorB_idx, int totalBrightness, float progress) {
  
  int R_final = 0;
  int G_final = 0;
  int B_final = 0;
  
  // 讓顏色 A 淡出 (從 100% 權重降到 0%)
  // 讓顏色 B 淡入 (從 0% 權重升到 100%)
  float weightA = 1.0 - progress;
  float weightB = progress;

  for (int i = 0; i < 3; i++) {
    // colorPresets[index][i] 是 0 或 1 (代表 R, G, B 是否應該亮)
    
    // 計算顏色 A 對最終亮度的貢獻
    float A_contrib = colorPresets[colorA_idx][i] * weightA;
    
    // 計算顏色 B 對最終亮度的貢獻
    float B_contrib = colorPresets[colorB_idx][i] * weightB;
    
    // 疊加貢獻值 (確保總和不超過 1.0)
    float combined_comp = min(1.0, A_contrib + B_contrib);

    // 將呼吸燈總亮度應用到疊加後的顏色組件
    int output_value = (int)(combined_comp * totalBrightness);

    if (i == 0) R_final = output_value;
    else if (i == 1) G_final = output_value;
    else if (i == 2) B_final = output_value;
  }

  // 共陽極 LED 反轉輸出 (255 - 亮度)
  analogWrite(RledPin, 255 - R_final);
  analogWrite(GledPin, 255 - G_final);
  analogWrite(BledPin, 255 - B_final);
}
