/*
 * 3_signal_block2.c — SIGINT 를 잠깐 막았다가 풀어, 대기하던 시그널이 전달되는지 확인합니다
 *
 * [원본과 달라진 점]
 *   1) 막힌 동안 1초마다 sigpending 으로 "SIGINT 가 이미 와서 대기 중인지"를 출력합니다.
 *   2) 핸들러가 플래그 대신 카운터를 올리게 해서, 막힌 동안 여러 번 눌러도
 *      풀었을 때 몇 번 전달되는지(= 1번) 눈으로 확인할 수 있게 했습니다.
 *   3) 막는 시간을 인자로 바꿀 수 있습니다. (기본 5초)
 *   흐름: sigprocmask(SIG_BLOCK) → 작업 → sigprocmask(SIG_SETMASK, &old) 로 복원
 *   출처: man 2 sigprocmask, man 2 sigpending, man 7 signal
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 3_signal_block2 3_signal_block2.c
 *   ./3_signal_block2 [막을초]      # 막혀 있는 동안 Ctrl+C 를 여러 번 눌러 봅니다
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t delivered = 0;   /* 핸들러가 실제로 실행된 횟수입니다 */

static void handler(int sig) { (void)sig; delivered++; }

int main(int argc, char *argv[])
{
    int secs = 5;
    if (argc > 1) {
        secs = atoi(argv[1]);
        if (secs <= 0 || secs > 60) {
            fprintf(stderr, "막을 시간은 1~60초 사이로 주세요.\n");
            return 1;
        }
    }

    struct sigaction sa;
    sa.sa_handler = handler;    /* SIGINT 가 오면 이 함수를 부르도록 등록합니다 (블록 여부와는 별개입니다) */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    sigset_t block, old, pending;
    sigemptyset(&block);            /* 빈 집합에서 시작합니다 */
    sigaddset(&block, SIGINT);      /* SIGINT 하나만 넣습니다 */

    /* SIG_BLOCK: 지금 마스크에 추가로 막습니다. old 에는 원래 마스크가 저장됩니다. */
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("지금부터 %d초간 SIGINT 를 막습니다. Ctrl+C 를 여러 번 눌러 보세요. (PID %d)\n", secs, getpid());
    for (int i = 1; i <= secs; i++) {
        sleep(1);   /* SIGINT 가 막혀 있으므로 sleep 이 중간에 깨지 않습니다 */
        sigpending(&pending);   /* 지금 대기(pending) 중인 시그널 집합을 얻습니다 */
        printf("  [%d/%d초] 핸들러 실행 %d회 | SIGINT 대기 중: %s\n",
               i, secs, (int)delivered,
               sigismember(&pending, SIGINT) ? "예 (pending)" : "아니오");
    }

    printf("막기를 풉니다 (SIG_SETMASK 로 원래 마스크 복원)\n");
    /* 원래 마스크로 되돌립니다 — 이 호출이 돌아오기 전에 대기 중이던 SIGINT 가 전달됩니다. */
    if (sigprocmask(SIG_SETMASK, &old, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    if (delivered > 0)
        printf("풀린 직후 핸들러 실행 횟수: %d회 → 막힌 동안 여러 번 눌러도 한 번만 전달됩니다.\n", (int)delivered);
    else
        printf("막힌 동안 들어온 SIGINT 가 없었습니다.\n");
    return 0;
}
