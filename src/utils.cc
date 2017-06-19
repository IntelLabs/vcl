#include <cpuid.h>
#include <string>
#include <sstream>

#include "utils.h"
#include "Exception.h"

namespace VCL {

    uint64_t rdrand()
    {
        static const unsigned retry_limit = 10;
        unsigned retries = retry_limit;
        do {
            uint64_t val;
            bool r;
            __asm("rdrand %0; setc %1" : "=r"(val), "=r"(r));
            if (r)
                return val;
        } while (--retries);

        throw VCLException(UndefinedException, "Random number not generated\n");
    }

    bool supports_rdrand()
    {
        const unsigned int flag_rdrand = (1 << 30);

        unsigned int eax, ebx, ecx, edx;
        __cpuid(1, eax, ebx, ecx, edx);

        return ((ecx & flag_rdrand) == flag_rdrand);
    }

    uint64_t combine(uint64_t a, uint64_t b)
    {
        int multiplier = 1;

        while (multiplier <= a) {
            multiplier *= 10;
        }

        return a*multiplier + b;
    }

    uint64_t get_int64()
    {
        if ( supports_rdrand() ) {
            return combine(rdrand(), rdrand());
        }
        else {
            init_rand;

            return combine(rand(), rand());
        }
    }

}