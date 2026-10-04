# HAC API Test

HAC API(`hwanhee1.hac`)를 실제 Geode 의존성으로 사용하는 소비자 테스트 모드입니다.

게임플레이 중 `hac::api::getSnapshot()`을 100ms마다 호출하고, 현재 판정 결과를 화면 우측 상단에 표시합니다.

## 표시 상태

- `HAC: HACK ENABLED`: 현재 금지 기능이 활성화됨
- `HAC: HACK HISTORY`: 현재는 꺼졌지만 이번 시도에서 금지 기능이 관찰됨
- `HAC: CLEAN`: 조회된 공급자에서 현재 금지 기능이 확인되지 않음
- `HAC: UNKNOWN`: 일부 판정 범위가 미확인
- `HAC: NOT PRESENT`: 조회 대상 공급자가 없음
- `HAC: API UNAVAILABLE`: HAC API 호출 실패

## 의존성

`mod.json`에 `hwanhee1.hac >= v1.0.0`을 required dependency로 선언하고, 빌드 시 공개 헤더를 `external/hwanhee1.hac/include/`에서 사용합니다.

테스트 대상 API 모드와 이 테스트 모드를 함께 설치해야 합니다.

## 빌드

Geode SDK 5.8.2와 Geometry Dash 2.2081 기준입니다.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

생성되는 모드 파일:

```
hwanhee1.hac-api-test.geode
```
