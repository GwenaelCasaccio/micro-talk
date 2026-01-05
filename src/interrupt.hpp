#pragma once
#include <array>
#include <atomic>
#include <csignal>

class InterruptHandling {

  private:
    static std::atomic<uint64_t> signal_event;
    static std::array<std::atomic<int>, 31> signal_count;

    static void handler(int signal) {
        if (signal < 1 || signal > 31)
            return;

        signal_event.fetch_add(1, std::memory_order_release);
        signal_count[signal - 1].fetch_add(1, std::memory_order_release);
    }

  public:
    InterruptHandling() {
        start_signals();
    }

    static void start_signals() {
        for (int signal = 1; signal <= 31; signal++) {
            if (signal == SIGKILL || signal == SIGSTOP)
                continue; // Skip uncatchable signals

            struct sigaction action{};
            action.sa_handler = &handler;
            sigfillset(&action.sa_mask);

            if (sigaction(signal, &action, NULL) != 0) {
                perror("sigaction");
                std::abort();
            }
        }
    }

    static void stop_signals() {
        struct sigaction action{};
        action.sa_handler = SIG_IGN;

        for (int signal = 1; signal <= 31; signal++) {
            if (signal == SIGKILL || signal == SIGSTOP)
                continue;
            if (sigaction(signal, &action, nullptr) != 0) {
                perror("sigaction");
                std::abort();
            }
        }
    }

    static bool has_event() {
        return signal_event.load(std::memory_order_acquire) > 0;
    }
    static int get_count(int sig) {
        if (sig < 1 || sig > 31)
            throw std::runtime_error("Invalid signal");
        return signal_count[sig - 1].load(std::memory_order_acquire);
    }
    static void consumme(int sig) {
        if (sig < 1 || sig > 31)
            throw std::runtime_error("Invalid signal");

        const int expected = signal_count[sig - 1].load(std::memory_order_acquire);
        while (expected > 0) {
            if (signal_count[sig - 1].compare_exchange_weak(
                    expected, expected - 1, std::memory_order_acq_rel, std::memory_order_acquire)) {
                signal_event.fetch_sub(1, std::memory_order_release);
                break;
            }
        }
    }

    static void clear() {
        // Block all signals (they get queued, not discarded)
        sigset_t mask, oldmask;
        sigfillset(&mask);
        sigdelset(&mask, SIGKILL);
        sigdelset(&mask, SIGSTOP);
        sigprocmask(SIG_BLOCK, &mask, &oldmask);

        for (int signal = 1; signal <= 31; signal++) {
            signal_count[signal - 1].store(0, std::memory_order_release);
        }
        signal_event.store(0, std::memory_order_release);

        sigprocmask(SIG_SETMASK, &oldmask, nullptr);
    }
};

std::atomic<uint64_t> InterruptHandling::signal_event{0};
std::array<std::atomic<int>, 31> InterruptHandling::signal_count{0};
