# 시스템프로그래밍 6주차 과제 — 시그널 처리

시그널 핸들러를 등록하고, 타이머를 만들고, 시그널을 잠깐 막아 본다.

| 파일 | 원본 | 내용 |
|---|---|---|
| `1_sigint3.c` | `original/1_sigint.c` | Ctrl+C 를 세 번 눌러야 종료 |
| `alarm.c` | `original/2_alarm.c` | `./alarm <간격초> <반복횟수>` 반복 타이머 |
| `3_signal_block2.c` | `original/3_signal_block.c` | SIGINT 를 5초 막았다 풀기 + 관찰 |

## 빌드 / 실행 (WSL2)

```bash
make
./1_sigint3            # Ctrl+C 세 번
./alarm 2 5            # 2초마다 5번
./3_signal_block2      # 5초 동안 Ctrl+C 여러 번
```

## 1. 1_sigint3 — Ctrl+C 세 번에 종료

원본은 `got_sigint` 플래그가 1이 되면 바로 끝났다. 이걸 `press_count` 카운터로 바꿔서 핸들러가 올 때마다 1씩 올리고, main 에서 3이 될 때까지 기다린다.

- 핸들러에서는 `press_count++` 만 한다. printf 는 main 에서.
- `volatile sig_atomic_t` 로 선언. volatile 없으면 컴파일러가 루프 조건을 최적화해서 무한 루프가 될 수 있고, sig_atomic_t 는 읽고 쓰는 게 한 번에 끝나는 타입이라 중간에 끼어들어도 깨진 값을 안 본다.
- main 은 `shown`(출력한 횟수)과 `press_count`(받은 횟수)를 비교해서, 같으면 `pause()` 로 자고, 다르면 따라가면서 출력한다.

실행 결과:
```
$ ./1_sigint3
Ctrl+C 를 3번 누르면 종료합니다 (PID 1234)
^C
[1/3] SIGINT 받음. 2번 더 누르면 종료
^C
[2/3] SIGINT 받음. 1번 더 누르면 종료
^C
[3/3] SIGINT 받음. 정리하고 종료합니다.
```
![1_sigint3](screenshots/1_sigint3.png)

## 2. alarm — N초마다 M번 울리는 타이머

alarm() 은 한 번만 울린다. 그래서 핸들러에서 `ticks++` 한 다음 아직 횟수가 남았으면 `alarm(interval)` 을 다시 걸어 준다. alarm 은 핸들러 안에서 불러도 되는 함수다 (man 7 signal-safety 목록에 있음).

- 간격과 횟수는 인자로 받는다. 안 주거나 0 이하면 사용법 출력하고 종료.
- 기다릴 때 빈 while 대신 `pause()` 를 쓴다. 빈 while 은 CPU 를 100% 먹는다.

실행 결과:
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

## 3. 3_signal_block2 — 막았다 풀기

```
sigprocmask(SIG_BLOCK, &block, &old)   // 막기. 원래 마스크는 old 에
  5초 동안 sleep(1) + sigpending 으로 대기 여부 출력
sigprocmask(SIG_SETMASK, &old, NULL)   // old 로 되돌리기
```

SIG_UNBLOCK 이 아니라 SIG_SETMASK 로 되돌리는 이유: UNBLOCK 은 SIGINT 만 빼는 건데, 원래부터 막혀 있던 다른 시그널이 있었다면 그건 그대로 둬야 하니까 old 로 통째로 되돌리는 게 정확하다.

관찰 결과 (막힌 5초 동안 Ctrl+C 를 3번 누름):
```
$ ./3_signal_block2
5초 동안 SIGINT 를 막습니다. Ctrl+C 를 여러 번 눌러 보세요. (PID 1236)
^C  [1초] 핸들러 실행 0회 | SIGINT 대기 중: 예
^C  [2초] 핸들러 실행 0회 | SIGINT 대기 중: 예
^C  [3초] 핸들러 실행 0회 | SIGINT 대기 중: 예
  [4초] 핸들러 실행 0회 | SIGINT 대기 중: 예
  [5초] 핸들러 실행 0회 | SIGINT 대기 중: 예
막기를 풉니다 (SIG_SETMASK 로 원래 마스크 복원)
풀린 뒤 핸들러 실행 횟수: 1회
```
![3_signal_block2](screenshots/3_signal_block2.png)

| 확인한 것 | 결과 |
|---|---|
| 막힌 동안 Ctrl+C 누르면 죽나 | 안 죽는다. 핸들러도 안 돈다 (0회) |
| 그 시그널은 버려지나 | 아니다. sigpending 에 "예"로 남아 있다 |
| 언제 전달되나 | sigprocmask(SIG_SETMASK) 호출할 때 |
| 3번 눌렀는데 몇 번 전달되나 | 1번. pending 은 비트 하나라 큐에 안 쌓인다 |

---

## AI 활용 내용

### 사용한 도구와 프롬프트
Claude 를 사용했다. 과제 폴더(code06)를 연결하고 아래 프롬프트를 줬다.

> 시그널 핸들러를 등록하고, 타이머를 만들고, 시그널을 잠깐 막아 본다.
> 1. Ctrl+C 를 세 번 눌러야 종료 — 1_sigint.c 를 변형. 누른 횟수를 세어 세 번째에 정리 메시지를 내고 끝낸다. 핸들러에서는 플래그(카운터)만 건드린다. 타입은 volatile sig_atomic_t. printf 는 main 에서 한다.
> 2. 타이머 만들기 — 2_alarm.c 를 변형. ./alarm <간격초> <반복횟수> 예) ./alarm 2 5. alarm 은 한 번만 울리므로, 핸들러가 처리한 뒤 다시 alarm(간격) 을 걸어야 반복된다.
> 3. 시그널 막아 보기 — 3_signal_block.c 를 변형. 막힌 구간에서 Ctrl+C 를 누르고, 풀어 줄 때 전달되는지 확인해 적는다. sigprocmask(SIG_BLOCK, ...) → 작업 → SIG_SETMASK 로 복원.
>
> 참고자료와 강의 ppt를 보고 참고해서 만들어줘.

그 뒤 "과제 설명이랑 코드 설명이랑 내가 해야 되는 부분을 하나하나 설명해 줘, 이번 주차를 아직 이해를 잘 못 했어" 라고 물어서 개념 설명을 들었고, README 정리와 문체 다듬기도 AI 의 도움을 받았다.

### AI 가 한 것
- 원본 세 파일과 강의 pptx 를 읽고 세 프로그램의 **초안** 작성
- Makefile, .gitignore, README 초안과 정리
- 시그널 개념(핸들러 규칙, volatile sig_atomic_t, pending, SETMASK vs UNBLOCK) 설명

### 내가 한 것 / 고친 것

<!-- 아래 표는 직접 고친 뒤에 채운다. -->

| 항목 | AI 초안 | 내가 바꾼 것 | 이유 / 배운 것 |
|---|---|---|---|
| | | | |

### 이해한 내용 (내 말로)

<!-- 아래 질문에 직접 답을 쓴다. -->

1. 핸들러 안에서 printf 를 쓰면 안 되는 이유:
2. volatile 과 sig_atomic_t 가 각각 하는 일:
3. alarm.c 에서 핸들러 안에서 alarm() 을 불러도 되는 이유:
4. SIG_UNBLOCK 대신 SIG_SETMASK 로 되돌리는 이유:
5. 막힌 동안 3번 눌렀는데 핸들러가 1번만 실행된 이유:
