/*
 * 1_sigint3.c — Ctrl+C(SIGINT)를 세 번 눌러야 종료되는 프로그램입니다
 *
 * [원본과 달라진 점]
 *   원본 1_sigint.c 는 플래그(0/1)로 "한 번 왔는지"만 확인했습니다.
 *   여기서는 플래그 대신 카운터를 두고, 핸들러는 카운터를 1 올리기만 합니다.
 *   몇 번째인지 출력하는 일과 세 번째에 정리하고 끝내는 일은 모두 main 에서 합니다.
 *   출처: man 2 sigaction, man 7 signal-safety
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_sigint3 1_sigint3.c
 *   ./1_sigint3      # Ctrl+C 를 세 번 눌러 봅니다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define NEED_PRESS 3   /* 종료에 필요한 Ctrl+C 횟수입니다 */

/* 핸들러와 main 이 함께 보는 카운터입니다. 핸들러가 언제든 바꿀 수 있으므로 volatile sig_atomic_t 로 선언합니다. */
static volatile sig_atomic_t press_count = 0;

static void handler(int sig)
{
    (void)sig;        /* 인자를 쓰지 않을 때 경고를 막는 관용적 표현입니다 */
    press_count++;    /* 카운터만 올립니다 — printf 같은 함수는 여기서 부르지 않습니다 */
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;      /* SIGINT 가 오면 기본 동작(종료) 대신 이 함수를 부르게 합니다 */
    sigemptyset(&sa.sa_mask);     /* 핸들러 도중 추가로 막을 시그널은 없습니다 */
    sa.sa_flags = SA_RESTART;     /* main 의 printf(write) 가 시그널로 중단되지 않도록 재시작을 켭니다 */

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Ctrl+C 를 %d번 누르면 종료합니다 (PID %d)\n", NEED_PRESS, getpid());

    int shown = 0;   /* main 이 지금까지 화면에 알린 횟수입니다 */
    while (shown < NEED_PRESS) {
        if (shown == press_count)   /* 새로 들어온 시그널이 없으면 */
            pause();                /* 시그널이 올 때까지 잠듭니다(CPU 를 쓰지 않습니다) */

        /* 깨어난 뒤, 핸들러가 올려 둔 횟수만큼 따라잡으며 출력합니다 */
        while (shown < press_count && shown < NEED_PRESS) {
            shown++;
            if (shown < NEED_PRESS)
                printf("\n[%d/%d] SIGINT 를 받았습니다. %d번 더 누르면 종료합니다.\n",
                       shown, NEED_PRESS, NEED_PRESS - shown);
        }
    }

    printf("\n[%d/%d] SIGINT 를 받았습니다. 정리하고 종료합니다.\n", NEED_PRESS, NEED_PRESS);
    return 0;
}
