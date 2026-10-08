/*
 * alarm.c — 지정한 간격마다, 지정한 횟수만큼 울리는 타이머입니다
 *
 * [원본과 달라진 점]
 *   원본 2_alarm.c 는 alarm(3) 을 한 번만 걸어 입력 시간 제한으로 썼습니다.
 *   alarm 은 한 번만 울리므로, 반복하려면 핸들러가 울린 횟수를 센 뒤
 *   다시 alarm(간격) 을 걸어 다음 알람을 예약해야 합니다.
 *   alarm() 은 async-signal-safe 함수라 핸들러 안에서 불러도 됩니다(man 7 signal-safety).
 *   출처: man 2 alarm, man 2 sigaction, man 2 pause
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o alarm alarm.c
 *   ./alarm <간격초> <반복횟수>      예) ./alarm 2 5
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t ticks = 0;   /* 핸들러가 올리는 "울린 횟수"입니다 */
static unsigned int interval = 0;         /* 알람 간격입니다. 핸들러 등록 전에 한 번 정하고 이후 읽기만 합니다 */
static int repeat = 0;                    /* 반복 횟수입니다. 마찬가지로 읽기만 합니다 */

static void on_alarm(int sig)
{
    (void)sig;
    ticks++;                  /* 울린 횟수를 셉니다 */
    if (ticks < repeat)       /* 아직 남았으면 */
        alarm(interval);      /* 다음 알람을 다시 예약합니다 — 이것이 반복의 핵심입니다 */
}

/* 문자열을 1 이상의 정수로 바꿉니다. 실패하면 -1 을 돌려줍니다. */
static long parse_positive(const char *s)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || *s == '\0' || *end != '\0' || v <= 0)
        return -1;
    return v;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <간격초> <반복횟수>\n  예) %s 2 5\n", argv[0], argv[0]);
        return 1;
    }

    long iv = parse_positive(argv[1]);
    long rp = parse_positive(argv[2]);
    if (iv < 0 || rp < 0 || iv > 3600 || rp > 10000) {
        fprintf(stderr, "간격(1~3600초)과 반복횟수(1~10000)는 양의 정수여야 합니다.\n");
        return 1;
    }
    interval = (unsigned int)iv;
    repeat = (int)rp;

    struct sigaction sa;
    sa.sa_handler = on_alarm;     /* SIGALRM 이 오면 이 함수를 부르게 등록합니다 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;     /* printf 도중 알람이 와도 출력이 중단되지 않게 합니다 (pause 는 이와 무관하게 깨어납니다) */
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("타이머 시작: %u초 간격, %d회 (PID %d)\n", interval, repeat, getpid());
    alarm(interval);   /* 첫 알람을 예약합니다. 이후 예약은 핸들러가 합니다 */

    int shown = 0;     /* main 이 출력한 횟수입니다 */
    while (shown < repeat) {
        if (shown == ticks)   /* 아직 새 알람이 없으면 */
            pause();          /* 빈 while 대신 pause 로 잠들어 CPU 를 쓰지 않습니다 */

        while (shown < ticks) {
            shown++;
            printf("[%d/%d] %u초 경과\n", shown, repeat, interval * (unsigned int)shown);
        }
    }

    printf("타이머 종료\n");
    return 0;
}
