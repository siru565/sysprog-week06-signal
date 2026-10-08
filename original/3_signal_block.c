/*
 * 3_signal_block.c — 잠깐 시그널을 막아 둔다
 *
 * [핵심 개념]
 *   중요한 작업 도중에 시그널이 끼어들면 안 될 때가 있다.
 *   sigprocmask 로 특정 시그널을 "블록"하면 그동안 오는 시그널은
 *   버려지지 않고 대기(pending)했다가, 풀어 주는 순간 전달된다.
 *   출처: man 2 sigprocmask
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 3_signal_block 3_signal_block.c
 *   ./3_signal_block      # 막혀 있는 5초 동안 Ctrl+C 를 눌러 본다
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t got = 0;

static void handler(int sig) { (void)sig; got = 1; }

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handler;   /* SIGINT 가 오면 이 함수를 부르도록 등록 (블록 여부와는 별개다) */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sigset_t block, old;
    sigemptyset(&block);            /* 빈 집합에서 시작 */
    sigaddset(&block, SIGINT);      /* SIGINT 하나만 넣는다 */

    /* SIG_BLOCK: 지금 마스크에 추가로 막는다. old 에는 원래 마스크가 저장된다. */
    if (sigprocmask(SIG_BLOCK, &block, &old) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("지금부터 5초간 SIGINT 를 막습니다. Ctrl+C 를 눌러 보세요.\n");
    sleep(5);   /* 이 5초 동안 SIGINT 가 와도 핸들러가 즉시 실행되지 않고 커널에 대기(pending) 상태로 쌓인다 */
    printf("5초 경과. 눌렀는지 여부: %s\n", got ? "전달됨" : "아직 대기 중");

    /* 원래 마스크로 되돌린다 — 이 순간 대기 중이던 SIGINT 가 전달된다. */
    sigprocmask(SIG_SETMASK, &old, NULL);   /* 세 번째 인자 NULL: 지금 마스크(=old)는 저장할 필요 없다 */

    printf("막기를 풀었습니다. 대기 중이던 시그널: %s\n", got ? "처리됨" : "없었음");
    return 0;
}
