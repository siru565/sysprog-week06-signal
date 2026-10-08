/*
 * 1_sigint.c — Ctrl+C(SIGINT)를 직접 받아 처리한다
 *
 * [핵심 개념]
 *   시그널은 커널이 프로세스에게 보내는 짧은 알림이다.
 *   기본 동작(SIGINT = 즉시 종료)을 내 함수로 바꾸는 것이 핸들러 등록이다.
 *   핸들러 안에서는 아무 함수나 부르면 안 된다 — 중간에 끼어들어 실행되기 때문이다.
 *   그래서 플래그만 켜고, 실제 처리는 main 에서 한다. 플래그는 volatile sig_atomic_t 로 둔다.
 *   출처: man 2 sigaction, man 7 signal-safety
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_sigint 1_sigint.c
 *   ./1_sigint      # Ctrl+C 를 눌러 본다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* 핸들러와 main 이 함께 보는 변수. 중간에 바뀔 수 있으므로 volatile 을 붙인다. */
static volatile sig_atomic_t got_sigint = 0;

static void handler(int sig)
{
    (void)sig;        /* 인자를 쓰지 않을 때 경고를 막는 관용적 표현 */
    got_sigint = 1;   /* 플래그만 켠다 — printf 같은 함수는 여기서 부르지 않는다 */
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;   /* SIGINT 가 오면 기본 동작(종료) 대신 이 함수를 부르게 한다 */
    sigemptyset(&sa.sa_mask);   /* 핸들러 도중 추가로 막을 시그널은 없다 */
    sa.sa_flags = 0;   /* 특별한 동작(SA_RESTART 등) 없이 기본값 사용 */

    if (sigaction(SIGINT, &sa, NULL) == -1) {   /* 세 번째 NULL: 이전 설정은 저장하지 않는다 */
        perror("sigaction");
        return 1;
    }

    printf("Ctrl+C 를 누르면 종료합니다 (PID %d)\n", getpid());

    while (!got_sigint)   /* 플래그가 켜질 때까지 기다린다 */
        pause();          /* 시그널이 올 때까지 잠들어 있는다(CPU 를 쓰지 않는다). 시그널이 오면 깨어나 루프 조건을 다시 검사 */

    printf("\nSIGINT 를 받았습니다. 정리하고 종료합니다.\n");
    return 0;
}
