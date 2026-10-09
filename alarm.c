/*
 * 2_alarm.c — 정해진 시간 뒤에 시그널을 받는다
 *
 * [핵심 개념]
 *   alarm(n) 은 n 초 뒤에 커널이 SIGALRM 을 보내도록 예약한다.
 *   시간 제한(타임아웃)을 구현하는 가장 간단한 방법이다.
 *   출처: man 2 alarm, man 2 sigaction
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_alarm 2_alarm.c
 *   ./2_alarm      # 3초 안에 뭔가 입력하지 않으면 시간이 끝난다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

static volatile sig_atomic_t timeout = 0; // 핸들러
int count;
int repeat;

static void on_alarm(int sig)
{
    (void)sig;
    timeout += 1;
    if (timeout < repeat)
        alarm(count);
}

int main(int argc, char *argv[])
{
    count = atoi(argv[1]);
    repeat = atoi(argv[2]);

    struct sigaction sa;
    sa.sa_handler = on_alarm;   /* SIGALRM 이 오면 이 함수를 부르게 등록한다 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    alarm(count);   /* count초 뒤 SIGALRM 예약 */
    printf("알람 시작\n");

    while  (timeout < repeat) {
        pause();
        printf("%d번째 알람 (%d초 경과)\n", timeout, timeout * count);
    }

    printf("타이머 종료\n");
    return 0;
}
