#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>

int
main()
{
    const char *fn = "/tmp/screen";
    int fd;
    struct stat st;
    struct timespec *t0 = NULL, *t1 = NULL;
    int kq;
    struct kevent kin, kout;

    if ((fd = open(fn, O_EVTONLY)) < 0) {
        perror("open");
        exit(1);
    }

    // init kqueue
    if ((kq = kqueue()) == -1) {
        perror("kqueue");
        exit(1);
    }

    memset(&kin, 0, sizeof(struct kevent));
    EV_SET(&kin, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, NULL);

    if (kevent(kq, &kin, 1, NULL, 0, NULL) == -1) {
        perror("rego");
        exit(1);
    }

    int c;
    while (true) {
        memset(&kout, 0, sizeof(kout));
        if ((c = kevent(kq, NULL, 0, &kout, 1, NULL)) <= 0)
            continue;
        if (kout.filter == EVFILT_VNODE && kout.fflags & NOTE_WRITE) {
            lseek(fd, 0, SEEK_SET);
            char buf[400]; // should be at most 80x25
            memset(buf, 0, 400);
            read(fd, buf, 400);
            puts(buf);
        }
    }

    close(fd);

    return 0;
}

