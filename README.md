# ESP32 Digital Pressure Sensor (I2C) + Web Dashboard

ESP32와 WNK 시리즈 I2C 디지털 압력 센서를 이용한 수압 측정 프로젝트입니다.
압력(mbar)과 온도(°C)를 읽어 시리얼 모니터에 출력하고, **내장 웹 서버**를 통해 브라우저에서 실시간 게이지 대시보드로 확인할 수 있습니다.

## 주요 기능

- **24-bit ADC 데이터 읽기** — 압력(0x06~0x08) 및 온도(0x09~0x0B) 레지스터에서 3바이트 데이터 수신
- **이동 평균 필터** — 10개 샘플 기반 노이즈 제거로 안정적인 출력
- **영점 보정** — 오프셋 설정을 통한 압력 영점 캘리브레이션
- **1초 주기 측정** — 압력/온도 동시 측정 및 시리얼 출력
- **웹 대시보드** — WiFi(STA) 연결 후 브라우저에서 실시간 게이지 표시
  - 압력: 원형 게이지 (그라데이션 아크 + 바늘 애니메이션)
  - 온도: 수은주 온도계 게이지
  - 5초 자동 갱신 (JSON API)
  - 다크 테마 반응형 UI

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

## WiFi 설정

소스 코드 상단의 WiFi SSID와 비밀번호를 수정합니다:

```cpp
const char* ssid     = "YOUR_SSID";       // WiFi SSID 입력
const char* password = "YOUR_PASSWORD";   // WiFi 비밀번호 입력
```

## 센서 설정 파라미터

```cpp
#define SENSOR_I2C_ADDR   0x6D       // I2C 주소
#define PRESSURE_RANGE    2000.0f    // 최대 압력 범위 (kPa)
#define PRESSURE_OFFSET   40.0f      // 영점 오프셋 (mbar)
#define READ_INTERVAL     1000       // 측정 주기 (ms)
#define FILTER_SIZE       10         // 이동 평균 필터 샘플 수
```

## 사용 방법

1. 위 배선표에 따라 ESP32와 압력 센서를 연결합니다.
2. 소스 코드의 `ssid`와 `password`를 실제 WiFi 정보로 수정합니다.
3. Arduino IDE에서 ESP32 보드를 선택하고 `ESP32_DigitalPressure_01.ino`를 업로드합니다.
4. 시리얼 모니터(115200 bps)에서 WiFi 연결 및 IP 주소를 확인합니다.
5. 브라우저에서 `http://<ESP32_IP>/` 에 접속하면 대시보드가 표시됩니다.

## 웹 API

| 엔드포인트 | 설명 |
|-----------|------|
| `GET /` | 대시보드 웹 페이지 |
| `GET /api/data` | 센서 데이터 JSON 응답 |

### JSON 응답 예시

```json
{
  "pressure": 1013.25,
  "pressureAvg": 1012.80,
  "temperature": 25.32,
  "temperatureAvg": 25.28,
  "rawP": 123456,
  "rawT": 654321,
  "ok": true
}
```

## 시리얼 출력 예시

```
================================
WNK I2C Pressure Sensor + Web Dashboard
I2C Addr: 0x6D, Range: 2000 kPa (20000 mbar)
Pressure Offset: 40.0 mbar
Moving Average Filter: 10 samples
================================
Sensor connected.
Connecting to WiFi: MyNetwork... Connected!
IP Address: 192.168.1.100
Open browser: http://192.168.1.100/
Web server started on port 80

P:  1013.25 mbar (avg:  1012.80) | T:  25.32 C (avg:  25.28) | Raw P: 123456, T: 654321
```

## 압력 계산 공식

```
ADC 전압 = 3.3V × (raw_value / 8,388,608)
압력 (kPa) = PRESSURE_RANGE × (ADC전압 - 0.5) / 2.0
압력 (mbar) = 압력(kPa) × 10 + PRESSURE_OFFSET
```

## 개발 환경

- **IDE**: Arduino IDE
- **라이브러리**: Wire.h, WiFi.h, WebServer.h (모두 ESP32 기본 내장)
- **시리얼 통신**: 115200 bps
- **웹서버 포트**: 80

## License

MIT
