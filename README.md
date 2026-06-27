# 🔒 리듬 패턴 인식 도어락 (Rhythm Lock)

> **상대 시간차 기반 정밀 판독 알고리즘을 적용한 ATmega128 임베디드 도어락 시스템** > 본 프로젝트는 부경대학교 정보통신공학전공 마이크로프로세서실습 과목의 텀 프로젝트 결과물입니다.

---

## 📌 1. 프로젝트 개요 & 기획 배경
기존의 디지털 도어락은 고정된 숫자열을 단순히 누르는 방식이라 시간이 지나면 숫자 조합을 망각하거나 타인에게 번호가 노출되기 쉽다는 단점이 있습니다. 

본 시스템은 **학습 시 리듬이나 노래를 곁들이면 암기 효율이 높아지는 현상**에서 아이디어를 얻어 개발되었습니다. 기계적인 숫자가 아닌 사용자가 기억하기 쉬운 본인만의 **'리듬' 자체를 비밀번호로 활용**하여 보안성과 편의성을 동시에 확보한 새로운 형태의 도어락입니다.

---

## 🛠 2. 개발 환경 및 하드웨어 구성
### Development Environment
- **MCU:** ATmega128 (8-bit AVR Microcontroller)
- **Compiler/IDE:** Microchip Studio (Atmel Studio)
- **Language:** Embedded C

### Hardware Component
- **입력 장치:** External Interrupt 스위치 (SW2: 리듬 입력, SW3: 확인, SW4: 설정 변경)
- **출력 장치:** Text LCD (상태 표시), 3색 Active LED (시각 피드백), 능동 부저 (청각 피드백)

---

## ✨ 3. 주요 핵심 기능

### 1) 사용자 지정 가변 리듬 패턴
- 고정된 자릿수의 비밀번호 제한을 탈피하여, 최소 단위부터 **최대 20박자**까지 사용자가 원하는 길이만큼 리듬을 자유롭게 조절하여 저장할 수 있습니다.

### 2) 상대 시간차 기반 정밀 판독 및 오차 보정
- 절대적인 타임스탬프를 비교하지 않고, **첫 박자를 기준점($0\text{ ms}$)**으로 잡은 뒤 이후 입력되는 박자 간의 **상대적 시간 간격(Interval)**을 $5\text{ ms}$ 단위로 정밀 측정합니다.
- 사람이 기계처럼 완벽한 박자를 입력할 수는 없으므로, 설정된 리듬값 대비 **$\pm150\text{ ms}$ ($\pm0.15$초)의 오차 허용 범위(Tolerance)** 알고리즘을 적용하여 실용성을 극대화했습니다.

### 3) 직관적인 시청각 인터페이스 & 예외 처리
- **Text LCD:** 현재 시스템의 모드 및 입력 진행 상황(입력된 박자 수 등)을 실시간 텍스트로 안내합니다.
- **3색 LED & 부저 피드백:** - *입력 중:* 주황색 LED 점등 + 클릭음
  - *인증 성공:* 초록색 LED 점등 + 경쾌한 멜로디
  - *인증 실패:* 빨간색 LED 점등 + 경고음
- **자동 타임아웃 기능:** 입력 도중 일정 시간(입력 모드 3초, 설정 모드 5초) 동안 추가 입력이 없으면 버퍼를 자동 초기화(Reset)하여 오작동을 방지합니다.

---

## ⚙️ 4. 시스템 동작 프로세스 및 구동 화면

### 1단계: 최초 전원 인가 및 비밀번호 설정 (초기 설정 모드)
> 시스템에 처음 전원이 켜지면 저장된 비밀번호가 없으므로 강제로 설정 모드가 실행됩니다.


<p align="center">
  <img width="310" height="413" alt="초기 화면" src="https://github.com/user-attachments/assets/dd749983-5fd4-4ca7-8680-861a5630a5a0" />
  <img width="310" height="413" alt="리듬 설정 화면" src="https://github.com/user-attachments/assets/f4c25a36-c169-47ce-b184-9d623cefc891" />
</p>
<p align="center">
  <em>&lt;초기 화면&gt; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &lt;리듬 설정 화면&gt;</em>
</p>

- **상세 설명:** 시스템 시작 시 LCD에 `Welcome!` 메시지가 출력된 후 자동으로 `SETUP MODE`에 진입합니다. 주황색 LED가 켜진 상태에서 `SW2`를 눌러 원하는 비밀 리듬을 입력한 후, `SW3`를 누르면 입력된 총 박자 수가 저장되며 시스템이 잠금 상태로 전환됩니다.

---

### 2단계: 잠금 대기 및 리듬 비밀번호 입력
> 평상시 문이 잠겨 있는 대기 상태에서 사용자가 리듬을 입력하는 단계입니다.

<p align="center">
  <img width="265" height="354" alt="대기 화면" src="https://github.com/user-attachments/assets/d8a80eca-ba54-49d9-8f5f-362343cbf03f" />
  <img width="265" height="354" alt="한 번 입력한 화면" src="https://github.com/user-attachments/assets/299d5e81-8022-4587-857c-7f332b5ea743" />
  <img width="265" height="354" alt="타임아웃된 화면" src="https://github.com/user-attachments/assets/2d549ba0-e05d-4677-85fb-387d5a958b30" />
</p>
<p align="center">
  <em>&lt;대기 화면&gt; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &lt;한번 입력한 화면&gt; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &lt;타임아웃된 화면&gt;</em>
</p>

- **상세 설명:** LCD에 `Locked [SW2]`가 표시되는 대기 상태에서 `SW2`를 눌러 리듬 입력을 시작합니다. 스위치를 누를 때마다 부저음(클릭음)과 함께 우측 하단에 실시간으로 입력된 박자 수가 표시됩니다. 입력 도중 3초 이상 멈추면 타임아웃 기능에 의해 `RST`(초기화) 메시지가 뜨며 버퍼가 리셋됩니다. 리듬 입력이 모두 끝나면 `SW3`를 눌러 검증을 요청합니다.

---

### 3단계: 리듬 판독 및 결과 피드백 (인증 성공 / 인증 실패)
> 입력된 리듬의 박자 수와 상대적 시간 간격을 기존 비밀번호 데이터와 정밀 비교하여 결과를 출력합니다.

<p align="center">
  <img width="310" height="413" alt="성공 시 화면" src="https://github.com/user-attachments/assets/87812e3a-23bc-4dc8-acf7-5c40a5ec5b14" />
  <img width="310" height="413" alt="실패 시 화면" src="https://github.com/user-attachments/assets/6358bb9f-7a14-4e89-b5cf-8c4b7c58b76f" />
</p>
<p align="center">
  <em>&lt;인증 성공: DOOR OPENED!&gt; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &lt;인증 실패: Access Denied&gt;</em>
</p>

- **상세 설명:** - **인증 성공 (왼쪽):** 저장된 박자 수와 시간차가 오차 허용 범위($\pm150\text{ ms}$) 이내일 경우, 초록색 LED가 켜지고 경쾌한 멜로디와 함께 `DOOR OPENED!` 메시지가 표시되며 문이 열립니다. (5초간 개방 유지)
  - **인증 실패 (오른쪽):** 박자 수가 다르거나 시간 오차가 범위를 초과하면 빨간색 LED와 경고 부저음이 발생하며 `Access Denied`가 표시된 후 대기 상태로 되돌아갑니다.

---

### 4단계: 비밀번호 변경 (설정 모드 재진입)
> 인증에 성공하여 문이 열려 있는 유예 시간 동안 기존 리듬 비밀번호를 변경할 수 있습니다.

*(※ 본 단계는 화면 전환 프로세스로 진행되므로 별도의 독립 구동 사진을 첨부하지 않습니다.)*

- **상세 설명:** 인증 성공 직후 주어지는 5초의 유예 시간 동안 LCD 하단에 `[SW4=Set]` 안내가 표시됩니다. 이 5초 이내에 `SW4` 버튼을 누르면 `mode_record_req` 플래그가 활성화되면서 **1단계의 초기 설정 모드(`SETUP MODE`)**로 다시 진입하여 새로운 리듬 비밀번호를 등록할 수 있습니다. 5초 동안 아무 입력이 없으면 자동으로 다시 잠금 대기 상태로 복귀합니다.
