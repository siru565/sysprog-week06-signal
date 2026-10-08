/*
 * 3_signal_block2.c — SIGINT 를 잠깐 막았다가 풀어 보기
 *
 * 원본 3_signal_block.c 에서 바꾼 것:
 *   - 막혀 있는 동안 1초마다 sigpending 으로 "SIGINT 가 와서 대기 중인지" 찍어 봄
 *   - 플래그 대신 카운터를 써서, 풀었을 때 핸들러가 몇 번 도는지 확인
 *
 * 흐름:  sigprocmask(SIG_BLOCK) → 작업 → sigprocmask(SIG_SETMASK, &old) 로 복원
 *
 * 컴파일:  gcc -Wall -Wextra -o 3_signal_block2 3_signal_block2.c
 * 실행:    ./3_signal_block2   (5초 동안 Ctrl+C 여러 번)
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define BLOCK_SECS 5

static volatile sig_atomic_t delivered = 0;   /* 핸들러가 실제로 실행된 횟수 */

static void handler(int sig) { (void)sig; delivered++; }

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;    /* 핸들러 등록. 막는 것과는 별개입니다 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    sigset_t block, old, pending;
    sigemptyset(&block);            /* 집합 비우기 (안 하면 쓰레기값) */
    sigaddset(&block, SIGINT);      /* SIGINT 만 넣기 */

    /* SIG_BLOCK: 지금 마스크에 추가로 막습니다. 원래 마스크는 old 에 저장됩니다 */
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("%d초 동안 SIGINT 를 막습니다. Ctrl+C 를 여러 번 눌러 보세요. (PID %d)\n", BLOCK_SECS, getpid());
    for (int i = 1; i <= BLOCK_SECS; i++) {
        sleep(1);
        sigpending(&pending);   /* 지금 대기 중인 시그널 집합 */
        printf("  [%d초] 핸들러 실행 %d회 | SIGINT 대기 중: %s\n",
               i, (int)delivered,
               sigismember(&pending, SIGINT) ? "예" : "아니오");
    }

    printf("막기를 풉니다 (SIG_SETMASK 로 원래 마스크 복원)\n");
    /* 이 호출이 끝나기 전에 대기 중이던 SIGINT 가 전달되고 핸들러가 실행됩니다 */
    sigprocmask(SIG_SETMASK, &old, NULL);

    printf("풀린 뒤 핸들러 실행 횟수: %d회\n", (int)delivered);
    return 0;
}
