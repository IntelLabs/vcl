import psutil
import time
from subprocess import call
from subprocess import PIPE
import os
import cPickle as pickle
import sys
from collections import deque

import boto3
import botocore


#TODO dump page cache and direntries helper string
DROP_CACHES = "echo 3 > /proc/sys/vm/drop_caches"

BUCKET_NAME = "netowrkingteam-test"
IMAGE_KEY_START = "FD19_SAMPLE_SET/ImageEnhancement/"

def output(fpath, output_dict):

    print "Writing Results Pickle to: %s" %fpath    
    outfile = open(fpath, 'wb')
    
    pickle.dump(output_dict, outfile)

    outfile.close()
    
def drop_cache():
    print "Dropping Page and Directory Entry Caches"
    os.system(DROP_CACHES)

def script_runner(command, tracking=True, drop_caches=True):   
    test_running = True
    counter = 0

    # TODO, check sizes and or do periodic dumps
    # TODO, create dictionary at end for a small speedup
    if drop_caches is True:
        print "Dropping Caches and Sleeping for a second"
        drop_cache()
        time.sleep(1)

    total_dict      = {
            "mem":deque(),
            "cpu_percent":deque(),
            "cpu_time":deque(),
            "disk":deque(),
            "network":deque(),
            "time":deque()
            ,"process":deque()
        }

    #initialize Counters: when processing results, initial datapoint
    #used to normalize/zero counters against
    total_dict["cpu_count"] = psutil.cpu_count()
    total_dict["time"].append(time.time())
    total_dict["cpu_percent"].append(psutil.cpu_percent(percpu=True))
    total_dict["cpu_time"].append(psutil.cpu_times(percpu=True))
    total_dict["mem"].append(psutil.virtual_memory())
    total_dict["disk"].append(psutil.disk_io_counters(perdisk=True))
    total_dict["network"].append(psutil.net_io_counters(pernic=True))
    
    # kick off test
    start_time = time.time()

    test_sub_proc = psutil.Popen(command, stdout=PIPE)
    proc_id = test_sub_proc.pid
    p = psutil.Process(proc_id)
    pdict = p.as_dict(attrs=['cpu_percent', 'cpu_times', 'io_counters', 'memory_info'])
    total_dict["process"].append(pdict)
        
    while test_running:
        if tracking is True:
            if p.is_running():
                pdict = p.as_dict(attrs=['cpu_percent', 'cpu_times', 'io_counters', 'memory_info'])
                total_dict["process"].append(pdict)
                    
                total_dict["time"].append(time.time())
                total_dict["cpu_percent"].append(psutil.cpu_percent(percpu=True))
                total_dict["cpu_time"].append(psutil.cpu_times(percpu=True))
                total_dict["mem"].append(psutil.virtual_memory())
                total_dict["disk"].append(psutil.disk_io_counters(perdisk=True))
                total_dict["network"].append(psutil.net_io_counters(pernic=True))
        
        # poll if test is still running, if not, break out of the
        # tracking loop
        test_sub_proc.poll()
        if test_sub_proc.returncode is not None:
            end_time = time.time()
            break

        
    if tracking is True:
        #grab final counts 
        total_dict["time"].append(time.time())
        total_dict["cpu_percent"].append(psutil.cpu_percent(percpu=True))
        total_dict["cpu_time"].append(psutil.cpu_times(percpu=True))
        total_dict["mem"].append(psutil.virtual_memory())
        total_dict["disk"].append(psutil.disk_io_counters(perdisk=True))
        total_dict["network"].append(psutil.net_io_counters(pernic=True))

    total_dict["total_time"] = end_time - start_time

    return total_dict

def get_images():
    s3 = boto3.resource('s3')
    img_list = []
    
    # for x in range(0, 129):
    for x in range(0, 1):
        frame = str(x)
        num_zeros = 4 - len(frame)
        zeros = "F"
        for y in range(0, num_zeros):
            zeros += "0"
        fdir = IMAGE_KEY_START + zeros + frame + "/ForReconstruction/"
        
        # for x in range(1, 39):
        for x in range(1, 11):
            cam = str(x)
            num_zeros = 4 - len(cam)
            zeros = ""
            for y in range(0, num_zeros):
                zeros += "0"
            key = fdir + zeros + cam + ".tif"
            print key

            try:
                img = frame + "_" + cam + ".tif"
                img_list.append(img)
                s3.Bucket(BUCKET_NAME).download_file(key, "../freed_images/" + img)
            except botocore.exceptions.ClientError as e:
                if e.response['Error']['Code'] == "404":
                    print("The object does not exist.")
                else:
                    raise

    return img_list

def write(res_out_dir, images, tracking, cache_drop):
    operation = "write"
    # filetypes = ["raw", "raw_compressed", "png", "jpg", "tdb"]
    filetype = "tdb"

    for i in images:
        base_name = i.split(".")[0].strip()

        operation_command = ["./test/benchmark_tests/cpu_tests", operation, filetype, res_out_dir, i, "../freed_images/"]
        print operation_command

        sys_outpath = res_out_dir + base_name + "_sys.pckl"
        print sys_outpath
        sys = script_runner(operation_command, tracking, cache_drop)
        delta_list = []
    
        print "Total Run Time: %f" % sys["total_time"]

        output(sys_outpath, sys)


# def read(d, res_out_dir, images, tracking, cache_drop):
#     operation = "read"
#     filetypes = ["raw", "raw_compressed", "png", "jpg", "tdb_buffer", "tdb_mat"]

#     for filetype in filetypes:

#         for img in images:
#             base_name = d + "_" + img + "_" + operation + "_" + filetype

#             img_file = d + "_" + img + "_write_" 

#             if filetype == "tdb_buffer" or filetype == "tdb_mat":
#                 img_file += "tdb.txt"
#             else:
#                 img_file += filetype + ".txt"

#             base_test_name = "output/" + base_name + ".txt"
#             operation_command = ["/home/crstrong/vcl/test/cpu_tests", operation, filetype, res_out_dir, img_file]

#             sys_outpath = res_out_dir + "output/" + base_name + "_sys.pckl"
#             sys = script_runner(operation_command, tracking, cache_drop)
#             delta_list = []
        
#             print "Total Run Time: %f" % sys["total_time"]

#             output(sys_outpath, sys)

# def crop(d, res_out_dir, images, tracking, cache_drop):
#     operation = "crop"
#     filetypes = ["raw", "png", "jpg", "tdb_buffer", "tdb_mat"]
#     start_x = "100"
#     start_y = "100"

#     for filetype in filetypes:

#         for img in images:
#             base_name = d + "_" + img + "_" + operation + "_" + filetype

#             img_file = d + "_" + img + "_write_" 

#             if filetype == "tdb_buffer" or filetype == "tdb_mat":
#                 img_file += "tdb.txt"
#             else:
#                 img_file += filetype + ".txt"

#             base_test_name = "output/" + base_name + ".txt"
#             operation_command = ["/home/crstrong/vcl/test/cpu_tests", operation, filetype, res_out_dir, 
#                 img_file, start_x, start_y]

#             sys_outpath = res_out_dir + "output/" + base_name + "_sys.pckl"
#             sys = script_runner(operation_command, tracking, cache_drop)
#             delta_list = []
        
#             print "Total Run Time: %f" % sys["total_time"]

#             output(sys_outpath, sys)


# def resize(d, res_out_dir, images, tracking, cache_drop):
#     operation = "resize"
#     filetypes = ["raw", "png", "jpg", "tdb_buffer", "tdb_mat"]
#     resize_height = "256"
#     resize_width = "256"

#     for filetype in filetypes:

#         for img in images:
#             base_name = d + "_" + img + "_" + operation + "_" + filetype

#             img_file = d + "_" + img + "_write_" 

#             if filetype == "tdb_buffer" or filetype == "tdb_mat":
#                 img_file += "tdb.txt"
#             else:
#                 img_file += filetype + ".txt"

#             base_test_name = "output/" + base_name + ".txt"
#             operation_command = ["/home/crstrong/vcl/test/cpu_tests", operation, filetype, res_out_dir, 
#                 img_file, resize_height, resize_width]

#             sys_outpath = res_out_dir + "output/" + base_name + "_sys.pckl"
#             sys = script_runner(operation_command, tracking, cache_drop)
#             delta_list = []
        
#             print "Total Run Time: %f" % sys["total_time"]

#             output(sys_outpath, sys)


def main():

    tracking = True
    cache_drop = True
    res_out_dir = "test_output/"
    remote_dir = "s3://irlcsrtdbtests/freed/"

    imgs = get_images()
    write(res_out_dir, imgs, False, True)
    # write(remote_dir, imgs, True, True)

    # read(d, res_out_dir, images)
    # crop(d, res_out_dir, images)
    # resize(d, res_out_dir, images)
    # call(["rm", "-r", "test_output/tdb/"])

if __name__ == "__main__":
    main()