#ifndef M61_UTILS_HH
#define M61_UTILS_HH 1
#include <array>
#include <charconv>
#include <chrono>
#include <format>
#include <print>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// random_source.hh
//    Source of randomness, with helpers for different distributions.

class random_source {
public:
    using random_engine_type = std::mt19937_64;

    inline random_source();
    explicit inline random_source(const random_engine_type&);
    random_source(const random_source&) = delete;
    random_source(random_source&&) = delete;
    random_source& operator=(random_source&) = delete;
    random_source& operator=(random_source&&) = delete;

    random_engine_type& engine() { return engine_; }
    inline void seed(random_engine_type::result_type value);
    inline void env_seed(const char* envname = "M61_SEED");

    // - returning bool
    inline bool coin_flip();     // returns true with P = 0.5
    inline bool coin_flip(double probability_of_true);
    // - uniform distribution: select from a list of items or choose within a
    //   range
    template <typename U>
    inline U uniform(std::initializer_list<U> list);
    template <std::integral I>
    inline I uniform(I min, I max);
    template <std::integral I, std::integral J>
    inline std::common_type_t<I, J> uniform(I min, J max);

private:
    random_engine_type engine_;

    static inline random_engine_type randomly_seeded();
};


inline auto random_source::randomly_seeded() -> random_source::random_engine_type {
    std::random_device device;
    std::array<unsigned, random_engine_type::state_size> seed_data;
    for (unsigned i = 0; i != random_engine_type::state_size; ++i) {
        seed_data[i] = device();
    }
    std::seed_seq seq(seed_data.begin(), seed_data.end());
    return random_engine_type(seq);
}

inline random_source::random_source()
    : engine_(randomly_seeded()) {
}

inline random_source::random_source(const random_engine_type& engine)
    : engine_(engine) {
}

inline void random_source::seed(random_engine_type::result_type value) {
    engine_.seed(value);
}

// random_source::env_seed(envname)
//    Seeds this random source based on an environment variable, by default
//    `M61_SEED`. If the variable is unset, the seed is 61, so a test behaves
//    the same way on every run. If the variable is a number, that number is
//    the seed. If it is set but empty (or not a number), the seed is random.

inline void random_source::env_seed(const char* envname) {
    const char* env = envname ? getenv(envname) : nullptr;
    if (!env) {
        env = "61";
    }
    unsigned long long s;
    auto [ptr, ec] = std::from_chars(env, env + strlen(env), s);
    if (ec == std::errc()) {
        seed(s);
    }
    // otherwise `env` did not hold a number, so keep the random seed
}

inline bool random_source::coin_flip() {
    return std::uniform_int_distribution<int>(0, 1)(engine_);
}

inline bool random_source::coin_flip(double probability_of_true) {
    if (probability_of_true <= 0.0) {
        return false;
    } else if (probability_of_true >= 1.0) {
        return true;
    }
    constexpr uint64_t one = uint64_t(1) << 53;
    auto val = std::uniform_int_distribution<uint64_t>(0, one - 1)(engine_);
    return val < static_cast<uint64_t>(probability_of_true * one);
}

template <typename U>
inline U random_source::uniform(std::initializer_list<U> list) {
    assert(list.size() > 0);
    auto idx = std::uniform_int_distribution<size_t>(0, list.size() - 1)(engine_);
    return list.begin()[idx];
}

template <std::integral I>
inline I random_source::uniform(I min, I max) {
    if (min >= max) {
        return min;
    }
    return std::uniform_int_distribution<I>(min, max)(engine_);
}

template <std::integral I, std::integral J>
inline std::common_type_t<I, J> random_source::uniform(I min, J max) {
    if (min >= max) {
        return min;
    }
    using T = std::common_type_t<I, J>;
    return std::uniform_int_distribution<T>(T(min), T(max))(engine_);
}



// run_subtest(f)
//    Runs a subtest by calling `f()` in a separate process. Returns a
//    `subtest_result`, which remembers the subtest’s exit status and
//    everything it printed. Useful for testing behavior that should terminate
//    the program, such as the response to an invalid free. Implemented in
//    subtest.cc.
//
//    Example:
//        void double_free() {
//            void* ptr = m61_malloc(10);
//            m61_free(ptr);
//            m61_free(ptr);
//        }
//        ...
//        subtest_result r = run_subtest(double_free);
//        r.print("double free");   // show what the subtest did
//        assert(r.aborted());

struct subtest_result {
    int status;                    // raw wait status; see `man 2 wait`
    std::string output;            // everything the subtest printed

    bool exited_ok() const;        // did the subtest exit with status 0?
    bool sanitizer_error() const;  // was there a sanitizer error?
    bool aborted() const;          // was it terminated by `abort()`, a
                                   // failed assert, or by `exit(1)`, but
                                   // not by a sanitizer error?
    // did its output contain `text`?
    bool contains(const std::string& text) const;

    // print what happened to the subtest named `what`, followed by
    // everything it printed
    void print(const std::string& what) const;
};

subtest_result run_subtest(std::function<void()> f);


// time_limit(seconds)
//    Kills the program with an error message if it runs longer than
//    `seconds` seconds. The limit is multiplied by 4 when sanitizers are on.
//    Setting the environment variable `M61_TIME_LIMIT` overrides `seconds`;
//    `M61_TIME_LIMIT=0` disables the limit.
// elapsed_time()
//    Returns the number of seconds since the program started.

inline char time_limit_message[100];

inline void time_limit_handler(int) {
    write(STDERR_FILENO, time_limit_message, strlen(time_limit_message));
    _exit(1);
}

inline void time_limit(unsigned seconds) {
#if SAN
    seconds *= 4;
#endif
    if (const char* env = getenv("M61_TIME_LIMIT")) {
        seconds = strtoul(env, nullptr, 0);
    }
    if (seconds != 0) {
        snprintf(time_limit_message, sizeof(time_limit_message),
                 "TIMEOUT: test did not finish within %u seconds\n", seconds);
        signal(SIGALRM, time_limit_handler);
        alarm(seconds);
    }
}

inline const auto program_start_time = std::chrono::steady_clock::now();

inline double elapsed_time() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - program_start_time).count();
}


// fill_pattern(ptr, sz, seed)
//    Fills the `sz` bytes at `ptr` with a byte pattern determined by `seed`.
// check_pattern(ptr, sz, seed)
//    Returns true iff the `sz` bytes at `ptr` still hold that pattern.

inline void fill_pattern(void* ptr, size_t sz, unsigned seed) {
    unsigned char* p = reinterpret_cast<unsigned char*>(ptr);
    unsigned h = seed * 2654435761U;
    for (size_t i = 0; i != sz; ++i) {
        p[i] = (unsigned char) (h + i * ((h >> 8) | 1));
    }
}

inline bool check_pattern(const void* ptr, size_t sz, unsigned seed) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(ptr);
    unsigned h = seed * 2654435761U;
    for (size_t i = 0; i != sz; ++i) {
        if (p[i] != (unsigned char) (h + i * ((h >> 8) | 1))) {
            return false;
        }
    }
    return true;
}


// termcolor
//    Generate fancy terminal colors when the standard error is a terminal.
//    Works with `std::format`, `std::print`, or `std::cerr << COLOR`.

struct termcolor {
    unsigned char d, r, g, b;
    static int wantcolor;

    termcolor()                    : d(0) { }
    termcolor(int c)               : d(c) { }
    termcolor(int r, int g, int b) : d(38), r(r), g(g), b(b) { }

    static termcolor plain, red, green, blue, orange;
};

template <>
struct std::formatter<termcolor> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    auto format(const termcolor& c, std::format_context& ctx) const {
        if (termcolor::wantcolor == 0) {
            // color if both stdout and stderr are terminals; `NO_COLOR`
            // turns color off and `M61_COLOR=1` forces it on
            if (getenv("M61_COLOR")) {
                termcolor::wantcolor = atoi(getenv("M61_COLOR")) ? 1 : -1;
            } else if (getenv("NO_COLOR")) {
                termcolor::wantcolor = -1;
            } else {
                termcolor::wantcolor = isatty(STDOUT_FILENO) && isatty(STDERR_FILENO) ? 1 : -1;
            }
        }
        if (termcolor::wantcolor < 0) {
            return ctx.out();
        } else if (c.d == 0) {
            return std::format_to(ctx.out(), "\033[m");
        } else if (c.d < 38) {
            return std::format_to(ctx.out(), "\033[{}m", c.d);
        }
        return std::format_to(ctx.out(), "\033[38;2;{};{};{}m", c.r, c.g, c.b);
    }
};

template <> inline constexpr bool std::enable_nonlocking_formatter_optimization<termcolor> = true;

inline std::ostream& operator<<(std::ostream& out, termcolor c) {
    return out << std::format("{}", c);
}

#endif
