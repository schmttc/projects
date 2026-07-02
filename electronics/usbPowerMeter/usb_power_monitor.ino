#include <Wire.h>
#include <Adafruit_INA228.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Adafruit_INA228 ina228;

// Nokia 5110: CLK (2), DIN (11), DC (6), CE (7), RST (8)
Adafruit_PCD8544 display = Adafruit_PCD8544(2, 11, 6, 7, 8);

const unsigned long DISPLAY_INTERVAL_MS = 2000;
unsigned long lastDisplay = 0;

// Hardware Synchronization Timing (307.2ms)
const unsigned long SAMPLE_INTERVAL_MS = 307; 
unsigned long lastSample = 0;

// Precise energy integration variables (Power x Time)
double totalEnergy_mWh = 0.0;
const double HOURS_PER_SAMPLE = (307.2 / 1000.0) / 3600.0;

// --- Always-Climbing Lifetime Average Variables ---
unsigned long totalPowerSamplesCount = 0;
double lifetimePowerSum = 0.0; // double prevents overflow issues over long periods

const unsigned char sigma_bitmap[] PROGMEM = {
  0b11111000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b11111000
};

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!ina228.begin()) {
    Serial.println(F("INA228 error"));
    while (1) delay(10);
  }

  ina228.setShunt(0.015, 10.0);
  ina228.setAveragingCount(INA228_COUNT_1024);
  ina228.setVoltageConversionTime(INA228_TIME_150_us);
  ina228.setCurrentConversionTime(INA228_TIME_150_us);

  display.begin();
  display.setContrast(50);
  display.setRotation(2);
  display.clearDisplay();
  display.display();
}

void formatScaled(float val, const char* unitBase, const char* unitUp,
                   const char* unitDown, char* buf) {
  float v = val;
  const char* unit = unitBase;

  if (fabs(val) >= 1000.0) {
    v = val / 1000.0;
    unit = unitUp;
  } else if (fabs(val) < 1.0 && val != 0.0) {
    v = val * 1000.0;
    unit = unitDown;
  }

  dtostrf(v, 0, 3, buf);
  strcat(buf, " ");
  strcat(buf, unit);
}

void formatScaledShort(float val, const char* unitBase, const char* unitUp,
                        const char* unitDown, char* buf) {
  float v = val;
  const char* unit = unitBase;

  if (fabs(val) >= 1000.0) {
    v = val / 1000.0;
    unit = unitUp;
  } else if (fabs(val) < 1.0 && val != 0.0) {
    v = val * 1000.0;
    unit = unitDown;
  }

  dtostrf(v, 0, 1, buf);
  strcat(buf, unit);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample += SAMPLE_INTERVAL_MS;
    
    float current_mA = ina228.getCurrent_mA();
    float power_mW   = ina228.getPower_mW();
    
    // Wh Tracking: Integrate milliwatts over sample time slice
    totalEnergy_mWh += (double)power_mW * HOURS_PER_SAMPLE;

    // Accumulate for lifetime average
    lifetimePowerSum += power_mW;
    totalPowerSamplesCount++;
  }

  if (now - lastDisplay < DISPLAY_INTERVAL_MS) return;
  lastDisplay = now;

  float busVoltage_V = ina228.getBusVoltage_V();
  float current_mA   = ina228.getCurrent_mA();
  float power_mW     = ina228.getPower_mW(); 

  char vStr[15], iStr[15], pStr[15], energyStr[15];
  formatScaled(busVoltage_V, "V", "kV", "mV", vStr);
  formatScaled(current_mA, "mA", "A", "uA", iStr);
  formatScaled(power_mW, "mW", "W", "uW", pStr);
  
  // Format scaled values using mWh / Wh / uWh metrics
  formatScaled((float)totalEnergy_mWh, "mWh", "Wh", "uWh", energyStr);

  // Calculate the single lifetime running average
  float lifetimeAvg = 0.0;
  if (totalPowerSamplesCount > 0) {
    lifetimeAvg = (float)(lifetimePowerSum / totalPowerSamplesCount);
  }

  char avgStr[15];
  formatScaledShort(lifetimeAvg, "mW", "W", "uW", avgStr);

  // Serial logs
  Serial.print(F("V: ")); Serial.print(vStr);
  Serial.print(F(" I: ")); Serial.print(iStr);
  Serial.print(F(" Avg: ")); Serial.print(avgStr);
  Serial.print(F(" Energy: ")); Serial.println(energyStr);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("V  "); display.println(vStr);
  
  display.setCursor(0, 10);
  display.print("I  "); display.println(iStr);
  
  display.setCursor(0, 20);
  display.print("P  "); display.println(pStr);

  display.setCursor(0, 30);
  display.print("Ap "); display.println(avgStr);

  // Draw the custom Sigma bitmap for total integrated energy sum
  display.drawBitmap(0, 40, sigma_bitmap, 5, 7, BLACK);
  
  display.setCursor(8, 40);
  display.print("  "); 
  display.println(energyStr);

  display.display();
}