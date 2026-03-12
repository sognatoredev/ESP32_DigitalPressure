/**
 * ESP32 Digital Pressure Sensor (WNK Series) I2C Reader
 *
 * MCU: ESP32-WROOM-32UE
 * Sensor: WNK21/WD19/WNK811/WNK80mA/WNK8010 (3.3V I2C)
 * I2C Address: 0x6D (7-bit)
 *
 * Wiring:
 *   ESP32 GPIO21 (SDA) -> Sensor SDA
 *   ESP32 GPIO22 (SCL) -> Sensor SCL
 *   ESP32 3.3V         -> Sensor VCC
 *   ESP32 GND          -> Sensor GND
 */

#include <Wire.h>

// I2C 설정
#define SENSOR_I2C_ADDR   0x6D    // 7-bit address (0xDA >> 1)
#define I2C_SDA_PIN       21
#define I2C_SCL_PIN       22
#define I2C_CLOCK_FREQ    400000  // 400kHz

// 레지스터 주소
#define REG_PRESSURE      0x06    // 압력 데이터 시작 주소 (0x06, 0x07, 0x08)
#define REG_TEMPERATURE   0x09    // 온도 데이터 시작 주소 (0x09, 0x0A, 0x0B)


// 센서 압력 레인지 (kPa) - 실제 센서 사양에 맞게 수정 필요
#define PRESSURE_RANGE    2000.0f

// 단위 변환: 1 kPa = 10 mbar
#define KPA_TO_MBAR       10.0f

// 영점 보정값 (mbar) - 무부하 상태에서의 오프셋 보정
#define PRESSURE_OFFSET   40.0f

// 읽기 주기 (ms)
#define READ_INTERVAL     1000

// 이동평균 필터 샘플 수 (클수록 안정적, 응답 느림)
#define FILTER_SIZE       10

// 이동평균 필터 구조체
typedef struct
{
  float   buffer[FILTER_SIZE];
  uint8_t index;
  uint8_t count;
  float   sum;
} MovingAvgFilter_t;

MovingAvgFilter_t pressureFilter;
MovingAvgFilter_t temperatureFilter;

/**
 * 이동평균 필터 초기화
 */
void filterInit(MovingAvgFilter_t *f)
{
  for (int i = 0; i < FILTER_SIZE; i++)
  {
    f->buffer[i] = 0.0f;
  }
  f->index = 0;
  f->count = 0;
  f->sum   = 0.0f;
}

/**
 * 이동평균 필터에 새 값 추가 후 평균값 반환
 */
float filterUpdate(MovingAvgFilter_t *f, float newValue)
{
  // 버퍼가 가득 찬 경우 가장 오래된 값을 합계에서 제거
  if (f->count >= FILTER_SIZE)
  {
    f->sum -= f->buffer[f->index];
  }
  else
  {
    f->count++;
  }

  // 새 값 저장 및 합계 갱신
  f->buffer[f->index] = newValue;
  f->sum += newValue;

  // 인덱스 순환
  f->index++;
  if (f->index >= FILTER_SIZE)
  {
    f->index = 0;
  }

  return f->sum / (float)f->count;
}

/**
 * 센서에서 24bit 데이터를 읽는다
 * @param regAddr 시작 레지스터 주소
 * @param data 읽은 24bit raw 데이터 (출력)
 * @return true: 성공, false: 실패
 */
bool readSensor24bit(uint8_t regAddr, int32_t *data)
{
  // 레지스터 주소 전송
  Wire.beginTransmission(SENSOR_I2C_ADDR);
  Wire.write(regAddr);
  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  // 3바이트 읽기
  uint8_t bytesRead = Wire.requestFrom((uint8_t)SENSOR_I2C_ADDR, (uint8_t)3);
  if (bytesRead != 3)
  {
    return false;
  }

  uint32_t raw = 0;
  raw  = (uint32_t)Wire.read() << 16;  // 0x06/0x09: MSB
  raw |= (uint32_t)Wire.read() << 8;   // 0x07/0x0A
  raw |= (uint32_t)Wire.read();        // 0x08/0x0B

  // 24bit signed -> 32bit signed 변환
  if (raw & 0x800000)
  {
    *data = (int32_t)raw - 16777216;  // 2^24
  }
  else
  {
    *data = (int32_t)raw;
  }

  return true;
}

/**
 * 압력값 계산 (mbar)
 * ADC = 3.3 * fadc / 8388608.0
 * P(kPa) = Range * (ADC - 0.5) / 2.0
 * P(mbar) = P(kPa) * 10
 */
float calculatePressure(int32_t rawData)
{
  float fadc = (float)rawData;
  float adc = 3.3f * fadc / 8388608.0f;
  float pressure_kPa = PRESSURE_RANGE * (adc - 0.5f) / 2.0f;
  float pressure_mbar = pressure_kPa * KPA_TO_MBAR;
  return pressure_mbar + PRESSURE_OFFSET;
}

/**
 * 온도값 계산 (°C)
 * T = 25.0 + fadc / 65536.0
 */
float calculateTemperature(int32_t rawData)
{
  float fadc = (float)rawData;
  float temperature = 25.0f + fadc / 65536.0f;
  return temperature;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_FREQ);

  // 필터 초기화
  filterInit(&pressureFilter);
  filterInit(&temperatureFilter);

  Serial.println("================================");
  Serial.println("WNK I2C Pressure Sensor Reader");
  Serial.printf("I2C Addr: 0x%02X, Range: %.0f kPa (%.0f mbar)\n", SENSOR_I2C_ADDR, PRESSURE_RANGE, PRESSURE_RANGE * KPA_TO_MBAR);
  Serial.printf("Pressure Offset: %.1f mbar\n", PRESSURE_OFFSET);
  Serial.printf("Moving Average Filter: %d samples\n", FILTER_SIZE);
  Serial.println("================================");

  // 센서 연결 확인
  Wire.beginTransmission(SENSOR_I2C_ADDR);
  if (Wire.endTransmission() == 0)
  {
    Serial.println("Sensor connected.");
  }
  else
  {
    Serial.println("ERROR: Sensor not found! Check wiring.");
  }
  Serial.println();
}

void loop()
{
  int32_t pressureRaw = 0;
  int32_t temperatureRaw = 0;

  bool pOk = readSensor24bit(REG_PRESSURE, &pressureRaw);
  bool tOk = readSensor24bit(REG_TEMPERATURE, &temperatureRaw);

  if (pOk && tOk)
  {
    float pressureNow    = calculatePressure(pressureRaw);
    float temperatureNow = calculateTemperature(temperatureRaw);

    // 이동평균 필터 적용
    float pressureAvg    = filterUpdate(&pressureFilter, pressureNow);
    float temperatureAvg = filterUpdate(&temperatureFilter, temperatureNow);

    Serial.printf("P: %8.2f mbar (avg: %8.2f) | T: %6.2f C (avg: %6.2f) | Raw P: %ld, T: %ld\n",
                  pressureNow, pressureAvg, temperatureNow, temperatureAvg,
                  pressureRaw, temperatureRaw);
  }
  else
  {
    Serial.print("Read error -");
    if (!pOk) Serial.print(" Pressure");
    if (!tOk) Serial.print(" Temperature");
    Serial.println();
  }

  delay(READ_INTERVAL);
}
