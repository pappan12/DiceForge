#ifndef DF_CORE_H
#define DF_CORE_H

#include <limits>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

namespace DiceForge
{
#if defined(___linux__)

    typedef int int32_t;           // A signed integer (32 bit)
    typedef __int64_t int64_t;     // A signed integer (64 bit)
    typedef unsigned int uint32_t; // An unsigned integer (32 bit)
    typedef __uint64_t uint64_t;   // An unsigned integer (64 bit)
    typedef __int128_t int128_t;   // A signed integer (128 bit)
    typedef __uint128_t uint128_t; // An unsigned integer (128 bit)

    typedef uint64_t uint_t; // An unsigned integer (64 bit)
    typedef int64_t int_t;   // A signed integer (64 bit)
    typedef double real_t;   // A signed floating point real number (64 bit)

#elif __APPLE__
    typedef int int32_t;                // A signed integer (32 bit)
    typedef long long int int64_t;           // A signed integer (64 bit)
    typedef unsigned int uint32_t;      // An unsigned integer (32 bit)
    typedef unsigned long long int uint64_t; // An unsigned integer (64 bit)
    typedef __int128_t int128_t;        // A signed integer (128 bit)
    typedef __uint128_t uint128_t;      // An unsigned integer (128 bit)

    typedef uint64_t uint_t; // An unsigned integer (64 bit)
    typedef int64_t int_t;   // A signed integer (64 bit)
    typedef double real_t;   // A signed floating point real number (64 bit)

#else

    typedef int int32_t;                     // A signed integer (32 bit)
    typedef long long int int64_t;           // A signed integer (64 bit)
    typedef unsigned int uint32_t;           // An unsigned integer (32 bit)
    typedef unsigned long long int uint64_t; // An unsigned integer (64 bit)
    typedef __int128_t int128_t;             // A signed integer (128 bit)
    typedef __uint128_t uint128_t;           // An unsigned integer (128 bit)

    typedef uint64_t uint_t; // An unsigned integer (64 bit)
    typedef int64_t int_t;   // A signed integer (64 bit)
    typedef double real_t;   // A signed floating point real number (64 bit)

#endif

    /// @brief DiceForge::Generator<T> - A generic class for RNGs
    /// @tparam T datatype of random number generated (RNG implementation specific)
    /// @note Every RNG implemented in DiceForge is derived from this base class.
    /// @note For writing your own RNG it is advisable to use this as the base class for compatibility with other features.
    template <typename T>
    class Generator
    {
    public:
        /// @brief Returns a random integer generated by the RNG
        /// @returns An unsigned integer (usually 32 or 64 bit)
        T next()
        {
            return generate();
        };
        /// @brief Returns a random real between 0 and 1
        /// @returns An floating-point real number (64 bit)
        real_t next_unit()
        {
            real_t x = 1.0;
            while (x == 1.0) {
                x = generate() / real_t(std::numeric_limits<T>().max());
            }
            return x;
        }
        /// @brief Returns a random integer in the specified range
        /// @param min minimum value of the random number (inclusive)
        /// @param max maximum value of the random number (inclusive)
        /// @returns An signed integer (64 bit)
        int64_t next_in_range(T min, T max)
        {
            return (int64_t)floor(next_unit() * (max - min + 1)) + min;
        };
        /// @brief Returns a random real number in the specified range
        /// @param min minimum value of the random number
        /// @param max maximum value of the random number
        /// @returns An signed floating-point real number (64 bit)
        real_t next_in_crange(real_t min, real_t max)
        {
            real_t x = (real_t)max;
            while (x == max) {
                x = (max - min) * next_unit() + min;
            }
            return x;
        };
        /// @brief Re-initializes the RNG with specified seed
        /// @param seed seed provided for initialization
        void reset_seed(T seed)
        {
            reseed(seed);
        }
        /// @brief Returns a random element from the sequence
        /// @param first Iterator of first element (like .begin() of vectors)
        /// @param last Iterator after last element (like .end() of vectors)
        template <typename RandomAccessIterator>
        auto choice(RandomAccessIterator first, RandomAccessIterator last)
        {
            return *(first + next_in_range(0, last - first - 1));
        };

        template <typename RandomAccessIterator1, typename RandomAccessIterator2>
        auto choice(RandomAccessIterator1 first, RandomAccessIterator1 last,
                    RandomAccessIterator2 weights_first, RandomAccessIterator2 weights_last)
        {
            if (last - first != weights_last - weights_first){
                throw std::invalid_argument("Lengths of sequence and weight sequence must be equal!");
            }
            else if (last == first){
                throw std::invalid_argument("Sequence must have non-zero length!");
            }
            auto cumulative_weights = (decltype(&(*weights_first)))malloc(sizeof(*weights_first) * (last - first));
            auto prev = *weights_first;
            prev = 0;
            for (auto it = weights_first; it != weights_last; it++){
                cumulative_weights[it - weights_first] = prev + *it;
                prev = cumulative_weights[it - weights_first];
            }
            auto ans = *(first + (std::upper_bound(cumulative_weights, &(cumulative_weights[last - first - 1]) + 1, next_unit() * cumulative_weights[last - first - 1]) - cumulative_weights));
            free(cumulative_weights);
            return ans;
        };

        /// @brief Shuffles the sequence in place
        /// @param first Iterator of first element (like .begin() of vectors)
        /// @param last Iterator after last element (like .end() of vectors)
        template <typename RandomAccessIterator>
        void shuffle(RandomAccessIterator first, RandomAccessIterator last)
        {
            auto temp = (decltype(&(*first)))malloc(sizeof(*first) * (last - first));
            for (auto it = first; it != last; it++){
                temp[it - first] = *it;
            }
            shuffle_array(temp, last - first);
            for (auto it = first; it!= last; it++){
                *it = temp[it - first];
            }
        };
        /*** Note: These are the only functions to be implemented by the implementation RNG ***/
    private:
        /// @brief Should return a random integer generated by the RNG
        virtual T generate() = 0;
        /// @brief Should initialize the seed for the RNG
        virtual void reseed(T seed) = 0;
        /// @brief Shuffles the array in place
        /// @param arr Pointer to the first element
        /// @param len Length of the array
        template <typename other_T>
        void shuffle_array(other_T arr[], T len) {
            T ind;
            std::vector<other_T> v;
            for (int i = 0; i < len; i++) {
                v.push_back(arr[i]);
            }
            for (int i = 0; i < len; i++) {
                ind = next_in_range(0, len - i - 1);
                arr[i] = v[ind];
                v.erase(v.begin() + ind);
            }
        };
    };

    /// @brief DiceForge::Continuous - A generic class for distributions describing continuous random variables
    class Continuous
    {
    public:
        /// @brief Returns the theoretical variance of the distribution
        virtual real_t variance() const = 0;
        /// @brief Returns the theoretical expectation value of the distribution
        virtual real_t expectation() const = 0;
        /// @brief Returns the minimum possible value of the random variable described by the distribution
        virtual real_t minValue() const = 0;
        /// @brief Returns the maximum possible value of the random variable described by the distribution
        virtual real_t maxValue() const = 0;
        /// @brief Probabiliity density function (pdf) of the distribution
        /// @param x location where the pdf is to be evaluated
        virtual real_t pdf(real_t x) const = 0;
        /// @brief Cumulative distribution function (cdf) of the distribution
        /// @param x location where the cdf is to be evaluated [P(X <= x)]
        virtual real_t cdf(real_t x) const = 0;
    };

    /// @brief DiceForge::Discrete - A generic class for distributions describing discrete random variables
    class Discrete
    {
    public:
        /// @brief Returns the theoretical variance of the distribution
        virtual real_t variance() const = 0;
        /// @brief Returns the theoretical expectation value of the distribution
        virtual real_t expectation() const = 0;
        /// @brief Returns the minimum possible value of the random variable described by the distribution
        virtual int_t minValue() const = 0;
        /// @brief Returns the maximum possible value of the random variable described by the distribution
        virtual int_t maxValue() const = 0;
        /// @brief Probabiliity mass function (pmf) of the distribution
        /// @param x location where the pdf is to be evaluated [P(X = x)]
        virtual real_t pmf(int_t x) const = 0;
        /// @brief Cumulative distribution function (cdf) of the distribution
        /// @param x location where the cdf is to be evaluated [P(X <= x)]
        virtual real_t cdf(int_t x) const = 0;
    };

    /**
     * @brief A struct representing a 128-bit integer.
     * 
     * This struct provides functionality for performing arithmetic operations
     * and modulus operation on 128-bit integers.
     */
    struct BigInt128 {
        uint64_t data[4]; /**< Array to store the components of the 128-bit integer. */
        static const uint64_t _2_32 = 1ULL << 32; /**< Constant representing 2^32. */

        /**
         * @brief Constructs a BigInt128 object with given components.
         * 
         * @param d0 The least significant 64 bits of the BigInt128.
         * @param d1 The second least significant 64 bits of the BigInt128.
         * @param d2 The second most significant 64 bits of the BigInt128.
         * @param d3 The most significant 64 bits of the BigInt128.
         */
        BigInt128(uint64_t d0, uint64_t d1, uint64_t d2, uint64_t d3) {
            data[0] = d0;
            data[1] = d1;
            data[2] = d2;
            data[3] = d3;
        }

        /**
         * @brief Computes the square of the BigInt128 number.
         * 
         * This method calculates the square of the BigInt128 number using
         * an algorithm optimized for 128-bit integers.
         */
        void square() {
            uint64_t product[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // pad with leading zeros

            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    product[i + j] += data[i] * data[j];
                    product[i + j + 1] += product[i + j] >> 32;
                }
            }

            for(int i = 0; i < 4; i++){
                data[i] = product[i] & 0xFFFFFFFF;
            }
        }

        /**
         * @brief Computes the modulus of the BigInt128 number.
         * 
         * This method computes the modulus of the BigInt128 number with
         * the given modulus using extended precision arithmetic.
         * 
         * @param n The modulus to compute the BigInt128 number modulo.
         */
        void mod(uint64_t n){
            uint64_t p1 = n >> 32, p2 = n % _2_32;
            if(data[3] > 0 || data[2] > 0 || data[1] > p1 || (data[1] == p1 && data[0] > p2)){
                uint128_t res = 0;
                for(int i = 3; i >= 0; i--){
                    res = ((res * _2_32) % n + data[i] % n) % n;
                }
                data[3] = 0;
                data[2] = 0;
                data[1] = res >> 32;
                data[0] = res & 0xFFFFFFFF;
            }
        }

        /**
         * @brief Prints the BigInt128 number.
         * 
         * This method prints the components of the BigInt128 number, from
         * most significant to least significant.
         */
        void print() const {
            for (int i = 3; i >= 0; --i) {
                std::cout << data[i] << " ";
            }
        }
    };
}

#endif
