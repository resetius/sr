#ifndef MY_SIGNAL_H
#define MY_SIGNAL_H

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void  (*sig_handler_t) (int);
void set_signal(int signo, sig_handler_t hndl);

#ifdef __cplusplus
}
#endif

#endif /* MY_SIGNAL_H */

