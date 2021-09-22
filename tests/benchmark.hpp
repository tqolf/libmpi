/**
 * Copyright 2021 Ethan.cr.yp.to
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <functional>
#include <any>
#include <unordered_map>

class Benchmark {
  public:
    inline static bool output_json_format = 0;
    Benchmark(const std::string name, unsigned security_bits, size_t chunk_size = 0) : m_name(name), m_security_bits(security_bits), m_chunk_size(chunk_size) {}

    bool run(std::function<bool()> func, double timeout = 0.5)
    {
        uint64_t total_us = static_cast<uint64_t>(timeout * 1000000); // total amount of time that we'll aim to measure a function for.
        unsigned iterations_between_time_checks;

        // checking
        {
            uint64_t start = time_now(), now;
            if (!func()) { return false; }
            now = time_now();

            uint64_t delta = now - start;
            if (delta == 0) {
                iterations_between_time_checks = 250;
            } else {
                // Aim for about 100ms between time checks.
                iterations_between_time_checks = static_cast<unsigned>(100000 / delta);
                if (iterations_between_time_checks > 1000) {
                    iterations_between_time_checks = 1000;
                } else if (iterations_between_time_checks < 1) {
                    iterations_between_time_checks = 1;
                }
            }

            if (timeout < 0.1 && delta != 0) { total_us = delta * 10000; }
        }

        // benchmark
        {
            unsigned done = 0;
            uint64_t start = time_now(), now;
            long long cycles_start = cpucycles(), cycles_now;
            for (;;) {
                for (unsigned i = 0; i < iterations_between_time_checks; i++) {
                    if (!func()) { return false; }
                    done++;
                }
                now = time_now();
                if (now - start > total_us) {
                    cycles_now = cpucycles();
                    break;
                }
            }

            this->calls = done;
            this->microseconds = static_cast<unsigned>(now - start);
            this->cycles = cycles_now - cycles_start;
        }

        return true;
    }

    void print(double coeff = 1.0) const
    {
        double throughput = (static_cast<double>(calls * coeff) / microseconds) * 1000000;
        double performance = static_cast<unsigned>(cycles) / (calls * coeff);

        if (output_json_format) {
            std::cout << "{ \"name\": \"" << m_name << "\""
                      << ", \"security bits\": " << m_security_bits;
            if (m_chunk_size != 0) { std::cout << ", \"chunk size\": " << m_chunk_size; }
            std::cout << ", \"throughput\": " << std::fixed << std::setprecision(2) << throughput;
            std::cout << ", \"performance\": " << std::fixed << std::setprecision(2) << performance;
            // std::cout << ", \"throughput\": \"" << std::fixed << std::setprecision(2) << throughput << " ops/s\"";
            // std::cout << ", \"performance\": \"" << std::fixed << std::setprecision(2) << performance << "
            // cycle/op\"";
            std::cout << " }" << std::endl;
        } else {
            std::cout << m_name;
            if (m_chunk_size != 0) { std::cout << " (" << m_chunk_size << " bytes)"; }
            std::cout << " " << static_cast<unsigned>(calls * coeff) << " operations in " << microseconds << " us (";
            std::cout << std::fixed << std::setprecision(2) << throughput << " ops/s, ";
            std::cout << std::fixed << std::setprecision(2) << performance << " cycle/op)";
            std::cout << std::endl;
        }
    }

    void print_bytes(size_t bytes_per_call, double coeff = 1.0) const
    {
        double throughput = static_cast<double>(bytes_per_call * calls) * coeff / ((1.024 * 1.024) * static_cast<double>(microseconds));
        double performance = static_cast<double>(cycles) / static_cast<double>(static_cast<double>(bytes_per_call * calls) * coeff);

        if (output_json_format) {
            std::cout << "{ \"name\": \"" << m_name << "\""
                      << ", \"security bits\": " << m_security_bits;
            if (m_chunk_size != 0) { std::cout << ", \"chunk size\": " << m_chunk_size; }
            std::cout << ", \"throughput\": " << std::fixed << std::setprecision(2) << throughput;
            std::cout << ", \"performance\": " << std::fixed << std::setprecision(2) << performance;
            // std::cout << ", \"throughput\": \"" << std::fixed << std::setprecision(2) << throughput << " MB/s\"";
            // std::cout << ", \"performance\": \"" << std::fixed << std::setprecision(2) << performance << "
            // cycle/byte\"";
            std::cout << " }" << std::endl;
        } else {
            std::cout << m_name;
            if (m_chunk_size != 0) { std::cout << " (" << m_chunk_size << " bytes)"; }
            std::cout << " ";
            std::cout << static_cast<unsigned>(calls * coeff) << " operations in " << microseconds << " us (";
            std::cout << std::fixed << std::setprecision(2) << throughput << " MB/s, ";
            std::cout << std::fixed << std::setprecision(2) << performance << " cycle/byte)";
            std::cout << std::endl;
        }
    }

  private:
    // performance data
    unsigned calls;        // the number of function calls done in the time period.
    unsigned microseconds; // microseconds that elapsed in the time period.
    long long cycles;      // cpu cycles that elapsed in the time period.

    // algorithm information
    const std::string m_name;
    unsigned m_security_bits;
    size_t m_chunk_size;

#if defined(_WIN32)
    uint64_t time_now()
    {
        return GetTickCount64() * 1000;
    }
#else
    uint64_t time_now()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        uint64_t ret = ts.tv_sec;
        ret *= 1000000;
        ret += ts.tv_nsec / 1000;
        return ret;
    }
#endif

#if defined(__x86_64) || defined(_M_AMD64) || defined(_M_X64)
    long long cpucycles(void)
    {
        unsigned long long n;
        asm volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax" : "=a"(n)::"%rdx");
        return n;
    }
#elif defined(__x86) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
    long long cpucycles(void)
    {
        long long n;
        asm volatile(".byte 15;.byte 49" : "=A"(n));
        return n;
    }
#elif defined(__aarch64__)
    long long cpucycles(void)
    {
        long long n;
        asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(n));
        return n;
    }
#elif defined(__arm) || defined(__arm__) || defined(_M_ARM)
    long long cpucycles(void)
    {
        static unsigned int readticks(void)
        {
            static int enabled;
            unsigned int r;
            if (!enabled) {
                asm volatile("mrc p15, 0, %0, c15, c12, 0" : "=r"(r));
                asm volatile("mcr p15, 0, %0, c15, c12, 0" ::"r"(r | 1));
                enabled = 1;
            }
            asm volatile("mrc p15, 0, %0, c15, c12, 1" : "=r"(r));
            return r;
        }

        unsigned int cycles32now;
        struct timeval t;
        long long usecnow;
        long long secnow;
        long long usecdiff;
        long long secdiff;
        unsigned long long cycles32diff;
        unsigned long long guesscycles;

        cycles32now = readticks();

        gettimeofday(&t, (struct timezone *)0);
        usecnow = t.tv_usec;
        secnow = t.tv_sec;

        cycles32diff = (unsigned int)(cycles32now - cycles32prev); /* unsigned change in number of cycles mod 2^32 */
        usecdiff = usecnow - usecprev;                             /* signed change in number of usec mod 10^9 */
        secdiff = secnow - secprev;                                /* signed change in number of sec */
        if ((secdiff == 0 && usecdiff < 20000) || (secdiff == 1 && usecdiff < -980000)) return cycles64prev + cycles32diff;

        cycles32prev = cycles32now;
        usecprev = usecnow;
        secprev = secnow;

        usecdiff += 1000000 * (long long)secdiff;
        if (usecdiff > 0 && usecdiff < 30000) {
            /* XXX: assuming cycles do not increase by 2^32 in <0.03 seconds */
            cyclespersec = 1000000 * (unsigned long long)cycles32diff;
            cyclespersec /= usecdiff;
        } else {
            guesscycles = (usecdiff * cyclespersec) / 1000000;
            while (cycles32diff + 2147483648ULL < guesscycles) cycles32diff += 4294967296ULL;
            /* XXX: could do longer-term extrapolation here */
        }

        cycles64prev += cycles32diff;
        return cycles64prev;
    }
#else
#error "Unknown target CPU"
#endif
};

#endif
