# ESP32 Digital Pressure Sensor (I2C)

ESP32와 WNK 시리즈 I2C 디지털 압력 센서를 이용한 수압 측정 프로젝트입니다.
압력(mbar)과 온도(°C)를 읽어 시리얼 모니터에 출력합니다.

## 하드웨어

| 부품 | 사양 |
|------|------|
| MCU | ESP32-WROOM-32UE |
| 센서 | WNK 시리즈 디지털 압력 센서 (WNK21 / WD19 / WNK811 등) |
| 통신 | I2C (400 kHz) |
| 동작 전압 | 3.3V |

## 배선

| ESP32 | 센서 |
|-------|------|
| GPIO21 (SDA) | SDA |
| GPIO22 (SCL) | SCL |
| 3.3V | VCC |
| GND | GND |

## 주요 기능

- **24-bit ADC 데이터 읽기** — 압력(0x06~0x08) 및 온도(0x09~0x0B) 레지스터에서 3바이트 데이터 수신
- **이동 평균 필터** — 10개 샘플 기반 노이즈 제거로 안정적인 출력
- **영점 보정** — 오프셋 설정을 통한 압력 영점 캘리브레이션
- **1초 주기 측정** — 압력/온도 동시 측정 및 시리얼 출력

## 설정 파라미터

```cpp
#define SENSOR_I2C_ADDR   0x6D       // I2C 주소
#define PRESSURE_RANGE    2000.0f    // 최대 압력 범위 (kPa)
#define PRESSURE_OFFSET   40.0f      // 영점 오프셋 (mbar)
#define READ_INTERVAL     1000       // 측정 주기 (ms)
#define FILTER_SIZE       10         // 이동 평균 필터 샘플 수
```

## 시리얼 출력 예시

```
Pressure: 1013.25 mbar (filtered: 1012.80 mbar)
Temperature: 25.32 °C (filtered: 25.28 °C)
```

## 개발 환경

- **IDE**: Arduino IDE
- **라이브러리**: Wire.h (기본 내장)
- **시리얼 통신**: 115200 bps

## 사용 방법

1. 위 배선표에 따라 ESP32와 압력 센서를 연결합니다.
2. Arduino IDE에서 ESP32 보드를 선택합니다.
3. `ESP32_DigitalPressure_01.ino`를 업로드합니다.
4. 시리얼 모니터(115200 bps)에서 측정값을 확인합니다.

## 압력 계산 공식

```
ADC 전압 = 3.3V × (raw_value / 8,388,608)
압력 (kPa) = PRESSURE_RANGE × (ADC전압 - 0.5) / 2.0
압력 (mbar) = 압력(kPa) × 10 - PRESSURE_OFFSET
```

## License

MIT
