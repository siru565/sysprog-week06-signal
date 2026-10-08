/*
 * alarm.c — 정해진 간격으로 정해진 횟수만큼 울리는 타이머
 *
 * 원본 2_alarm.c 는 alarm(3) 을 한 번만 걸어서 입력 제한시간으로 썼다.
 * alarm 은 한 번만 울리기 때문에, 반복하려면 핸들러 안에서
 * 다시 alarm(간격) 을 걸어 줘야 한다. 그게 이 프로그램의 핵심이다.
 *
 * 컴파일:  gcc -Wall -Wextra -o alarm alarm.c
 * 실행:    ./alarm <간격초> <반복횟수>     예) ./alarm 2 5
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t ticks = 0;   /* 핸들러가 올리는 "울린 횟수" */
static unsigned int interval = 0;         /* 간격(초). 시작할 때 한 번 정하고 읽기만 함 */
static int repeat = 0;                    /* 반복 횟수. 마찬가지로 읽기만 함 */

static void on_alarm(int sig)
{
    (void)sig;
    ticks++;                  /* 울린 횟수 +1 */
    if (ticks < repeat)       /* 아직 남았으면 */
        alarm(interval);      /* 다음 알람 예약 — 이게 없으면 한 번만 울립니다 */
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <간격초> <반복횟수>   예) %s 2 5\n", argv[0], argv[0]);
        return 1;
    }

    int iv = atoi(argv[1]);
    int rp = atoi(argv[2]);
    if (iv <= 0 || rp <= 0) {
        fprintf(stderr, "간격과 횟수는 1 이상의 정수여야 합니다.\n");
        return 1;
    }
    interval = (unsigned int)iv;
    repeat = rp;

    struct sigaction sa;
    sa.sa_handler = on_alarm;     /* SIGALRM 오면 on_alarm 실행 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("타이머 시작: %u초 간격, %d회 (PID %d)\n", interval, repeat, getpid());
    alarm(interval);   /* 첫 알람. 그 다음부터는 핸들러가 예약합니다 */

    int shown = 0;     /* main 이 출력한 횟수 */
    while (shown < repeat) {
        if (shown == ticks)   /* 새 알람 없으면 */
            pause();          /* 잠들어서 CPU 안 씀 (빈 while 돌리면 100% 먹음) */

        while (shown < ticks) {
            shown++;
            printf("[%d/%d] %u초 경과\n", shown, repeat, interval * (unsigned int)shown);
        }
    }

    printf("타이머 종료\n");
    return 0;
}
