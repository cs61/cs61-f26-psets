#ifndef M61_UTILS_HH
#define M61_UTILS_HH 1
#include <random>
#include <array>

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

#endif
