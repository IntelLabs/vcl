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
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


std::string get_name(std::string &fullpath){
    size_t dir_ext = fullpath.find_last_of("/");
    std::string name = fullpath.substr(dir_ext + 1);
    size_t pos = name.find_last_of(".");
    name = name.substr(0, pos);
    return name;
}

std::string get_dir(std::string &fullpath) {
    size_t dir_ext = fullpath.find_last_of("/");
    std::string name = fullpath.substr(dir_ext + 1); 
    return name;   
}


void write_raw(std::string &output_dir, std::string &image_dir,
    std::string &image,
    std::string &new_file,
    int &height, int &width)
{
    std::string raw_outdir = output_dir + "image_results/raw/";

    // Read the image in 
    std::vector<float> time;
    cv::Mat cv_img = cv::imread(image_dir + image, cv::IMREAD_ANYCOLOR);
    
    // get height and width
    height = cv_img.rows;
    width = cv_img.cols;

    std::string basename = get_name(image);

    // Write the raw CV Mat
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    new_file = raw_outdir + basename + ".raw";
    std::ofstream outfile(new_file.c_str(), std::ios::out|std::ios::binary);

    int buffer_size = height * width * 3;
    outfile.write((const char*)cv_img.data, buffer_size);
    outfile.close();
}

void write_raw_compressed(std::string &output_dir, std::string &image_dir,
    std::string &image,
    std::string &new_file,
    int &height, int &width)
{
    std::string raw_outdir = output_dir + "image_results/raw/";

    // Read the image in 
    std::vector<float> time;
    cv::Mat cv_img = cv::imread(image_dir + image, cv::IMREAD_ANYCOLOR);
    
    // get height and width
    height = cv_img.rows;
    width = cv_img.cols;

    std::string basename = get_name(image);

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    new_file = raw_outdir + basename + "_comp.raw";
    std::ofstream outfile(new_file.c_str(), std::ios::out|std::ios::binary);

    int buffer_size = height * width * 3;
    char *comp_buffer = (char *)malloc(buffer_size + 1000000);
    int compressed_size;    
    
    compressed_size = LZ4_compress_default((const char*)cv_img.data, comp_buffer, buffer_size, buffer_size + 1000000);
    outfile.write((const char*)comp_buffer, compressed_size);

    outfile.close();
}

void write_img(std::string &output_dir, std::string &image_dir,
    std::string &image,
    std::string &new_file,
    int &height, int &width, 
    const std::string &ext)
{
    std::string outdir = output_dir + "image_results/" + ext + "/";

    // Read the image in 
    std::vector<float> time;
    cv::Mat cv_img = cv::imread(image_dir + image, cv::IMREAD_ANYCOLOR);
    
    // get height and width
    height = cv_img.rows;
    width = cv_img.cols;

    std::string basename = get_name(image);
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    new_file = outdir + basename + "." + ext;
    cv::imwrite(new_file, cv_img);
}

void write_tdb(std::string &output_dir, std::string &image_dir,
    std::string &image,
    std::string &new_file,
    int &height, int &width)
{
    std::string tdb_outdir = output_dir + "tdb/";

    // Read the image in 
    std::vector<float> time;
    cv::Mat cv_img = cv::imread(image_dir + image, cv::IMREAD_ANYCOLOR);


    // get height and width
    height = cv_img.rows;
    width = cv_img.cols;

    std::string basename = get_name(image);
    VCL::Image tdbimg(cv_img);
    new_file = tdb_outdir + basename + ".tdb";

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    tdbimg.set_compression(VCL::CompressionType::LZ4);
    tdbimg.set_minimum_dimension(4);
    
    tdbimg.store(new_file, VCL::TDB);
}

void write(std::string &file_type, 
    std::string &output_dir, std::string &image_dir, 
    std::string &image)
{
    std::string new_file;
    int height;
    int width;

    std::string pixels = get_dir(image_dir);

    image_dir = image_dir + "/";

    if ( file_type == "raw" )
        write_raw(output_dir, image_dir, image, new_file, height, width);
    else if ( file_type == "raw_compressed" )
        write_raw_compressed(output_dir, image_dir, image, new_file, height, width);
    else if ( file_type == "png" )
        write_img(output_dir, image_dir, image, new_file, height, width, "png");
    else if ( file_type == "jpg" )
        write_img(output_dir, image_dir, image, new_file, height, width, "jpg");
    else if ( file_type == "tif" )
        write_img(output_dir, image_dir, image, new_file, height, width, "tif");
    else if ( file_type == "tdb" )
        write_tdb(output_dir, image_dir, image, new_file, height, width);
    else {
        std::cout << "File type is not supported with this operation,";
        std::cout <<" please enter 'raw', 'raw_compressed' 'png', 'jpg', 'tif', or 'tdb' \n";
    }

    std::string img_name = get_name(image);
    std::string filename = pixels + "_" + img_name + "_write_" + file_type + ".txt";
    std::ofstream outfile (filename);

    outfile << new_file << "\n";
    outfile << height << "\n";
    outfile << width << "\n";

    outfile.close();    
    std::system("sync");
}





void read_raw(std::string &filename,
    int height, int width)
{
    struct  stat stat_buff;
    size_t  raw_size;

    if (stat(filename.c_str(), &stat_buff) == 0)
        raw_size = stat_buff.st_size;
    
    // Read the raw CV Mat into a CV Mat
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    
    cv::Mat raw_mat(height, width, CV_8UC3);  

    unsigned char* input_buf = new unsigned char[raw_size];
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    file.read((char*)input_buf, raw_size);

    memcpy(raw_mat.data, input_buf, raw_size);

    delete [] input_buf;
}

void read_raw_compressed(std::string &filename,
    int height, int width)
{
    struct  stat stat_buff;

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    size_t comp_size;
    if (stat(filename.c_str(), &stat_buff) == 0)
        comp_size = stat_buff.st_size;

    int decompressed_size = 0;
    char* decompressed_buf = new char[53000000];

    cv::Mat comp_mat(height, width, CV_8UC3);  

    unsigned char* raw_compressed = new unsigned char[comp_size];
    std::ifstream cfile(filename.c_str(), std::ios::in | std::ios::binary);
    cfile.read((char*)raw_compressed, comp_size);

    decompressed_size = LZ4_decompress_safe((const char*) raw_compressed, decompressed_buf, comp_size, 53000000);
    memcpy(comp_mat.data, raw_compressed, decompressed_size);

    delete [] raw_compressed;
    delete [] decompressed_buf;
}

void read_img(std::string &filename)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    cv::Mat cv_img = cv::imread(filename, cv::IMREAD_ANYCOLOR);
}

void read_tdb_buffer(std::string &filename)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image tdb_img(filename);

    int size = tdb_img.get_raw_data_size();
    unsigned char* buffer = new unsigned char[size];

    tdb_img.get_raw_data(buffer, size);

    delete [] buffer;
}

void read_tdb_mat(std::string &filename)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image tdb_img(filename);

    cv::Mat tdbmat = tdb_img.get_cvmat();
}

void read(std::string &file_type,
    std::string &image_data)
{
    std::string filename;
    int height;
    int width;

    std::ifstream datafile (image_data);

    getline(datafile, filename);
    std::string dim;
    getline(datafile, dim);
    height = atoi(dim.c_str());
    getline(datafile, dim);
    width = atoi(dim.c_str());

    if ( file_type == "raw" )
        read_raw(filename, height, width);
    else if ( file_type == "raw_compressed" )
        read_raw_compressed(filename, height, width);
    else if ( file_type == "png" || file_type == "jpg" || file_type == "tif" )
        read_img(filename);
    else if ( file_type == "tdb_buffer" )
        read_tdb_buffer(filename);
    else if ( file_type == "tdb_mat" )
        read_tdb_mat(filename);
    else {
        std::cout << "File type is not supported with this operation,";
        std::cout <<" please enter 'raw', 'raw_compressed' 'png', 'jpg', 'tif', tdb_buffer', or 'tdb_mat'\n";
    }
    std::system("sync");
}



void crop_raw(std::string &filename, 
    int height, int width,
    int start_x, int start_y)
{
    int h = (int)(height / 6.0);
    int w = (int)(width / 6.0);

    // Crop the raw CV Mat
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    struct  stat stat_buff;
    size_t  raw_size;

    if (stat(filename.c_str(), &stat_buff) == 0)
        raw_size = stat_buff.st_size;

    cv::Mat raw_mat(height, width, CV_8UC3);  

    unsigned char* input_buf = new unsigned char[raw_size];
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    file.read((char*)input_buf, raw_size);

    memcpy(raw_mat.data, input_buf, raw_size);
    cv::Mat cropraw(raw_mat, VCL::Rectangle(start_x, start_y, w, h));

    delete [] input_buf;
}

void crop_img(std::string &file, 
    int height, int width,
    int start_x, int start_y)
{
    int h = (int)(height / 6.0);
    int w = (int)(width / 6.0);

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    cv::Mat img = cv::imread(file, cv::IMREAD_ANYCOLOR);
    cv::Mat croppng(img, VCL::Rectangle(start_x, start_y, w, h));
}

void crop_tdb_buffer(std::string &file, 
    int height, int width,
    int start_x, int start_y)
{
    int h = (int)(height / 6.0);
    int w = (int)(width / 6.0);

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image tdb_img(file);
    tdb_img.crop(VCL::Rectangle(start_x, start_y, w, h));
    int size = h * w * 3;
    unsigned char* raw_buffer = new unsigned char[size];

    tdb_img.get_raw_data(raw_buffer, size);
}

void crop_tdb_mat(std::string &file, 
    int height, int width,
    int start_x, int start_y)
{
    int h = (int)(height / 6.0);
    int w = (int)(width / 6.0);

    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image img(file);
    img.crop(VCL::Rectangle(start_x, start_y, w, h));
    cv::Mat crop_mat = img.get_cvmat();
}

void crop(std::string &file_type, std::string &image_data,
    int start_x, int start_y)
{
    std::string file;
    int height;
    int width;

    std::ifstream datafile (image_data);

    getline(datafile, file);
    std::string dim;
    getline(datafile, dim);
    height = atoi(dim.c_str());
    getline(datafile, dim);
    width = atoi(dim.c_str());

    if ( file_type == "raw" )
        crop_raw(file, height, width, start_x, start_y);
    else if ( file_type == "png" || file_type == "jpg" || file_type == "tif" )
        crop_img(file, height, width, start_x, start_y);
    else if ( file_type == "tdb_buffer" )
        crop_tdb_buffer(file, height, width, start_x, start_y);
    else if ( file_type == "tdb_mat" )
        crop_tdb_mat(file, height, width, start_x, start_y);
    else {
        std::cout << "File type is not supported with this operation,";
        std::cout <<" please enter 'raw', 'png', 'jpg', 'tif', tdb_buffer', or 'tdb_mat'\n";
    }
    std::system("sync");
}




void resize_raw(std::string &filename, 
    int height, int width, 
    int resize_height, int resize_width)
{
    // Resize the raw CV Mat
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    struct  stat stat_buff;
    size_t  raw_size;

    if (stat(filename.c_str(), &stat_buff) == 0)
        raw_size = stat_buff.st_size;

    cv::Mat raw_mat(height, width, CV_8UC3);  

    unsigned char* input_buf = new unsigned char[raw_size];
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    file.read((char*)input_buf, raw_size);

    memcpy(raw_mat.data, input_buf, raw_size);
    cv::Mat raw_resized;
    cv::resize(raw_mat, raw_resized, cv::Size(resize_width, resize_height));

    delete [] input_buf;
}

void resize_img(std::string &file,
    int resize_height, int resize_width)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    cv::Mat img = cv::imread(file, cv::IMREAD_ANYCOLOR);
    cv::Mat img_resized;
    cv::resize(img, img_resized, cv::Size(resize_width, resize_height));
}

void resize_tdb_buffer(std::string &file, 
    int resize_height, int resize_width)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image tdb_img(file);
    tdb_img.resize(resize_height, resize_width);
    int size = resize_height * resize_width * 3;
    unsigned char* raw_buffer = new unsigned char[size];

    tdb_img.get_raw_data(raw_buffer, size);

    delete [] raw_buffer;
}

void resize_tdb_mat(std::string &file, 
    int resize_height, int resize_width)
{
    std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
    VCL::Image img(file);
    img.resize(resize_height, resize_width);
    cv::Mat crop_mat = img.get_cvmat();
}

void resize(std::string &file_type, std::string &image_data,
    int resize_height, int resize_width)
{
    std::string file;
    int height;
    int width;

    std::ifstream datafile (image_data);

    getline(datafile, file);
    std::string dim;
    getline(datafile, dim);
    height = atoi(dim.c_str());
    getline(datafile, dim);
    width = atoi(dim.c_str());

    if ( file_type == "raw" )
        resize_raw(file, height, width, resize_height, resize_width);
    else if ( file_type == "png" || file_type == "jpg" || file_type == "tif" )
        resize_img(file, resize_height, resize_width);
    else if ( file_type == "tdb_buffer" )
        resize_tdb_buffer(file, resize_height, resize_width);
    else if ( file_type == "tdb_mat" )
        resize_tdb_mat(file, resize_height, resize_width);
    else {
        std::cout << "File type is not supported with this operation,";
        std::cout <<" please enter 'raw', 'png', 'jpg', 'tdb_buffer', or 'tdb_mat'\n";
    }
    std::system("sync");
}


int main(int argc, char** argv )
{
    if ( argc < 4 )
    {
        printf("Usage: operation, file_type, output directory, image to use \n");
        printf("  if operation = write: image directory \n");
        printf("  if operation = read: image to use, image height, image width \n");
        printf("  if operation = crop: image to use, image height, image width, start_x, start_y \n");
        printf("  if operation = resize: image to use, image height, image width, new_width, new_height \n");
        return -1;
    }

    std::string operation = argv[1];
    std::string file_type = argv[2];
    std::string output_dir = argv[3];
    std::string image = argv[4];


    if ( operation == "write" ) {
        std::string image_dir = argv[5];            
        write(file_type, output_dir, image_dir, image);
    }
    else if ( operation == "read" ) {
        read(file_type, image);
    }
    else if ( operation == "crop" ) {
        int start_x = atoi(argv[5]);
        int start_y = atoi(argv[6]);

        crop(file_type, image, start_x, start_y);
    }
    else if ( operation == "resize" ) {
        int resize_height = atoi(argv[5]);
        int resize_width = atoi(argv[6]);

        resize(file_type, image, resize_height, resize_width);
    }
    else
        std::cout << "Not a supported operation, please enter 'write', 'read', 'crop', or 'resize'\n";

  return 0;
}
