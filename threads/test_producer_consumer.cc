/*
 * Implements producer/consumer communication through a bounded buffer, using
 * locks and condition variables (i.e., a de-facto monitor). The producer places
 * characters from the string "Hello world" into the buffer one character at a
 * time; it must wait if the buffer is full. The consumer pulls characters out
 * of the buffer one at a time and prints them to the screen; it must wait if
 * the buffer is empty. Test your solution with a multi-character buffer and
 * with multiple producers and consumers. Of course, with multiple producers or
 * consumers, the output display will be gibberish.
 */

#include "system.h"
#include "synch.h"

/* the number of producer/consumer pairs created */
static const int n = 3;

static const int ring_size = 5;
static const char *const message = "Hello world\n";

static char ring[ring_size];
static int fullness, consume_at;
static Lock *ring_lock;
static Condition *consumed, *produced;

/* for dynamically generating thread names */
static char **m;

static void produce(int _)
{
    for (const char *c = message; *c; c++) {
        /* yield randomly to simulate scheduler */
        if (rand() < RAND_MAX / 2)
            currentThread->Yield();

        ring_lock->Acquire();

        /* if the buffer is full, wait until an item is consumed */
        while (fullness == ring_size)
            consumed->Wait(ring_lock);

        /* store an item */
        int produce_at = (consume_at + fullness) % ring_size;
        ring[produce_at] = *c;
        fullness++;

        /* now that there's at least one item, wake any waiting consumers */
        produced->Broadcast(ring_lock);

        ring_lock->Release();
    }
}

static void consume(int _)
{
    char c;
    while (true) {
        /* yield randomly to simulate scheduler */
        if (rand() < RAND_MAX / 2)
            currentThread->Yield();

        ring_lock->Acquire();

        /* if the buffer is empty, wait until an item is produced */
        while (!fullness)
            produced->Wait(ring_lock);

        /* retrieve an item */
        c = ring[consume_at];
        fullness--;

        /* move the 'consume' pointer to the next ring position */
        consume_at++;
        consume_at %= ring_size;

        /* now that there's one fewer item, wake any waiting producers */
        consumed->Broadcast(ring_lock);

        ring_lock->Release();

        printf("%c", c);
        if (c == '\n') return;
    }
}

void TestProducerConsumer(void)
{
    printf("Beginning producer/consumer test with %d producers and consumers...\n", n);

    ring_lock = new Lock("ring_lock");
    produced = new Condition("produced");
    consumed = new Condition("consumed");

    fullness = 0;
    consume_at = 0;

    Thread *producers[n], *consumers[n];

    m = new char*[n + n];
    for (int i = 0; i < n; i++) {
        m[i] = new char[20];
        snprintf(m[i], 20, "producer %d", i);
        producers[i] = new Thread(m[i]);
        producers[i]->Fork(produce, 0);

        m[n + i] = new char[20];
        snprintf(m[n + i], 20, "consumer %d", i);
        consumers[i] = new Thread(m[n + i]);
        consumers[i]->Fork(consume, 0);
    }
}
