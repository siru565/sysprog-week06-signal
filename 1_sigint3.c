/*
 * 1_sigint3.c — Ctrl+C 를 세 번 눌러야 종료되는 프로그램
 *
 * 원본 1_sigint.c 는 플래그(0/1) 하나로 "한 번 왔는지"만 봤다.
 * 여기서는 카운터로 바꿔서 몇 번 왔는지 센다.
 * 핸들러는 카운터만 올리고, 출력은 전부 main 에서 한다.
 *
 * 컴파일:  gcc -Wall -Wextra -o 1_sigint3 1_sigint3.c
 * 실행:    ./1_sigint3   (Ctrl+C 세 번)
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define NEED_PRESS 3   /* 종료까지 필요한 Ctrl+C 횟수 */

/* 핸들러가 바꾸는 변수라서 volatile sig_atomic_t 로 선언합니다 */
static volatile sig_atomic_t press_count = 0;

static void handler(int sig)
{
    (void)sig;        /* 안 쓰는 인자 경고 방지 */
    press_count++;    /* 여기서는 이것만 합니다. printf 금지 */
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;      /* SIGINT 오면 handler 실행 */
    sigemptyset(&sa.sa_mask);     /* 핸들러 중에 추가로 막을 시그널 없음 */
    sa.sa_flags = SA_RESTART;     /* printf 중에 시그널 와도 중단 안 되게 */

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Ctrl+C 를 %d번 누르면 종료합니다 (PID %d)\n", NEED_PRESS, getpid());

    int shown = 0;   /* main 이 지금까지 출력한 횟수 */
    while (shown < NEED_PRESS) {
        if (shown == press_count)   /* 새로 온 시그널 없으면 */
            pause();                /* 시그널 올 때까지 잠듭니다 */

        /* 핸들러가 올려 둔 만큼 따라가면서 출력합니다 */
        while (shown < press_count && shown < NEED_PRESS) {
            shown++;
            if (shown < NEED_PRESS)
                printf("\n[%d/%d] SIGINT 받음. %d번 더 누르면 종료\n",
                       shown, NEED_PRESS, NEED_PRESS - shown);
        }
    }

    printf("\n[%d/%d] SIGINT 받음. 정리하고 종료합니다.\n", NEED_PRESS, NEED_PRESS);
    return 0;
}
