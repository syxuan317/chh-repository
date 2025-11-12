/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground through 220 ohm resistor
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/digital/Button/
*/

// constants won't change. They're used here to set pin numbers:
// --- 引腳定義 ---
const int buttonPin = 2;  
const int RledPin = 9;    
const int GledPin = 10;   
const int BledPin = 11;   

// --- 時序與狀態控制變數 ---
const long SPIN_INTERVAL = 2000;    
const long FLASH_INTERVAL = 500;    
const long LONG_PRESS_DURATION = 1500; // 1.5 秒 (長按判定時間)

unsigned long previousMillis = 0;   
unsigned long pressStartTime = 0;      
unsigned long lastFadeTime = 0;     
int currentColorIndex = 0;          
int currentMode = 1;                
int previousMode = 1;               // 記錄長按前的模式

// --- 按鈕狀態變數 (用於防抖動和邊緣檢測) ---
int buttonState = HIGH; 
int lastButtonState = HIGH; 
long lastDebounceTime = 0;
long debounceDelay = 50; 
bool isLEDOn = true;                
bool isLongPressedHandled = false; 

// --- 模式 4：呼吸燈變數 ---
int fadeValue = 0;  
const int FADE_STEP = 3; 
const long FADE_DELAY = 25; 
int fadePhase = 0; // 0: 紅色變暗; 1: 藍色變亮; 2: 藍色變暗; 3: 紅色變亮

// 顏色預設陣列 (R, G, B) - 共陽極邏輯 (LOW=亮/0, HIGH=滅/255)
int colorPresets[3][3] = {
  {HIGH, LOW, HIGH}, // 0: 綠色 
  {LOW, LOW, HIGH},  // 1: 黃色 
  {LOW, HIGH, HIGH}, // 2: 紅色 
};

// --- 輔助函數：設置 RGB LED 顏色 ---
void setColor(int R, int G, int B) {
    if (R == HIGH || R == LOW) {
        // 模式 1-3：將 LOW/HIGH 轉換為 0/255 PWM 值
        analogWrite(RledPin, R * 255);
        analogWrite(GledPin, G * 255);
        analogWrite(BledPin, B * 255);
    } else {
        // 模式 4：直接使用 0-255 PWM 值
        analogWrite(RledPin, R);
        analogWrite(GledPin, G);
        analogWrite(BledPin, B);
    }
}

// --- 輔助函數：初始化當前模式的顯示 (不遞增模式) ---
void initializeModeDisplay() {
  
  previousMillis = millis();
  isLEDOn = true; 

  if (currentMode == 1) {
    setColor(colorPresets[currentColorIndex][0], colorPresets[currentColorIndex][1], colorPresets[currentColorIndex][2]);
    Serial.print("初始化顯示：模式 1 (2秒輪播) ");
    Serial.println(currentColorIndex);

  } else if (currentMode == 2) {
    setColor(LOW, LOW, HIGH); 
    Serial.println("初始化顯示：模式 2 (0.5秒閃爍黃燈)");

  } else if (currentMode == 3) {
    setColor(LOW, HIGH, HIGH); 
    Serial.println("初始化顯示：模式 3 (0.5秒閃爍紅燈)");
  }
}

void setup() {
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  
  initializeModeDisplay();
  
  Serial.begin(9600);
  Serial.println("系統啟動：模式 1 (2秒輪播)");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // --- 1. 按鈕狀態檢測與防抖動 (更新 buttonState) ---
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      
      // A. 偵測到按鈕被按下 (HIGH -> LOW 邊緣)
      if (reading == LOW) { 
        pressStartTime = currentMillis; 
        isLongPressedHandled = false; 
        if (currentMode != 4) {
             previousMode = currentMode;
        }

      // B. 偵測到按鈕被放開 (LOW -> HIGH 邊緣)
      } else { 
        if (currentMode == 4) {
          // B1. 從呼吸燈模式恢復
          currentMode = previousMode; 
          Serial.print("長按放開：恢復到模式 ");
          Serial.println(currentMode);
          initializeModeDisplay();
          
        } else if (!isLongPressedHandled) {
          // B2. 短按觸發模式切換
          currentMode++;
          if (currentMode > 3) {
            currentMode = 1; 
          }
          Serial.print("短按觸發：進入下一個模式 ");
          Serial.println(currentMode);
          initializeModeDisplay();
        }
      }
      buttonState = reading;
    }
  }
  lastButtonState = reading;

  // --- 2. 長按模式 (模式 4) 觸發檢查 ---
  if (buttonState == LOW && !isLongPressedHandled && currentMode != 4) {
    if ((currentMillis - pressStartTime) >= LONG_PRESS_DURATION) {
      currentMode = 4;
      isLongPressedHandled = true; 
      
      // 確保從全紅開始呼吸，抑制閃爍
      setColor(LOW, HIGH * 255, HIGH * 255); // R=0, G=255, B=255 (Full Red)
      
      // 在下一個循環開始計時
      fadeValue = 0; 
      fadePhase = 0; 
      Serial.println("長按觸發：進入模式 4 (呼吸燈)");
    }
  }

  // --- 3. 瞬間關閉/恢復功能 (Override Logic) ---
  if (buttonState == LOW && currentMode != 4) { 
    setColor(HIGH * 255, HIGH * 255, HIGH * 255); // 熄滅
    return; // 跳過模式運行邏輯
  }
  
  // --- 4. 模式運行邏輯 ---
  
  if (currentMode == 1) {
    // ... (Mode 1 邏輯) ...
    if (currentMillis - previousMillis >= SPIN_INTERVAL) {
      previousMillis = currentMillis;
      currentColorIndex++;
      if (currentColorIndex > 2) { currentColorIndex = 0; }
      
      setColor(colorPresets[currentColorIndex][0], colorPresets[currentColorIndex][1], colorPresets[currentColorIndex][2]);
    }
    
  } else if (currentMode == 2) {
    // ... (Mode 2 邏輯) ...
    if (currentMillis - previousMillis >= FLASH_INTERVAL) {
        previousMillis = currentMillis;
        if (isLEDOn) { setColor(HIGH * 255, HIGH * 255, HIGH * 255); isLEDOn = false; }
        else { setColor(LOW, LOW, HIGH); isLEDOn = true; } 
    }
    
  } else if (currentMode == 3) {
    // ... (Mode 3 邏輯) ...
    if (currentMillis - previousMillis >= FLASH_INTERVAL) {
        previousMillis = currentMillis;
        if (isLEDOn) { setColor(HIGH * 255, HIGH * 255, HIGH * 255); isLEDOn = false; }
        else { setColor(LOW, HIGH, HIGH); isLEDOn = true; } 
    }
    
  } else if (currentMode == 4) {
    // === 模式 4：紅 -> 暗 -> 藍 -> 暗 -> 紅 完整呼吸燈 ===
    if (currentMillis - lastFadeTime >= FADE_DELAY) {
      lastFadeTime = currentMillis;
      
      int redPWM = 255;
      int bluePWM = 255;
      
      switch (fadePhase) {
          case 0: // Phase 0: 紅色變暗 (Red -> Dim)
              redPWM = fadeValue;       
              bluePWM = 255;            
              break;
          case 1: // Phase 1: 藍色變亮 (Dim -> Blue)
              redPWM = 255;             
              bluePWM = 255 - fadeValue;
              break;
          case 2: // Phase 2: 藍色變暗 (Blue -> Dim)
              redPWM = 255;             
              bluePWM = fadeValue;      
              break;
          case 3: // Phase 3: 紅色變亮 (Dim -> Red)
              redPWM = 255 - fadeValue; 
              bluePWM = 255;            
              break;
      }
      
      fadeValue += FADE_STEP;
      
      if (fadeValue > 255) {
        fadeValue = 0; 
        fadePhase++;   
        if (fadePhase > 3) {
          fadePhase = 0; 
        }
      }
      
      setColor(redPWM, 255, bluePWM); 
    }
  }
}