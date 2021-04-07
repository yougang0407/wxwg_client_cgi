#include <sys/signal.h>
#include <sys/time.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <event2/event.h>
#include "./libevent-2.0.21-stable/event-internal.h"

void signal_cb(int fd, short event, void *argc)
{
    struct event_base* base = (struct event_base*)argc;
    struct timeval delay = {2, 0};
    printf("Caught an interrupt signal; exiting cleanly in two seconds....\n");
    event_base_loopexit(base, &delay);
}


void timeout_cb(int fd, short event, void* argc)
{
    printf("timeout\n");
}


int main(int argc, char const *argv[])
{
    struct event_base* base = event_base_new();
    struct event* signal_event = evsignal_new(base, SIGINT, signal_cb, base);
    event_add(signal_event, NULL);

    struct timeval tv = {1, 0};
    struct event* timeout_event = evtimer_new(base, timeout_cb, NULL);
    event_add(timeout_event, &tv);

    event_base_dispatch(base);

    event_free(timeout_event);
    event_free(signal_event);
    event_base_free(base);

    return 0;
}
