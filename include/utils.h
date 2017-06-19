#pragma once

#include <time.h>
#include <vector>

namespace VCL {

    typedef std::vector<unsigned char> cv_buffer;

    /*  *********************** */
    /*          ENUMS           */
    /*  *********************** */
    /**
     *  Determines what kind of compression to use
     */
    enum class CompressionType : int { NOCOMPRESSION = 0,
                    GZIP = 1,
                    ZSTD = 2,
                    LZ4 = 3,
                    BLOSC = 4,
                    BLZ4 = 5,
                    BLZ4HC =  6,
                    BSNAPPY = 7,
                    BZLIB = 8,
                    BZSTD = 9,
                    RLE = 10, };


    static const struct init_rand_t { init_rand_t() { srand(time(NULL)); } } init_rand;

    uint64_t rdrand();

    bool supports_rdrand();

    uint64_t get_int64();

};