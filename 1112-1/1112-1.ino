// --- I. 硬體定義 ---
// --- 引腳定義 ---
const int buttonPin = 2;  
const int RledPin = 3;    
const int GledPin = 10;  
const int BledPin = 11;  

// --- 時序與狀態控制常數 (所有時間單位為毫秒) ---
const long FLASH_1_5S = 1000; // 模式 2: 間隔 1.5 秒
const long FLASH_1_0S = 400; // 模式 3: 間隔 1.0 秒
const long FLASH_0_5S = 100;  // 模式 4: 間隔 0.5 秒

// 共陽極邏輯 (Common Anode): 0=亮, 255=滅
const int PWM_ON = 0;    
const int PWM_OFF = 255; 

// --- 顏色定義：原始 D3 角 (紅光) ---
// {R, G, B}
const int COLOR_RED_ON[] = {PWM_ON, PWM_OFF, PWM_OFF};      // 紅燈亮 (0, 255, 255)
const int COLOR_ALL_OFF[] = {PWM_OFF, PWM_OFF, PWM_OFF};    // 完全熄滅 (255, 255, 255)

// --- 狀態控制變數 ---
unsigned long previousMillis = 0;  
int currentMode = 1;               // 模式 1 ~ 4
bool isLEDOn = true;               // 記錄 LED 當前狀態 (用於閃爍)

// --- 按鈕狀態變數 (用於防抖動) ---
int buttonState = HIGH; 
int lastButtonState = HIGH; 
long lastDebounceTime = 0;
long debounceDelay = 50; 

// --- 輔助函數：設置 RGB LED 顏色 (使用 PWM 陣列) ---
void setRGB_Color(const int color[]) {
    analogWrite(RledPin, color[0]);
    analogWrite(GledPin, color[1]);
    analogWrite(BledPin, color[2]);
}

// --- 輔助函數：初始化當前模式的顯示 ---
void initializeModeDisplay() {
    
    previousMillis = millis();
    isLEDOn = true; // 進入新模式時，預設 LED 是亮的

    Serial.print("進入模式 ");
    Serial.print(currentMode);

    if (currentMode == 1) {
        setRGB_Color(COLOR_RED_ON); // 恆亮模式：紅色
        Serial.println(" (恆亮)");
    } else {
        // 閃爍模式
        setRGB_Color(COLOR_RED_ON); // 閃爍模式：先確保 LED 是紅色亮的
        Serial.print(" (閃爍，間隔 ");
        if (currentMode == 2) Serial.println("1.5s)");
        else if (currentMode == 3) Serial.println("1.0s)");
        else if (currentMode == 4) Serial.println("0.5s)");
    }
}

void setup() {
    pinMode(RledPin, OUTPUT);
    pinMode(GledPin, OUTPUT);
    pinMode(BledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP); // 使用內部上拉電阻

    Serial.begin(9600);
    Serial.println("系統啟動：單鍵模式切換，固定紅光。");
    
    // 初始化為模式 1
    initializeModeDisplay();
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
            
            // 偵測到按鈕被放開 (LOW -> HIGH 邊緣)
            if (reading == HIGH) { 
                // 短按觸發模式切換
                currentMode++;
                if (currentMode > 4) {
                    currentMode = 1; 
                }
                initializeModeDisplay();
            }
            buttonState = reading;
        }
    }
    lastButtonState = reading;

    // --- 2. 模式運行邏輯 ---
    long flashInterval = 0;
    
    if (currentMode == 1) {
        // 模式 1: 恆亮
        if (!isLEDOn) {
            setRGB_Color(COLOR_RED_ON);
            isLEDOn = true;
        }
    } else {
        // 模式 2, 3, 4: 閃爍模式
        if (currentMode == 2) flashInterval = FLASH_1_5S;
        else if (currentMode == 3) flashInterval = FLASH_1_0S;
        else if (currentMode == 4) flashInterval = FLASH_0_5S;
        
        // 檢查是否到達閃爍間隔時間
        if (currentMillis - previousMillis >= flashInterval) {
            previousMillis = currentMillis;
            
            // 切換 LED 狀態
            if (isLEDOn) { 
                setRGB_Color(COLOR_ALL_OFF); // 熄滅
                isLEDOn = false; 
            } else { 
                setRGB_Color(COLOR_RED_ON);  // 點亮 (紅色)
                isLEDOn = true; 
            } 
        }
    }
}
