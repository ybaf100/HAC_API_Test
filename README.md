# HAC API Test

H-Anticheat API(`hwanhee1.hac`)를 **필수 Geode dependency**로 사용하는 별도의 테스트 모드입니다. 다른 모드가 HAC의 공개 API를 호출하는 실제 예제이며, 자체 핵 감지 코드는 없습니다.

메뉴·플레이·일시정지 화면에서 `hac::api::getSnapshot()`을 렌더링 메인 스레드에서 실시간 시계 기준 100ms 간격으로 호출합니다. 상태 텍스트는 화면 우측 상단에 고정되며, 일시정지 버튼을 피하도록 위에서 44 포인트 내려 배치합니다. 기본 글자 크기는 `bigFont.fnt` 0.35배, 긴 문자열은 화면 너비에 맞춰 축소하고 그림자로 대비를 줍니다. 렌더링이 멈추거나 프레임 간격이 100ms보다 길면 다음 렌더 프레임에 갱신됩니다.

## 표시 상태

| 표시 | 뜻 |
| --- | --- |
| `HAC: HACK ENABLED` (빨강) | 현재 금지 기능이 확인됨. 다른 공급자가 미확인이어도 이 경고를 유지 |
| `HAC: INACTIVE` (초록) | HAC가 조회한 범위에서 현재 비활성 판정. 전체 클라이언트가 안전하다는 뜻은 아님 |
| `HAC: UNKNOWN` (노랑) | 조회된 상태나 일부 공급자 범위가 미확인. 정상/안전으로 간주하지 않음 |
| `HAC: NOT PRESENT` (회색) | HAC의 조회 대상 공급자가 로드되지 않음. HAC 모드 자체의 부재와는 다름 |
| `HAC: API UNAVAILABLE` (연한 빨강) | API 호출 실패 또는 API 버전 불일치 |

HAC가 `prohibitedInAttempt=true`를 반환하면 작은 둘째 줄에 `This attempt: hack observed`를 표시합니다. **현재 활성 상태와 과거 시도 이력을 구분**하므로, 기능을 껐을 때 현재 상태는 바뀌어도 시도 이력은 남을 수 있습니다. Accurate Hitbox처럼 HAC가 레벨 진입 상태를 유지하는 기능은 HAC의 현재 판정을 그대로 표시합니다.

이 모드는 기능을 켜거나 끄지 않으며, 게임 플레이·시도·기록 저장을 변경하지 않습니다. 자동 제재 기능도 없습니다.

## 의존성

`mod.json`에 `hwanhee1.hac >= v1.0.0`을 required dependency로 선언하고, 빌드 시 공개 헤더를 `external/hwanhee1.hac/include/`에서 사용합니다.

런타임에 실제 HAC 모드와 이 테스트 모드를 함께 설치해야 합니다. HAC가 없거나 필요한 버전보다 낮으면 Geode가 테스트 모드를 정상 로드하지 않습니다.

HAC는 아직 Geode 인덱스 등록을 전제로 하지 않으므로 빌드에는 고정된 API v2 공개 헤더 두 개를 사용합니다. `setup_geode_mod(... EXTERNALS "hwanhee1.hac:1.0.0")`은 CLI가 미등록 dependency를 다운로드하려 하지 않도록 하는 **빌드 설정**입니다. 런타임의 `required: true` 의존성을 없애지 않습니다. HAC 감지 구현이나 바이너리를 테스트 모드에 포함하지 않습니다.

## 다른 모드에서 호출하는 핵심 코드

```cpp
#include <hwanhee1.hac/include/HAC.hpp>

// 게임 메인 스레드에서 실행
auto result = hac::api::getSnapshot();
if (result.isErr()) {
    // API UNAVAILABLE: 오류를 정상 판정으로 바꾸지 않음
} else {
    auto const& snapshot = result.unwrap();
    if (snapshot.anyHackEnabled || snapshot.state == hac::State::Active) {
        // 현재 금지 기능 감지 -> 빨간 경고 표시
    }
    // Unknown / Inactive / NotPresent는 각각 별도로 표시
    // snapshot.prohibitedInAttempt는 현재 활성 상태와 별도 이력
}
```

현재 상태 하나만 필요하면 `hac::api::getCurrentState()`를 사용할 수도 있습니다. 이 테스트 모드는 상태와 시도 이력을 같은 검사 결과에서 읽기 위해 `getSnapshot()`을 한 번만 호출합니다.

## 설치와 실기기 확인

1. [HAC 원본 빌드](https://github.com/ybaf100/CR-AC/actions)에서 `hwanhee1.hac.geode`를 받아 설치합니다.
2. [테스트 모드 Actions](https://github.com/ybaf100/HAC_API_Test/actions)의 성공한 빌드에서 `HAC-API-Test-v1.0.0` artifact를 받습니다.
3. ZIP 안의 `hwanhee1.hac-api-test.geode`를 설치하고 게임을 재시작합니다.
4. 금지 기능을 켜면 `HACK ENABLED`가 표시되는지 확인합니다.
5. 끄면 현재 판정이 변경되는지 확인합니다. `UNKNOWN`으로 돌아오는 것도 HAC의 판정 범위에 따라 정상일 수 있습니다.
6. 플레이·일시정지·레벨 종료·재진입에서 우측 상단 표시가 유지되고 중복되지 않는지 확인합니다. 이번 시도 이력은 HAC가 초기화한 결과를 따릅니다.

## 빌드

Geode SDK/CLI 설치와 `GEODE_SDK` 환경 변수 설정이 필요합니다. SDK 5.8.2와 Geometry Dash 2.2081 기준입니다.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

생성되는 모드 파일:

```
hwanhee1.hac-api-test.geode
```

GitHub Actions는 Windows, macOS(arm64/x86_64), iOS, Android32, Android64를 빌드한 후 하나의 `.geode`로 결합합니다. 최종 패키지에 필수 HAC dependency와 5개 플랫폼 바이너리가 있는지 검사합니다.

## 검증

```sh
python3 tools/test.py --sanitize
```

이 검사는 상태 표시·API 오류/버전 불일치 처리·현재 상태와 시도 이력 분리, dependency 설정, 원본 공개 헤더의 Git blob 해시를 확인합니다. Geode 이벤트의 실제 모드 간 통신과 화면 배치는 Geometry Dash 실기기 검증이 별도로 필요합니다.

HAC 공개 헤더는 `external/hwanhee1.hac/`에 출처와 MIT 라이선스를 보존했습니다. API 버전이 바뀌면 헤더 두 개와 의존성 기준을 함께 갱신하세요.
