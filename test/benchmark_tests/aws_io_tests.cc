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
#include "tiledb/tiledb.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

bool compare_mat_mat(cv::Mat &cv_img, cv::Mat &img)
{
    int rows = img.rows;
    int columns = img.cols;
    int channels = img.channels();

    if ( img.isContinuous() ) {
        columns *= rows;
        rows = 1;
    }

    for ( int i = 0; i < rows; ++i ) {
        for ( int j = 0; j < columns; ++j ) {
            if (channels == 1) {
                unsigned char pixel = img.at<unsigned char>(i, j);
                unsigned char test_pixel = cv_img.at<unsigned char>(i, j);
                if (pixel != test_pixel)
                    return false;
            }
            else {
                cv::Vec3b colors = img.at<cv::Vec3b>(i, j);
                cv::Vec3b test_colors = cv_img.at<cv::Vec3b>(i, j);
                for ( int x = 0; x < channels; ++x ) {
                    if(colors.val[x] != test_colors.val[x])
                        return false;
                }
            }
        }
    }
    return true;
}


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

void get_file_sizes(std::string &local_dir,
    std::string &efs_dir,
    std::string &remote_dir,
    std::string &base,
    std::string &type, 
    std::vector<std::vector<long long int>> &sizes, 
    VCL::RemoteConnection &remote)
{
    std::string local_name = local_dir +  type + "/" + base + "." + type;
    std::string efs_name = efs_dir + type + "/" + base + "." + type;
    std::string remote_name = remote_dir + type + "/" + base + "." + type;

    long long int img_size = 0;
    struct stat stat_buf;

    std::vector<long long int> size;

    if ( type == "tdb") {
        size.push_back(get_folder_size(local_name));
        size.push_back(get_folder_size(efs_name));
    }
    else {
        int irc = stat(local_name.c_str(), &stat_buf);
        if ( irc == 0 )
            img_size = stat_buf.st_size;
        size.push_back(img_size);
        irc = stat(efs_name.c_str(), &stat_buf);
        if ( irc == 0 )
            img_size = stat_buf.st_size;
        size.push_back(img_size);
    }
    
    size.push_back(remote.get_object_size(remote_name));    
    sizes.push_back(size);
}


void write(std::string &local_dir,
    std::string &efs_dir,
    std::string &remote_dir, 
    std::string &base, 
    cv::Mat &cv_img,
    std::string &type,
    int compression, int minimum, 
    VCL::RemoteConnection &remote, 
    std::vector<std::vector<float>> &times)
{
    std::vector<float> time;

    ChronoCpu localchrono("Write Local");
    ChronoCpu nfschrono("Write NFS");
    ChronoCpu remotechrono("Write Remote");
    
    std::string local_name = local_dir + type + "/" + base + "." + type;
    std::string efs_name = efs_dir + type + "/" + base + "." + type;
    std::string remote_name = remote_dir + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::CompressionType comp = get_compression(compression);

        VCL::Image tdbimg(cv_img);
        tdbimg.set_compression(comp);
        tdbimg.set_minimum_dimension(minimum);
        localchrono.tic();
        tdbimg.store(local_name, VCL::TDB);
        localchrono.tac();

        VCL::Image nfstdb(cv_img);
        nfstdb.set_compression(comp);
        nfstdb.set_minimum_dimension(minimum);
        nfschrono.tic();
        nfstdb.store(efs_name, VCL::TDB, false);
        nfschrono.tac();

        VCL::Image remotetdb(cv_img);
        remotetdb.set_connection(remote);
        remotetdb.set_compression(comp);
        remotetdb.set_minimum_dimension(minimum);
        remotechrono.tic();
        remotetdb.store(remote_name, VCL::TDB);
        remotechrono.tac();
    }
    else if (type == "tiff") {
        localchrono.tic();
        cv::imwrite(local_name, cv_img);
        localchrono.tac();

        nfschrono.tic();
        cv::imwrite(efs_name, cv_img);
        nfschrono.tac();

        std::vector<unsigned char> data;
        cv::imencode(".tiff", cv_img, data);
        remotechrono.tic();
        remote.write(remote_name, data);
        remotechrono.tac();
    }

    time.push_back(localchrono.getLastTime_us() / 1000.0);
    time.push_back(nfschrono.getLastTime_us() / 1000.0);
    time.push_back(remotechrono.getLastTime_us() / 1000.0);

    times.push_back(time);
}

void write_all(std::string &local_dir,
    std::string &efs_dir,
    std::string &remote_dir,
    std::string &base, 
    cv::Mat &cv_img,
    std::string &type,
    int compression, int minimum, 
    bool remote_io,
    VCL::RemoteConnection &remote)
{
    std::string local_name = local_dir + base + "." + type;
    std::string efs_name = efs_dir + base + "." + type;
    std::string remote_name = remote_dir + base + "." + type;

    if (type == "tdb") {
        VCL::CompressionType comp = get_compression(compression);
        if (remote_io) {
            VCL::Image remotetdb(cv_img);
            remotetdb.set_connection(remote);
            remotetdb.set_compression(comp);
            remotetdb.set_minimum_dimension(minimum);
            remotetdb.store(remote_name, VCL::TDB);
        }
        else {
            VCL::Image tdbimg(cv_img);
            tdbimg.set_compression(comp);
            tdbimg.set_minimum_dimension(minimum);
            tdbimg.store(local_name, VCL::TDB);
        }
    }
    else if (type == "tiff") {
        if (remote_io) {
            std::vector<unsigned char> data;
            cv::imencode(".tiff", cv_img, data);
            remote.write(remote_name, data);
        }
        else
            cv::imwrite(local_name, cv_img);
    }
}



void read(std::string &local_dir,
    std::string &efs_dir,
    std::string &remote_dir,
    std::string &base,
    std::string &type,
    VCL::RemoteConnection &remote, 
    std::vector<std::vector<float>> &times, 
    int index)
{
    ChronoCpu localchrono("Read Local");
    ChronoCpu nfschrono("Read NFS");
    ChronoCpu remotechrono("Read Remote");
    
    std::string local_name = local_dir + type + "/" + base + "." + type;
    std::string efs_name = efs_dir + type + "/" + base + "." + type;
    std::string remote_name = remote_dir + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::Image tdbimg(local_name);
        localchrono.tic();
        cv::Mat limg = tdbimg.get_cvmat();
        localchrono.tac();

        VCL::Image fsimg(efs_name);
        nfschrono.tic();
        cv::Mat fimg = fsimg.get_cvmat();
        nfschrono.tac();

        VCL::Image remotetdb(remote_name, remote);
        remotechrono.tic();
        cv::Mat rimg = remotetdb.get_cvmat();
        remotechrono.tac();

        // if (!compare_mat_mat(limg, rimg))
        //     std::cout << "Remote Image does not match Local Image\n";
    }
    else if (type == "tiff") {
        localchrono.tic();
        cv::Mat rimg = cv::imread(local_name, cv::IMREAD_ANYCOLOR);
        localchrono.tac();

        nfschrono.tic();
        cv::Mat fimg = cv::imread(efs_name, cv::IMREAD_ANYCOLOR);
        nfschrono.tac();

        remotechrono.tic();
        std::vector<char> imgdata = remote.read(remote_name);
        if ( !imgdata.empty() )
            cv::Mat limg = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
        remotechrono.tac();
    }

    times[index].push_back(localchrono.getLastTime_us() / 1000.0);
    times[index].push_back(nfschrono.getLastTime_us() / 1000.0);
    times[index].push_back(remotechrono.getLastTime_us() / 1000.0);
}




void crop(std::string &local_dir,
    std::string &efs_dir,
    std::string &remote_dir,
    std::string &base,
    std::string &type,
    VCL::RemoteConnection &remote, 
    std::vector<std::vector<float>> &times, 
    int index, 
    cv::Mat &img)
{
    ChronoCpu localchrono("Crop Local");
    ChronoCpu nfschrono("Crop NFS");
    ChronoCpu remotechrono("Crop Remote");

    int start_x = 2235;
    int start_y = 1233;
    int height = 211;
    int width = 81;
    
    std::string local_name = local_dir + type + "/" + base + "." + type;
    std::string efs_name = efs_dir + type + "/" + base + "." + type;
    std::string remote_name = remote_dir + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::Image tdbimg(local_name);
        localchrono.tic();
        tdbimg.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat crop_mat = tdbimg.get_cvmat();
        localchrono.tac();

        VCL::Image fsimg(efs_name);
        nfschrono.tic();
        fsimg.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat fimg = fsimg.get_cvmat();
        nfschrono.tac();

        VCL::Image remotetdb(remote_name, remote);
        remotechrono.tic();
        remotetdb.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat rimg = remotetdb.get_cvmat();
        remotechrono.tac();
    }
    else if (type == "tiff") {
        localchrono.tic();
        cv::Mat limg = cv::imread(local_name, cv::IMREAD_ANYCOLOR);
        cv::Mat localcrop(limg, VCL::Rectangle(start_x, start_y, width, height));
        localchrono.tac();

        nfschrono.tic();
        cv::Mat fimg = cv::imread(efs_name, cv::IMREAD_ANYCOLOR);
        cv::Mat efscrop(fimg, VCL::Rectangle(start_x, start_y, width, height));
        nfschrono.tac();

        remotechrono.tic();
        std::vector<char> imgdata = remote.read(remote_name);
        if ( !imgdata.empty() ) {
            cv::Mat rimg = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
            cv::Mat remotecrop(rimg, VCL::Rectangle(start_x, start_y, width, height));
        }
        remotechrono.tac();
    }

    times[index].push_back(localchrono.getLastTime_us() / 1000.0);
    times[index].push_back(nfschrono.getLastTime_us() / 1000.0);
    times[index].push_back(remotechrono.getLastTime_us() / 1000.0);
}

int main(int argc, char** argv )
{
    if ( argc != 5 )
    {
        printf("Usage: image dir, compression type, min tiles output file name \n");
        return -1;
    }

    std::string image_dir = argv[1];
    int compression = atoi(argv[2]);
    int min_tiles = atoi(argv[3]);
    std::string output_file = argv[4];

    std::ofstream outfile(output_file);

    std::vector<std::string> cameras;
    std::vector<std::string> frames;
    std::vector<std::string> tiff_files;
    std::vector<std::string> tdb_files;
    std::vector<int> heights;
    std::vector<int> widths;

    std::vector<std::vector<float>> tdb_times;
    std::vector<std::vector<float>> tiff_times;
    std::vector< std::vector<long long int>> tdb_sizes;
    std::vector< std::vector<long long int>> tiff_sizes;

    cv::Mat cv_img;

    // add parameters to remote_read for where images are being read from 
    // remote_read(<region>, <accessID>, <secretKey>)
    VCL::RemoteConnection remote_read();
    remote_read.start();

    // add parameters to remote_write for where images should be written to 
    // remote_write(<region>, <accessID>, <secretKey>)
    VCL::RemoteConnection remote_write();
    remote_write.start();
    
    int total_frames = 1;
    int total_cameras = 1;

    for (int i = 0; i < total_frames; ++i)
    {
        std::string frame_num = std::to_string(i);
        int zeros = 4 - frame_num.length();
        std::string frame = "F";
        for (int x = 0; x < zeros; ++x)
            frame += "0";
        frame += frame_num;
        frames.push_back(frame);
    }

    for (int j = 0; j < total_cameras; ++j)
    {
        int cam = j + 1;
        std::string cam_num = std::to_string(cam);
        int zeros = 4 - cam_num.length();

        std::string camera = "";
        for (int x = 0; x < zeros; ++x)
            camera += "0";

        camera += cam_num;
        cameras.push_back(camera);
    }

    std::string extension;
    std::string local_dir = "/data/image_results/";
    std::string efs_dir = "/efs/image_results/";
    std::string remote_dir = "s3://image_results/";


    std::cout << "Writing Individual\n";
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j]; 

            std::string fullpath = image_dir + frames[i] + "/ForReconstruction/" + cameras[j] + ".tif";
            std::cout << fullpath << std::endl;
            std::vector<char> data = remote_read.read(fullpath);
            if ( !data.empty() )
                cv_img = cv::imdecode(cv::Mat(data), cv::IMREAD_ANYCOLOR);

            extension = "tdb";
            write(local_dir, efs_dir, remote_dir, name, cv_img, extension, compression, min_tiles, remote_write, tdb_times);
            extension = "tiff";
            write(local_dir, efs_dir, remote_dir, name, cv_img, extension, compression, min_tiles, remote_write, tiff_times);
        }
    }

    std::cout << "Reading\n";
    int index = 0;
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j];
            extension = "tdb";
            read(local_dir, efs_dir, remote_dir, name, extension, remote_write, tdb_times, index);
            extension = "tiff";
            read(local_dir, efs_dir, remote_dir, name, extension, remote_write, tiff_times, index);
            ++index;
         }
     }

    std::cout << "Cropping\n";
    index = 0;
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j];
            extension = "tdb";
            crop(local_dir, efs_dir, remote_dir, name, extension, remote_write, tdb_times, index, cv_img);
            extension = "tiff";
            crop(local_dir, efs_dir, remote_dir, name, extension, remote_write, tiff_times, index, cv_img);
            ++index;
        }
    }

    std::cout << "Getting File Sizes\n";
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {
            std::string name = frames[i] + "_" + cameras[j] + "_" + std::to_string(z);  
            std::cout << name << std::endl;  
            extension = "tdb";
            get_file_sizes(local_dir, efs_dir, remote_dir, name, extension, tdb_sizes, remote_write);
            extension = "tiff";
            get_file_sizes(local_dir, efs_dir, remote_dir, name, extension, tiff_sizes, remote_write);
        }
    }

    std::cout << "Output to " << output_file << std::endl;
    outfile << "# Image Name, Frame/Camera, ";
    outfile << "TIFF EBS Size, TDB EBS Size, ";
    outfile << "TIFF EFS Size, TDB EFS Size, ";
    outfile << "TIFF S3 Size, TDB S3 Size, ";
    outfile << "TIFF EBS Write, TDB EBS Write, ";
    outfile << "TIFF EFS Write, TDB EFS Write, ";
    outfile << "TIFF S3 Write, TDB S3 Write, ";
    outfile << "TIFF EBS Read, TDB EBS Read, ";
    outfile << "TIFF EFS Read, TDB EFS Read, ";
    outfile << "TIFF S3 Read, TDB S3 Read, ";
    outfile << "TIFF EBS Crop, TDB EBS Crop, ";
    outfile << "TIFF EFS Crop, TDB EFS Crop, ";
    outfile << "TIFF S3 Crop, TDB S3 Crop, ";
    outfile << "\n";


    for (int i = 0; i < frames.size(); ++i) {
        for (int j = 0; j < cameras.size(); ++j) {
            outfile << frames[i] << ", " << cameras[j] << ", ";
            for (int k = 0; k < tiff_sizes[i + j].size(); ++k) {
                outfile << tiff_sizes[i + j][k] << ", ";
                outfile << tdb_sizes[i + j][k] << ", ";
            }
            for (int x = 0; x < tdb_times[i + j].size(); ++x) {
                outfile << tiff_times[i + j][x] << ", ";
                outfile << tdb_times[i + j][x] << ", ";
            }
            outfile << std::endl;
        }
    }

  return 0;
}