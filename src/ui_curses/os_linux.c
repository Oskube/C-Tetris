#include <unistd.h> /* readlink() */
#include <sys/time.h> /* gettimeofday() */

#include "os_dependent.h"

int GetExecutablePath(char* buf, unsigned len) {
    int ret = (int)readlink("/proc/self/exe", buf, len);
    //  On error return straight
    if (ret == len || ret < 0) return -1;

    //  Strip name of the executable from the read link
    while (buf[ret] != '/') buf[ret--] = '\0';

    return ret+1;
}

unsigned GetTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    unsigned ret = t.tv_sec*1000 + t.tv_usec/1000;
    return ret;
}
