// Copyright 2010-2013 RethinkDB, all rights reserved.
#ifndef ARCH_TIMING_HPP_
#define ARCH_TIMING_HPP_

#include "concurrency/signal.hpp"
#include "arch/timer.hpp"

/* Coroutine function that delays for some number of milliseconds. */

void nap(int64_t ms) THROWS_NOTHING;

/* This variant takes an interruptor, and throws `interrupted_exc_t` if the
interruptor is pulsed before the timeout is up */

void nap(int64_t ms, signal_t *interruptor) THROWS_ONLY(interrupted_exc_t);

class timer_token_t;

/* Construct a `signal_timer_t` to start a one-shot timer. When the timer "rings",
the signal will be pulsed. It is safe to destroy the `signal_timer_t` before the
timer "rings". */

class signal_timer_t : public signal_t, private timer_callback_t {
public:
    explicit signal_timer_t();
    ~signal_timer_t();

    // Starts the timer, cannot be called if the timer is already running
    void start(int64_t ms);

    // Stops the timer from running
    // Returns true if the timer was canceled, false if there was no timer to cancel
    bool cancel();

    // Returns true if the timer is running or has been pulsed
    bool is_running() const;

private:
    void on_timer();
    timer_token_t *timer;
};

/* Construct a repeating_timer_t to start a repeating timer. It will call its function
when the timer "rings". */

class repeating_timer_callback_t {
public:
    virtual void on_ring() = 0;
protected:
    virtual ~repeating_timer_callback_t() { }
};

class repeating_timer_t : private timer_callback_t {
public:
    repeating_timer_t(int64_t frequency_ms, repeating_timer_callback_t *ringee);
    ~repeating_timer_t();

private:
    void on_timer();
    timer_token_t *timer;
    repeating_timer_callback_t *ringee;

    DISABLE_COPYING(repeating_timer_t);
};

#endif /* ARCH_TIMING_HPP_ */
