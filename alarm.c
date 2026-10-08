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

static volatile sig_atomic_t timeout = 0;

static void on_alarm(int sig)
{
    (void)sig;
    timeout = 1;
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = on_alarm;   /* SIGALRM 이 오면 이 함수를 부르게 등록한다 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;   /* SA_RESTART 를 안 줬으므로, 이 시그널은 fgets 같은 블로킹 호출을 중단시킨다 */
    sigaction(SIGALRM, &sa, NULL);

    alarm(3);   /* 3초 뒤 SIGALRM 예약 */
    printf("3초 안에 한 줄 입력하세요: ");
    fflush(stdout);   /* 프롬프트를 즉시 보이게 한다(버퍼에 남지 않도록) */

    char buf[64];
    /*
     * 시그널이 오면 읽는 중이던 read/fgets 가 중단되고 NULL 을 준다.
     * 그때 timeout 플래그로 "시간 초과"인지 "진짜 입력 끝"인지 구분한다.
     */
    if (fgets(buf, sizeof buf, stdin) == NULL) {   /* SIGALRM 에 중단됐거나 EOF 를 만난 경우 */
        if (timeout)
            printf("\n시간이 끝났습니다 (SIGALRM).\n");
        else
            printf("\n입력이 끝났습니다.\n");
        return 0;
    }

    alarm(0);   /* 입력을 받았으니 예약을 취소한다 */
    printf("입력받음: %s", buf);
    return 0;
}
