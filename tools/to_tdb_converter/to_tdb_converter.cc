#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "VCL.h"
#include "utils.h"

int main(int argc, char const *argv[])
{
    std::string input(argv[1]);
    std::string tdb_dir(argv[2]);

    std::string tdb_name = tdb_dir + std::to_string(VCL::get_int64());

    std::cout << "TDB OUT Name: " << tdb_name << std::endl;

    try{
        VCL::Image img(input);
        img.store(tdb_name, VCL::TDB);
    }
    catch (VCL::Exception e) {
    printf("[Exception] %s at %s:%d\n", e.name, e.file, e.line);
    if (e.errno_val != 0)
        printf("%s: %s\n", e.msg.c_str(), strerror(e.errno_val));
    else if (!e.msg.empty())
        printf("%s\n", e.msg.c_str());
    }

    try{
        VCL::Image img(tdb_name + ".tdb");
        img.resize(440, 440);
        img.store(tdb_name + "_2", VCL::TDB);
    }
    catch (VCL::Exception e) {
    printf("[Exception] %s at %s:%d\n", e.name, e.file, e.line);
    if (e.errno_val != 0)
        printf("%s: %s\n", e.msg.c_str(), strerror(e.errno_val));
    else if (!e.msg.empty())
        printf("%s\n", e.msg.c_str());
    }

    return 0;
}