#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <lz4.h>
#include <math.h>
#include <VCL.h>

#include "chrono/Chrono.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


long long int get_folder_size(const std::string &path) {
    std::string cmd("du -sb ");
    cmd.append(path);
    cmd.append(" | cut -f1 2>&1");

    FILE *stream = popen(cmd.c_str(), "r");
    if (stream) {
        const int max_size = 256;
        char readbuf[max_size];
        if (fgets(readbuf, max_size, stream) != NULL) {
            return atoll(readbuf);
        }
        pclose(stream);
    }
    return -1;
}

std::string get_name(std::string &fullpath){
    size_t dir_ext = fullpath.find_last_of("/");
    std::string name = fullpath.substr(dir_ext + 1);
    size_t pos = name.find_last_of(".");
    name = name.substr(0, pos);
    return name;
}

VCL::CompressionType get_compression(int comp)
{
    switch (comp) {
        case 0:
            return VCL::CompressionType::NOCOMPRESSION;
        case 1:
            return VCL::CompressionType::GZIP;
        case 2:
            return VCL::CompressionType::ZSTD;
        case 3:
            return VCL::CompressionType::LZ4;
        case 4:
            return VCL::CompressionType::BLOSC;
        case 5:
            return VCL::CompressionType::BLZ4;
        case 6:
            return VCL::CompressionType::BLZ4HC;
        case 7:
            return VCL::CompressionType::BSNAPPY;
        case 8:
            return VCL::CompressionType::BZLIB;
        case 9:
            return VCL::CompressionType::BZSTD;
        case 10:
            return VCL::CompressionType::RLE;
    }
}

std::string compression_string(int comp)
{
    switch (comp) {
        case 0:
            return "NOCOMPRESSION";
        case 1:
            return "GZIP";
        case 2:
            return "ZSTD";
        case 3:
            return "LZ4";
        case 4:
            return "BLOSC";
        case 5:
            return "BLZ4";
        case 6:
            return "BLZ4HC";
        case 7:
            return "BSNAPPY";
        case 8:
            return "BZLIB";
        case 9:
            return "BZSTD";
        case 10:
            return "RLE";
    }
}

void get_file_sizes( std::vector<std::string> &tiff_files,
    std::vector<std::string> &tdb_files, 
    std::vector<std::vector<long long int>> &sizes)
{
    long long int img_size = 0;
    struct stat stat_buf;

    for ( int i = 0; i < tdb_files.size(); ++i ) {
        std::vector<long long int> size;

        int irc = stat(tiff_files[i].c_str(), &stat_buf);
        if ( irc == 0 )
            img_size = stat_buf.st_size;

        size.push_back(img_size);


        size.push_back(get_folder_size(tdb_files[i]));
        
        sizes.push_back(size);
    }
}

void write(std::string &output_dir, cv::Mat &cv_img,
    std::vector<std::string> &tiff_files,
    std::vector<std::string> &tdb_files,
    std::vector<int> &heights, std::vector<int> &widths,
    std::vector<std::vector<float>> &times,
    int compression, int minimum)
{
    int height = 0;
    int width = 0;

    ChronoCpu tdb_chrono("Write TDB");
    ChronoCpu tif_chrono("Write Tiff");

    std::string tif_outdir = output_dir + "image_results/tiff/";
    std::string tdb_outdir = output_dir + "image_results/tdb/";

    std::vector<float> time;
    
    // get height and width
    height = cv_img.rows;
    width = cv_img.cols;
    heights.push_back(height);
    widths.push_back(width);

    // Determine the tdb image name
    VCL::CompressionType comp = get_compression(compression);
    VCL::Image tdbimg(cv_img);
    std::string tdbname = tdbimg.create_unique(tdb_outdir, VCL::TDB);
    std::string basename = get_name(tdbname);

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    std::string cv_name = tif_outdir + basename + ".tiff";
    tiff_files.push_back(cv_name);
    tif_chrono.tic();
    cv::imwrite(cv_name, cv_img);
    tif_chrono.tac();

    // Write the TDB
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    tdbimg.set_compression(comp);
    tdbimg.set_minimum_dimension(minimum);
    tdb_files.push_back(tdbname);

    tdb_chrono.tic();
    tdbimg.store(tdbname, VCL::TDB);
    tdb_chrono.tac();
   
    time.push_back(tif_chrono.getLastTime_us() / 1000.0);
    time.push_back(tdb_chrono.getLastTime_us() / 1000.0);

    times.push_back(time);
}

void read( std::vector<std::string> &tiff_files,
    std::vector<std::string> &tdb_files,
    std::vector<int> &heights, std::vector<int> &widths,
    std::vector<std::vector<float>> &times)
{
    ChronoCpu read_tdb("Read TDB");
    ChronoCpu tdb_mat("Read TDB to CV");
    ChronoCpu read_tiff("Read TIFF");

    for (int i = 0; i < tdb_files.size(); ++i) {
        // Read the tiff image
        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

        read_tiff.tic();
        cv::Mat tif_img = cv::imread(tiff_files[i], cv::IMREAD_ANYCOLOR);
        read_tiff.tac();

        // Read the TDB image into a buffer
        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

        read_tdb.tic();
        VCL::Image tdb_img(tdb_files[i]);

        int size = tdb_img.get_raw_data_size();
        unsigned char* buffer = new unsigned char[size];

        tdb_img.get_raw_data(buffer, size);
        read_tdb.tac();

        delete [] buffer;

        // Read the TDB image into a mat
        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

        tdb_mat.tic();
        VCL::Image tdbimg(tdb_files[i]);

        cv::Mat tdbmat = tdbimg.get_cvmat();
        tdb_mat.tac();
        times[i].push_back(read_tiff.getLastTime_us() / 1000.0);
        times[i].push_back(read_tdb.getLastTime_us() / 1000.0);
        times[i].push_back(tdb_mat.getLastTime_us() / 1000.0);
    }
}

void crop(std::vector<std::string> &tiff_files,
    std::vector<std::string> &tdb_files,
    std::vector<int> &heights, std::vector<int> &widths,
    std::vector<std::vector<float>> &times)
{
    // ChronoCpu crop_raw("Crop Raw");
    // ChronoCpu crop_png("Crop PNG");
    // ChronoCpu crop_jpg("Crop JPG");
    ChronoCpu crop_tdb("Crop TDB");
    // ChronoCpu raw("Read and Crop Raw");
    // ChronoCpu png("Read and Crop PNG");
    // ChronoCpu jpg("Read and Crop JPG");
    ChronoCpu tdb_mat("Read to Mat and Crop TDB");
    ChronoCpu tiff("Read and Crop TIFF");

    int start_x = 100;
    int start_y = 100;

    // rectangle is x, y, width, height

    for (int i = 0; i < tdb_files.size(); ++i) {
        int height = (int)(heights[i] / 6.0);
        int width = (int)(widths[i] / 6.0);
        // Crop the TDB 
        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
        crop_tdb.tic();
        VCL::Image tdb_img(tdb_files[i]);
        tdb_img.crop(VCL::Rectangle(start_x, start_y, width, height));
        int size = height * width * 3;
        unsigned char* raw_buffer = new unsigned char[size];

        tdb_img.get_raw_data(raw_buffer, size);
        crop_tdb.tac();

        delete [] raw_buffer;

        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
        tiff.tic();
        cv::Mat tifcrop = cv::imread(tiff_files[i], cv::IMREAD_ANYCOLOR);
        cv::Mat croppedtif(tifcrop, VCL::Rectangle(start_x, start_y, width, height));
        tiff.tac();

        // Read to CV and Crop the TDB
        std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
        tdb_mat.tic();
        VCL::Image img(tdb_files[i]);
        img.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat crop_mat = img.get_cvmat();
        tdb_mat.tac();

        times[i].push_back(tiff.getLastTime_us() / 1000.0);
        times[i].push_back(crop_tdb.getLastTime_us() / 1000.0);
        times[i].push_back(tdb_mat.getLastTime_us() / 1000.0);
    }
}

int main(int argc, char** argv )
{
    if ( argc != 6 )
    {
        printf("Usage: image dir, compression type, min tiles, output directory, output file name \n");
        return -1;
    }

    std::string image_dir = argv[1];
    int compression = atoi(argv[2]);
    int min_tiles = atoi(argv[3]);
    std::string output_dir = argv[4];
    std::string output_file = argv[5];

    std::ofstream outfile(output_dir + output_file);

    std::vector<std::string> files;

    std::vector<std::string> tiff_files;
    std::vector<std::string> tdb_files;
    std::vector<int> heights;
    std::vector<int> widths;

    std::vector<std::vector<float>> times;
    std::vector< std::vector<long long int>> sizes;

    VCL::RemoteConnection remote("us-east-1", "AKIAIZZYHO3BGM3I4C2Q", "VrVoLsiUvCCKCw1HlqOcj0E5i8FUZXMlORj4FU9f");
    remote.set_https_proxy("proxy.jf.intel.com", 911);
    remote.start();

    cv::Mat cv_img;

    for (int i = 0; i < 1; ++i)
    {
        std::string frame_num = std::to_string(i);
        int zeros = 4 - frame_num.length();
        std::string frame = "F";
        for (int x = 0; x < zeros; ++x)
            frame += "0";
        frame += frame_num;

        for (int j = 1; j < 11; ++j)
        {
            std::string cam_num = std::to_string(j);
            zeros = 4 - cam_num.length();

            std::string camera = "";
            for (int x = 0; x < zeros; ++x)
                camera += "0";
            camera += cam_num;

            std::string fullpath = image_dir + frame + "/ForReconstruction/" + camera + ".tif";

            std::cout << fullpath << std::endl;
            files.push_back(frame + "_" + camera);

            std::vector<char> data = remote.read(fullpath);
            if ( !data.empty() )
                cv_img = cv::imdecode(cv::Mat(data), cv::IMREAD_ANYCOLOR);

            std::cout << "Writing\n";
            system("sync && echo 3 > /proc/sys/vm/drop_caches");
            write(output_dir, cv_img, tiff_files, tdb_files, heights, widths, times, compression, min_tiles);
        }
    }

    std::cout << "Reading\n";
    system("sync && echo 3 > /proc/sys/vm/drop_caches");
    read(tiff_files, tdb_files, heights, widths, times);
    std::cout << "Cropping\n";
    system("sync && echo 3 > /proc/sys/vm/drop_caches");
    crop(tiff_files, tdb_files, heights, widths, times);
    std::cout << "Getting File Sizes\n";
    system("sync && echo 3 > /proc/sys/vm/drop_caches");
    get_file_sizes(tiff_files, tdb_files, sizes);

    // for (int i = 0; i < 1; ++i)
    // {
    //     for (int j = 1; j < 11; ++j)
    //     {
    //         std::string file = std::to_string(i) + "_" + std::to_string(j) + ".tif";
    //         files.push_back(file);
    //     }
    // }


    // std::cout << "Writing\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // write(output_dir, image_dir, files, tiff_files, tdb_files, heights, widths, times, compression, min_tiles);
    // std::cout << "Reading\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // read(tiff_files, tdb_files, heights, widths, times);
    // std::cout << "Cropping\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // crop(tiff_files, tdb_files, heights, widths, times);
    // std::cout << "Getting File Sizes\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // get_file_sizes(tiff_files, tdb_files, sizes);

    std::cout << "Output to file\n";

    outfile << "# Image Name, Compression Type, Num Pixels, Min Tiles, ";
    outfile << "TIFF Size, TDB Size, TIFF Write, TDB Write, ";
    
    outfile << "TIFF Read, TDB Read, TDB Read to Mat, ";
    outfile << "TIFF ROI, TDB ROI, TDB ROI to Mat, \n";

    for (int i = 0; i < times.size(); ++i) {
        outfile << files[i] << ", ";
        outfile << compression_string(compression) << ", ";
        outfile << heights[i] * widths[i] << ", ";
        outfile << min_tiles * min_tiles << ", ";
        for (int k = 0; k < sizes[i].size(); ++k) {
            outfile << sizes[i][k] << ", ";
        }
        for (int j = 0; j < times[i].size(); ++j) {
            outfile << times[i][j] << ", ";
        }
        outfile << std::endl;
    }


  return 0;
}