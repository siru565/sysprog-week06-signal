# Week 06 — 시그널 처리 과제

시스템프로그래밍 6주차 과제. 시그널 핸들러 등록(`sigaction`), 반복 타이머(`alarm`), 시그널 마스킹(`sigprocmask`) 세 가지를 실습한다.

| 파일 | 원본 | 과제 | 배점 |
|---|---|---|---|
| `1_sigint3.c` | `1_sigint.c` | Ctrl+C 를 세 번 눌러야 종료 | 35 |
| `alarm.c` | `2_alarm.c` | `./alarm <간격초> <반복횟수>` 반복 타이머 | 35 |
| `3_signal_block2.c` | `3_signal_block.c` | SIGINT 막았다 풀기 + 관찰 보고 | 30 |

## 빌드 · 실행 (WSL2 / Linux)

```bash
make            # gcc -Wall -Wextra 로 세 프로그램을 빌드한다 (경고 0개)
./1_sigint3     # Ctrl+C 세 번
./alarm 2 5     # 2초 간격 5회
./3_signal_block2 [막을초]   # 기본 5초, 막힌 동안 Ctrl+C 여러 번
make clean
```

## 1. `1_sigint3.c` — Ctrl+C 세 번에 종료

### 설계
- 원본은 `got_sigint` 플래그(0/1)였다. 이를 **카운터** `press_count` 로 바꾸고, 핸들러는 `press_count++` 한 줄만 수행한다.
- 타입은 `volatile sig_atomic_t`. `volatile` 은 컴파일러가 루프 조건을 상수로 최적화하는 것을 막고, `sig_atomic_t` 는 읽기/쓰기가 원자적임을 보장한다. (강의 1.5절)
- `printf` 는 전부 `main` 에서 한다. `main` 은 `shown`(출력한 횟수)과 `press_count`(받은 횟수)를 비교해, 차이가 없으면 `pause()` 로 잠들고 차이가 생기면 따라잡으며 출력한다.
- `sa_flags = SA_RESTART`: `main` 의 `printf`(내부 `write`) 도중 시그널이 와도 호출이 EINTR 로 깨지지 않게 한다. `pause()` 는 SA_RESTART 와 무관하게 시그널이 오면 반환하므로 루프는 정상 동작한다.

### 실행 결과
```
$ ./1_sigint3
Ctrl+C 를 3번 누르면 종료합니다 (PID 1234)
^C
[1/3] SIGINT 를 받았습니다. 2번 더 누르면 종료합니다.
^C
[2/3] SIGINT 를 받았습니다. 1번 더 누르면 종료합니다.
^C
[3/3] SIGINT 를 받았습니다. 정리하고 종료합니다.
$ echo $?
0
```
![1_sigint3](screenshots/1_sigint3.png)

## 2. `alarm.c` — N초마다 M회 울리는 타이머

### 설계
- `alarm(n)` 은 **한 번만** 울린다(강의 1.7절). 핸들러 `on_alarm` 이 `ticks++` 한 뒤 `ticks < repeat` 이면 `alarm(interval)` 을 다시 걸어 다음 알람을 예약한다.
- `alarm()` 은 `man 7 signal-safety` 의 async-signal-safe 목록에 있으므로 핸들러 안에서 호출해도 된다.
- `interval`, `repeat` 는 핸들러 등록 **전에** 인자에서 한 번 정하고 이후에는 읽기만 하므로 `sig_atomic_t` 일 필요가 없다. 핸들러가 바꾸는 값은 `ticks` 하나뿐이다.
- 빈 `while` 루프 대신 `pause()` 로 기다려 CPU 를 쓰지 않는다. (`time` 으로 재면 `user 0.000s`)
- 인자 검증: `strtol` + `errno` 로 양의 정수만 받고, 아니면 사용법을 출력하고 종료 코드 1.

### 실행 결과
```
$ ./alarm 2 5
타이머 시작: 2초 간격, 5회 (PID 1235)
[1/5] 2초 경과
[2/5] 4초 경과
[3/5] 6초 경과
[4/5] 8초 경과
[5/5] 10초 경과
타이머 종료
```
![alarm](screenshots/alarm.png)

## 3. `3_signal_block2.c` — 시그널 막았다 풀기

### 설계
```
sigprocmask(SIG_BLOCK, &block, &old)   → 막는다, 원래 마스크는 old 에 저장
  sleep(1) × N  +  sigpending 으로 대기 여부 출력
sigprocmask(SIG_SETMASK, &old, NULL)   → 원래 마스크로 "정확히" 복원
```
- `SIG_UNBLOCK` 이 아니라 `SIG_SETMASK` 로 복원하는 이유: `SIG_UNBLOCK` 은 원래부터 막혀 있던 다른 시그널까지 풀어 버릴 수 있다. (강의 1.6절)
- 원본에 두 가지를 더했다. ① 매초 `sigpending()` 으로 SIGINT 가 이미 와서 **pending** 인지 출력한다. ② 플래그 대신 카운터 `delivered` 를 써서, 풀었을 때 핸들러가 **몇 번** 실행되는지 보여 준다.

### 관찰 결과 (막힌 5초 동안 Ctrl+C 를 3번 누름)
```
$ ./3_signal_block2
지금부터 5초간 SIGINT 를 막습니다. Ctrl+C 를 여러 번 눌러 보세요. (PID 1236)
^C  [1/5초] 핸들러 실행 0회 | SIGINT 대기 중: 예 (pending)
^C  [2/5초] 핸들러 실행 0회 | SIGINT 대기 중: 예 (pending)
^C  [3/5초] 핸들러 실행 0회 | SIGINT 대기 중: 예 (pending)
  [4/5초] 핸들러 실행 0회 | SIGINT 대기 중: 예 (pending)
  [5/5초] 핸들러 실행 0회 | SIGINT 대기 중: 예 (pending)
막기를 풉니다 (SIG_SETMASK 로 원래 마스크 복원)
풀린 직후 핸들러 실행 횟수: 1회 → 막힌 동안 여러 번 눌러도 한 번만 전달됩니다.
```
![3_signal_block2](screenshots/3_signal_block2.png)

| 확인한 것 | 결과 |
|---|---|
| 막힌 동안 Ctrl+C 를 누르면 프로그램이 죽는가 | 죽지 않는다. 핸들러도 실행되지 않는다(0회). |
| 막힌 동안의 시그널이 버려지는가 | 버려지지 않는다. `sigpending` 에 SIGINT 가 "예"로 남아 있다. |
| 풀면 언제 전달되는가 | `sigprocmask(SIG_SETMASK, ...)` 가 반환하기 전에 핸들러가 실행된다. |
| 3번 눌렀는데 몇 번 전달되는가 | **1번**. 표준 시그널은 큐에 쌓이지 않고 pending 비트 하나뿐이다. (강의 1.3절) |

## 제출 전 체크리스트 (강의 32쪽)
- [x] `gcc -Wall -Wextra` 경고 없음
- [x] 핸들러 등록에 `signal()` 이 아니라 `sigaction()` 사용
- [x] 핸들러와 공유하는 변수는 모두 `volatile sig_atomic_t`
- [x] 핸들러 안에서는 대입/증가와 `alarm()` 만 호출 (모두 async-signal-safe)

---

## AI 활용 내용 (교과목 AI 정책 ⑦·⑧)

### 사용한 도구
Claude (Anthropic) — Claude 앱에서 로컬 폴더를 연결해 사용.

### 사용한 프롬프트
> 시그널 핸들러를 등록하고, 타이머를 만들고, 시그널을 잠깐 막아 본다.
> 1. Ctrl+C 를 세 번 눌러야 종료 — 1_sigint.c 를 변형. 누른 횟수를 세어 세 번째에 정리 메시지를 내고 끝낸다. 핸들러에서는 플래그(카운터)만 건드린다. 타입은 volatile sig_atomic_t. printf 는 main 에서 한다.
> 2. 타이머 만들기 — 2_alarm.c 를 변형. ./alarm <간격초> <반복횟수> 예) ./alarm 2 5. alarm 은 한 번만 울리므로, 핸들러가 처리한 뒤 다시 alarm(간격) 을 걸어야 반복된다.
> 3. 시그널 막아 보기 — 3_signal_block.c 를 변형. 막힌 구간에서 Ctrl+C 를 누르고, 풀어 줄 때 전달되는지 확인해 적는다. sigprocmask(SIG_BLOCK, ...) → 작업 → SIG_SETMASK 로 복원.
>
> 참고자료와 강의 ppt를 보고 참고해서 만들어줘. (code06 폴더: 1_sigint.c, 2_alarm.c, 3_signal_block.c, week06.pptx)

### AI 가 한 일
- `week06.pptx` 와 원본 세 소스를 읽고, 원본 구조(주석 스타일·sigaction 패턴)를 유지한 채 세 파일을 변형했다.
- `Makefile`, `.gitignore`, 이 README 초안을 작성했다.
- 리눅스 환경에서 `kill -INT` 로 시그널을 보내 세 프로그램의 동작을 검증했다 (위 실행 결과 텍스트는 그 로그를 정리한 것).

### AI 결과물과 본인이 고친 부분의 차이
<!-- ⑧ 개선 사항 제출: AI 가 준 코드를 직접 읽고 바꾼 내용을 적는다. 바꾼 것이 없으면 "없음"이라고 쓰지 말고, 아래 후보 중 하나라도 직접 해 보고 적을 것. -->

| 항목 | AI 가 제시한 것 | 내가 바꾼 것 | 이유 |
|---|---|---|---|
| (예) | | | |

### 내가 원리를 설명할 수 있는지 (⑥ 내재화 원칙)
<!-- 아래 질문에 본인 말로 답을 적는다. 답이 막히면 코드를 다시 읽는다. -->
1. 핸들러에서 `printf` 를 부르면 왜 위험한가? 대신 `main` 에서 출력하려면 어떤 구조가 필요한가?
2. `volatile` 과 `sig_atomic_t` 는 각각 무엇을 보장하는가? 둘 중 하나만 빼면 어떤 일이 생길 수 있는가?
3. `alarm.c` 에서 `alarm()` 을 핸들러 안에서 불러도 되는 근거는 무엇인가?
4. `3_signal_block2.c` 에서 `SIG_UNBLOCK` 대신 `SIG_SETMASK` 를 쓴 이유는?
5. 막힌 동안 Ctrl+C 를 3번 눌렀는데 핸들러가 1번만 실행된 이유는?

### 교차 검증 (④)
- `man 2 sigaction`, `man 2 alarm`, `man 2 sigprocmask`, `man 2 sigpending`, `man 7 signal-safety` 로 다음을 확인했다:
  - `alarm`, `sigaction`, `sigprocmask` 는 async-signal-safe 목록에 있음 → 핸들러 안 `alarm()` 호출 OK
  - 표준 시그널은 큐잉되지 않음(`man 7 signal`, "Queueing and delivery semantics for standard signals")
