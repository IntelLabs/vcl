import cPickle as pickle
import numpy as np
import datetime

#TODO cleanup comments/documentation



CPU_USER = 0
CPU_NICE = 1
CPU_SYSTEM = 2
CPU_IDLE = 3
CPU_IOWAIT = 4
CPU_IRQ = 5
CPU_SOFTIRQ = 6
CPU_STEAL = 7
CPU_GUEST = 8
CPU_GUEST_NICE = 9

def load_pickle(fpath):
    infile = open(fpath, 'rb')

    results = pickle.load(infile)
    
    infile.close()

    return results

def load_caffe_glog(fpath):
    infile = open(fpath, 'r')
    raw_linelist = []
    
    for line in infile:
        raw_linelist.append(line)

    return raw_linelist



def mnist_classify_bar(fpath):

    results = load_pickle(fpath)


    proc_dict = {}
    for key in results.iterkeys():
        print key
        print len(results[key])
        for res in results[key]:
            print res

        proc_dict[key] = sum(results[key])
        
    return proc_dict

def mnist_classify_raw(fpath):
    return load_pickle(fpath)

# returns dictionary
# each dictionary key is op type metric (rw counts/bytes)
# each value is ordered list of timestamped tuples
# normalized to start at 0
def disk_rw_counts(res_dict, dev_id):
    
    disk_list = res_dict["disk"]        
    time_list = res_dict["time"]
    cur_idx = 0

    baseline_time = time_list[0]

    rw_dict = {'r_count':[(0.0,0)], 'r_bytes':[(0.0,0)], \
                   'w_count':[(0.0,0)], 'w_bytes':[(0.0,0)]}
    
    while cur_idx < len(disk_list)-1:
        
        cur_dev_list = disk_list[cur_idx]   
        next_dev_list = disk_list[cur_idx + 1]
        cur_time = time_list[cur_idx + 1] - baseline_time   

        read_count = next_dev_list[dev_id].read_count - cur_dev_list[dev_id].read_count
        write_count = next_dev_list[dev_id].write_count - cur_dev_list[dev_id].write_count
        read_bytes = next_dev_list[dev_id].read_bytes - cur_dev_list[dev_id].read_bytes
        write_bytes = next_dev_list[dev_id].write_bytes - cur_dev_list[dev_id].write_bytes
        
        rc_tup = (cur_time, read_count)
        wc_tup = (cur_time, write_count)
        r_bytes = (cur_time, read_bytes)
        w_bytes = (cur_time, write_bytes)

        rw_dict['r_count'].append(rc_tup)
        rw_dict['w_count'].append(wc_tup)
        rw_dict['r_bytes'].append(r_bytes)
        rw_dict['w_bytes'].append(w_bytes)
    
        cur_idx += 1  

    return rw_dict

def disk_rw_counts_bucketed(res_dict, dev_id):
    
    disk_list = res_dict["disk"]
    time_list = res_dict["time"]
    cur_idx = 0


    baseline_time = time_list[0]
    max_time = time_list[len(time_list)-1]
    time_range = int(max_time - baseline_time) + 1

    rc_buckets = [0 for x in range(time_range)]
    wc_buckets = [0 for x in range(time_range)]
    rb_buckets = [0 for x in range(time_range)]
    wb_buckets = [0 for x in range(time_range)]

    rw_dict = {'r_count':[], 'r_bytes':[], \
                   'w_count':[], 'w_bytes':[]}

    while cur_idx < len(disk_list)-1:

        cur_dev_list = disk_list[cur_idx]
        next_dev_list = disk_list[cur_idx + 1]
        cur_time = time_list[cur_idx + 1] - baseline_time

        buck_idx = int(cur_time)

        read_count = next_dev_list[dev_id].read_count - cur_dev_list[dev_id].read_count
        write_count = next_dev_list[dev_id].write_count - cur_dev_list[dev_id].write_count
        read_bytes = next_dev_list[dev_id].read_bytes - cur_dev_list[dev_id].read_bytes
        write_bytes = next_dev_list[dev_id].write_bytes - cur_dev_list[dev_id].write_bytes

        rc_buckets[buck_idx] += read_count
        rb_buckets[buck_idx] += read_bytes
        wc_buckets[buck_idx] += write_count
        wb_buckets[buck_idx] += write_bytes
        
        cur_idx += 1

    # now convert this into the expected tuple lists for the plotters
    for buck_idx in range(len(rc_buckets)):
        rw_dict['r_count'].append((buck_idx,rc_buckets[buck_idx]))
        rw_dict['w_count'].append((buck_idx,wc_buckets[buck_idx]))
        rw_dict['r_bytes'].append((buck_idx,rb_buckets[buck_idx]))
        rw_dict['w_bytes'].append((buck_idx,wb_buckets[buck_idx]))

    return rw_dict

def cpu_percs(res_dict):
    percs =  res_dict["cpu_percent"]
    times = res_dict["time"]
    baseline_time = times[0]
    by_cpu = []
    for _ in range(72):
        by_cpu.append([])

    #TODO, hardcoded for 72 CPUs
    for outer_idx in range(len(percs)):
        
        perc_list = percs[outer_idx]
        time = times[outer_idx] - baseline_time
        for idx in range(len(perc_list)):
            # if perc_list[idx] > 0.0:
            by_cpu[idx].append((time, perc_list[idx]))

    
    return by_cpu

# TODO
def cpu_counters(res_dict):
    
    cpu_times = res_dict["cpu_time"]
    times = res_dict["time"]
    baseline_time = times[0]

    user = []
    nice = []
    system = []
    idle = []
    iowait = []
    irq = []
    softirq = []
    steal = []
    guest = []
    guest_nice = []

    cur_step_entry = cpu_times[0]
    next_step_entry = cpu_times[len(cpu_times)-1]
    cur_time = times[len(times)-1] - baseline_time 
    
    for cpu_idx in range(len(cur_step_entry)):
        user_time = next_step_entry[cpu_idx].user - cur_step_entry[cpu_idx].user
        nice_time = next_step_entry[cpu_idx].nice - cur_step_entry[cpu_idx].nice
        system_time = next_step_entry[cpu_idx].system - cur_step_entry[cpu_idx].system
        idle_time = next_step_entry[cpu_idx].idle - cur_step_entry[cpu_idx].idle
        iowait_time = next_step_entry[cpu_idx].iowait - cur_step_entry[cpu_idx].iowait
        irq_time = next_step_entry[cpu_idx].irq - cur_step_entry[cpu_idx].irq
        soft_irq_time = next_step_entry[cpu_idx].softirq - cur_step_entry[cpu_idx].softirq
        steal_time = next_step_entry[cpu_idx].steal - cur_step_entry[cpu_idx].steal
        guest_time = next_step_entry[cpu_idx].guest - cur_step_entry[cpu_idx].guest
        guest_nice_time = next_step_entry[cpu_idx].guest_nice - cur_step_entry[cpu_idx].guest_nice
    
        user.append(user_time)
        nice.append(nice_time)
        system.append(system_time)
        idle.append(idle_time)
        iowait.append(iowait_time)
        irq.append(irq_time)
        softirq.append(soft_irq_time)
        steal.append(steal_time)
        guest.append(guest_time)
        guest_nice.append(guest_nice_time)

    return user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice

def mem_usage(res_dict):
    
    times = res_dict["time"]
    mem_usage = res_dict["mem"]
    baseline_time = times[0]

    total = []
    available = []
    percent = []
    used = []
    free = []
    active = []
    inactive = []
    buffers = []
    cached = []
    # shared = []

    for idx in range(len(mem_usage)):
        entry = mem_usage[idx]
        time = times[idx] - baseline_time

        total.append((time, entry.total))
        available.append((time, entry.available))
        percent.append((time, entry.percent))
        used.append((time, entry.used))
        free.append((time, entry.free))
        active.append((time, entry.active))
        inactive.append((time, entry.inactive))
        buffers.append((time, entry.buffers))
        cached.append((time, entry.cached))
        # shared.append((time, entry.shared))

    return total, available, percent, used, free, active, inactive, buffers, cached#, shared

def net_stats(res_dict, tgt_nic):

        
    net_stats = res_dict["network"]
    times = res_dict["time"]
    baseline_time = times[0]

    bytes_sent = []
    bytes_rec = []
    pack_sent = []
    pack_rec = []
    err_in = []
    err_out = []
    drop_in = []
    drop_out = []

    for idx in range(len(net_stats) -1 ):
        cur_nic_info = net_stats[idx]
        next_nic_info = net_stats[idx + 1]
        time = times[idx] - baseline_time
        
        nic = cur_nic_info[tgt_nic] 
        next_nic = next_nic_info[tgt_nic]
        
    
        b_sent = next_nic.bytes_sent - nic.bytes_sent
        b_rec = next_nic.bytes_recv - nic.bytes_recv
        p_sent = next_nic.packets_sent - nic.packets_sent
        p_rec = next_nic.packets_recv - nic.packets_recv
        e_in = next_nic.errin - nic.errin
        e_out = next_nic.errout - nic.errout
        d_in = next_nic.dropin - nic.dropin
        d_out = next_nic.dropout - nic.dropout


        bytes_sent.append((time,b_sent))
        bytes_rec.append((time, b_rec))
        pack_sent.append((time, p_sent))
        pack_rec.append((time, p_rec))
        err_in.append((time, e_in))
        err_out.append((time, e_out))
        drop_in.append((time, d_in))
        drop_out.append((time, d_out))

    return bytes_sent, bytes_rec, pack_sent, pack_rec, err_in, err_out, drop_in, drop_out

def net_stats_bucketed(res_dict, tgt_nic):

    net_stats = res_dict["network"]
    times = res_dict["time"]
    baseline_time = times[0]
    max_time = times[len(times)-1]
    time_range = int(max_time - baseline_time) + 1

    bytes_sent = [0 for x in range(time_range)]
    bytes_rec = [0 for x in range(time_range)]
    pack_sent = [0 for x in range(time_range)]
    pack_rec = [0 for x in range(time_range)]
    err_in = [0 for x in range(time_range)]
    err_out = [0 for x in range(time_range)]
    drop_in = [0 for x in range(time_range)]
    drop_out = [0 for x in range(time_range)]

    for idx in range(len(net_stats) -1 ):
            cur_nic_info = net_stats[idx]
            next_nic_info = net_stats[idx + 1]
            time = times[idx] - baseline_time

    buck_idx = int(time)

    nic = cur_nic_info[tgt_nic]
    next_nic = next_nic_info[tgt_nic]

    b_sent = next_nic.bytes_sent - nic.bytes_sent
    b_rec = next_nic.bytes_recv - nic.bytes_recv
    p_sent = next_nic.packets_sent - nic.packets_sent
    p_rec = next_nic.packets_recv - nic.packets_recv
    e_in = next_nic.errin - nic.errin
    e_out = next_nic.errout - nic.errout
    d_in = next_nic.dropin - nic.dropin
    d_out = next_nic.dropout - nic.dropout

    bytes_sent[buck_idx] += b_sent
    bytes_rec[buck_idx] += b_rec
    pack_sent[buck_idx] += p_sent
    pack_rec[buck_idx] += p_rec
    err_in[buck_idx] += e_in
    err_out[buck_idx] += e_out
    drop_in[buck_idx] += d_in
    drop_out[buck_idx] += d_out

    #convert to tuples
    bs_tup = []
    br_tup = []
    ps_tup = []
    pr_tup = []
    ei_tup = []
    eo_tup = []
    di_tup = []
    do_tup = []

    for buck_idx in range(len(bytes_sent)):
        bs_tup.append((buck_idx, bytes_sent[buck_idx]))
        br_tup.append((buck_idx, bytes_rec[buck_idx]))
        ps_tup.append((buck_idx, pack_sent[buck_idx]))
        pr_tup.append((buck_idx, pack_rec[buck_idx]))
        ei_tup.append((buck_idx, err_in[buck_idx]))
        eo_tup.append((buck_idx, err_out[buck_idx]))
        di_tup.append((buck_idx, drop_in[buck_idx]))
        do_tup.append((buck_idx, drop_out[buck_idx]))

    return bs_tup, br_tup, ps_tup, pr_tup, ei_tup, eo_tup, di_tup, do_tup


def gpu_stats(res_dict, tgt_gpu):
    
    gpu_stats = res_dict["gpu_stats"]
    times = res_dict["time"]
    baseline_time = times[0]
    tup_list = []

    for idx in range(len(times)-1):
        cur_time = times[idx] - baseline_time
        cur_stats = gpu_stats[idx]
        cur_tup=(cur_time, cur_stats[tgt_gpu])
        tup_list.append(cur_tup)    
    
    return tup_list 


# NOTE, currently brittle, acts weird if the times being looked at
# span a day boundary
def caffe_iter_stats(lines):
    
    #First, get the starting time, everything is relative to this
    line_split = lines[0].strip().split(" ")
    date_split = line_split[len(line_split)-2].split("/")
    time_split = line_split[len(line_split)-1].split(":")


    start_time =  datetime.datetime(int(date_split[0]), int(date_split[1]), int(date_split[2]),
                    int(time_split[0]), int(time_split[1]), int(time_split[2]))


    res_tups = []
    # now store each iter per second statistic 
    for line in lines:
        # if we fine and iter per second count, lets process it
        if line.count("iter/s") != 0:
            space_split = line.split(" ")
            initial_split = line.strip().split("(")
            secondary_split = initial_split[1].split(" ")   
            iter_s = secondary_split[0]
            time_string = space_split[1]    
            time_split = time_string.split(":")
            seconds_split = time_split[2].split(".")
        
            cur_hours = int(time_split[0])
            cur_minutes = int(time_split[1])
            cur_seconds = int(seconds_split[0])

            cur_dt = datetime.datetime(start_time.year, start_time.month, start_time.day,
                           cur_hours, cur_minutes, cur_seconds)
            delta = cur_dt - start_time
            seconds = delta.seconds
            datapoint_tup = (seconds, iter_s)
            
            res_tups.append(datapoint_tup)


    return res_tups 

def process_info(process):

    last_grab = len(process) - 1

    cpu_percents = []
    for x in range(1, last_grab + 1):
        cpu_percents.append(process[x]["cpu_percent"])

    avg_percent = np.mean(cpu_percents)
    std_dev_percent = np.std(cpu_percents)

    return avg_percent, std_dev_percent, process[last_grab]["io_counters"]


def set_cpu_header():
    cpu_header = "# Image Name, File Size, Write Raw Avg, Write Raw Dev, Write Raw Compressed Avg, Write Raw Compressed Dev, "
    cpu_header += "Write PNG Avg, Write PNG Dev, Write JPG Avg, Write JPG Dev, Write TDB Avg, Write TDB Dev, "
    cpu_header += "Read Raw Avg, Read Raw Dev, Read Raw Compressed Avg, Read Raw Compressed Dev, "
    cpu_header += "Read PNG Avg, Read PNG Dev, Read JPG Avg, Read JPG Dev, "
    cpu_header += "Read TDB Buffer Avg, Read TDB Buffer Dev, Read TDB Mat Avg, Read TDB Mat Dev, "
    cpu_header += "Crop Raw Avg, Crop Raw Dev, Crop PNG Avg, Crop PNG Dev, Crop JPG Avg, Crop JPG Dev, "
    cpu_header += "Crop TDB Buffer Avg, Crop TDB Buffer Dev, Crop TDB Mat Avg, Crop TDB Mat Dev, "
    cpu_header += "Resize Raw Avg, Resize Raw Dev, Resize PNG Avg, Resize PNG Dev, Resize JPG Avg, Resize JPG Dev, "
    cpu_header += "Resize TDB Buffer Avg, Resize TDB Buffer Dev, Resize TDB Mat Avg, Resize TDB Mat Dev, "

    return cpu_header + "\n"

def set_io_header():
    writes = "# Image Name, Size, Write Raw Read Calls, Write Raw Write Calls, Write Raw Read Bytes, Write Raw Write Bytes, "
    writes += "Write Raw Compressed Read Calls, Write Raw Compressed Write Calls, Write Raw Compressed Read Bytes, Write Raw Compressed Write Bytes, "
    writes += "Write PNG Read Calls, Write PNG Write Calls, Write PNG Read Bytes, Write PNG Write Bytes, "
    writes += "Write JPG Read Calls, Write JPG Write Calls, Write JPG Read Bytes, Write JPG Write Bytes, "
    writes += "Write TDB Read Calls, Write TDB Write Calls, Write TDB Read Bytes, Write TDB Write Bytes, "
    
    reads = "Read Raw Read Calls, Read Raw Write Calls, Read Raw Read Bytes, Read Raw Write Bytes, "
    reads += "Read Raw Compressed Read Calls, Read Raw Compressed Write Calls, Read Raw Compressed Read Bytes, Read Raw Compressed Write Bytes, "
    reads += "Read PNG Read Calls, Read PNG Write Calls, Read PNG Read Bytes, Read PNG Write Bytes, "
    reads += "Read JPG Read Calls, Read JPG Write Calls, Read JPG Read Bytes, Read JPG Write Bytes, "
    reads += "Read TDB Buffer Read Calls, Read TDB Buffer Write Calls, Read TDB Buffer Read Bytes, Read TDB Buffer Write Bytes, "
    reads += "Read TDB Mat Read Calls, Read TDB Mat Write Calls, Read TDB Mat Read Bytes, Read TDB Mat Write Bytes, "

    crops = "Crop Raw Read Calls, Crop Raw Write Calls, Crop Raw Read Bytes, Crop Raw Write Bytes, "
    crops += "Crop PNG Read Calls, Crop PNG Write Calls, Crop PNG Read Bytes, Crop PNG Write Bytes, "
    crops += "Crop JPG Read Calls, Crop JPG Write Calls, Crop JPG Read Bytes, Crop JPG Write Bytes, "
    crops += "Crop TDB Buffer Read Calls, Crop TDB Buffer Write Calls, Crop TDB Buffer Read Bytes, Crop TDB Buffer Write Bytes, "
    crops += "Crop TDB Mat Read Calls, Crop TDB Mat Write Calls, Crop TDB Mat Read Bytes, Crop TDB Mat Write Bytes, "

    resizes = "Resize Raw Read Calls, Resize Raw Write Calls, Resize Raw Read Bytes, Resize Raw Write Bytes, "
    resizes += "Resize PNG Read Calls, Resize PNG Write Calls, Resize PNG Read Bytes, Resize PNG Write Bytes, "
    resizes += "Resize JPG Read Calls, Resize JPG Write Calls, Resize JPG Read Bytes, Resize JPG Write Bytes, "
    resizes += "Resize TDB Buffer Read Calls, Resize TDB Buffer Write Calls, Resize TDB Buffer Read Bytes, Resize TDB Buffer Write Bytes, "
    resizes += "Resize TDB Mat Read Calls, Resize TDB Mat Write Calls, Resize TDB Mat Read Bytes, Resize TDB Mat Write Bytes, "

    return writes + reads + crops + resizes + "\n"


def get_write_data(base_path, dir_name, i, end, target, sys_io_nums, io_nums, cpu_percents):
    operation = "write"
    # types = ["raw", "raw_compressed", "png", "jpg", "tdb"]
    types = ["tdb"]

    for t in types:
        fpath = base_path + dir_name + "_" + i + "_" + operation + "_" + t + end
        res_dict = load_pickle(fpath) 

        size = len(res_dict["disk"]) - 1
        io_start = res_dict["disk"][0][target]
        io_end = res_dict["disk"][size][target]

        sys_io_nums += str(io_end[0] - io_start[0]) + ", " + str(io_end[1] - io_start[1]) + ", "
        sys_io_nums += str(io_end[2] - io_start[2]) + ", " + str(io_end[3] - io_start[3]) + ", "

        avg_cpu, std_cpu, io_counts = process_info(res_dict["process"])

        cpu_percents += str(avg_cpu) + ", " + str(std_cpu) + ", "
        io_nums += str(io_counts[0]) + ", " + str(io_counts[1]) + ", " + str(io_counts[2]) + ", " + str(io_counts[3]) + ", "

    return sys_io_nums, io_nums, cpu_percents

def get_read_data(base_path, dir_name, i, end, target, sys_io_nums, io_nums, cpu_percents):
    operation = "read"
    types = ["raw", "raw_compressed", "png", "jpg", "tdb_buffer", "tdb_mat"]

    for t in types:
        fpath = base_path + dir_name + "_" + i + "_" + operation + "_" + t + end
        res_dict = load_pickle(fpath) 

        size = len(res_dict["disk"]) - 1
        io_start = res_dict["disk"][0][target]
        io_end = res_dict["disk"][size][target]
        
        sys_io_nums += str(io_end[0] - io_start[0]) + ", " + str(io_end[1] - io_start[1]) + ", "
        sys_io_nums += str(io_end[2] - io_start[2]) + ", " + str(io_end[3] - io_start[3]) + ", "

        avg_cpu, std_cpu, io_counts = process_info(res_dict["process"])
        cpu_percents += str(avg_cpu) + ", " + str(std_cpu) + ", "
        io_nums += str(io_counts[0]) + ", " + str(io_counts[1]) + ", " + str(io_counts[2]) + ", " + str(io_counts[3]) + ", "

    return sys_io_nums, io_nums, cpu_percents

def get_crop_data(base_path, dir_name, i, end, target, sys_io_nums, io_nums, cpu_percents):
    operation = "crop"
    types = ["raw", "png", "jpg", "tdb_buffer", "tdb_mat"]

    for t in types:
        fpath = base_path + dir_name + "_" + i + "_" + operation + "_" + t + end
        res_dict = load_pickle(fpath) 

        size = len(res_dict["disk"]) - 1
        io_start = res_dict["disk"][0][target]
        io_end = res_dict["disk"][size][target]

        sys_io_nums += str(io_end[0] - io_start[0]) + ", " + str(io_end[1] - io_start[1]) + ", "
        sys_io_nums += str(io_end[2] - io_start[2]) + ", " + str(io_end[3] - io_start[3]) + ", "

        avg_cpu, std_cpu, io_counts = process_info(res_dict["process"])
        cpu_percents += str(avg_cpu) + ", " + str(std_cpu) + ", "
        io_nums += str(io_counts[0]) + ", " + str(io_counts[1]) + ", " + str(io_counts[2]) + ", " + str(io_counts[3]) + ", "

    return sys_io_nums, io_nums, cpu_percents

def get_resize_data(base_path, dir_name, i, end, target, sys_io_nums, io_nums, cpu_percents):
    operation = "resize"
    types = ["raw", "png", "jpg", "tdb_buffer", "tdb_mat"]

    for t in types:
        fpath = base_path + dir_name + "_" + i + "_" + operation + "_" + t + end
        res_dict = load_pickle(fpath) 

        size = len(res_dict["disk"]) - 1
        io_start = res_dict["disk"][0][target]
        io_end = res_dict["disk"][size][target]

        sys_io_nums += str(io_end[0] - io_start[0]) + ", " + str(io_end[1] - io_start[1]) + ", "
        sys_io_nums += str(io_end[2] - io_start[2]) + ", " + str(io_end[3] - io_start[3]) + ", "

        avg_cpu, std_cpu, io_counts = process_info(res_dict["process"])
        cpu_percents += str(avg_cpu) + ", " + str(std_cpu) + ", "
        io_nums += str(io_counts[0]) + ", " + str(io_counts[1]) + ", " + str(io_counts[2]) + ", " + str(io_counts[3]) + ", "

    return sys_io_nums, io_nums, cpu_percents


def main():
    # fpath = "/data/io_tests/cpu/output/0_07images_write_raw_sys.pckl"
    # # glog_fpath = "/tmp/caffe.sulaco.ifadams.log.INFO.20170905-054506.8798"
    # target_device = "sda7"

    # res_dict = load_pickle(fpath) 
    # lines = load_caffe_glog(glog_fpath)
    # caffe_iter_stats(lines)

    # res_dict = disk_rw_counts(res_dict, target_device)
    # print res_dict

    # by_cpu = cpu_percs(res_dict)
    # for x in range(0, len(by_cpu[0])):
    #   total_cpu = 0
    #   for y in by_cpu:
    #       total_cpu += y[x][1]
    #   print by_cpu[0][x][0], total_cpu

    # total_avg = 0
    # count = 0
    # for cpu in by_cpu:
    #   avg = 0
    #   for x in range(0, len(cpu)):
    #       avg += cpu[x][1]
    #   if avg > 0:
    #       count += 1
    #   total_avg += avg / len(cpu)

    # print total_avg / len(by_cpu), count
    # total, available, percent, used, free, active, inactive, buffers, cached = mem_usage(res_dict)
    # print total
    #b_s, b_r, p_s, p_r, e_i, e_o, d_i, d_o = net_stats(res_dict, "ens513f0")
    # user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice = cpu_counters(res_dict)

    # print user

    # base_path = "/data/io_tests/cpu/output/"

    # dir_names = ["0_07images", 
    #   "0_23images", "0_43images", "0_60images", "0_81images", "1mpimages", 
    #   "2mpimages", "3mpimages", "4mpimages", "5mpimages", "6mpimages", "7mpimages", "8mpimages", 
    #   "10mpimages", "12mpimages", 
    #   "16mpimages"
    #   ]
    # filesize = ["0.07", 
    #   "0.23", "0.43", "0.60", "0.81", "1", 
    #   "2", "3", "4", "5", "6", "7", "8", "10", "12", 
    #   "16"
    #   ]

    base_path = "test_output/"
    end = "_sys.pckl"

    frames = 0
    cameras = range(1,11)

    target_device = "sda7"

    # f = open("gt16mp.txt", "r")
    # images = []
    # for line in f:
    #     images.append(line.split(".")[0])
    # f.close()

    # for x in range(len(dir_names)):
    #     dir_name = dir_names[x]
    #     base = filesize[x] + ".txt"
    #     print base

    for x in cameras:
        path = str(frames) + "_" + str(x) + end

        # sysfile = "sys_io_counters_" + base
        # sys_iofile = open(sysfile, 'w')
        # sys_iofile.write(set_io_header())

        # procfile = "io_counters_" + base
        # iofile = open(procfile, 'w')
        # iofile.write(set_io_header())

        # proccpu = "avg_cpu_" + base
        # cpufile = open(proccpu, 'w')
        # cpufile.write(set_cpu_header())

    #     target_device = "sda7"

        sys_io_nums = ""
        io_nums = ""
        cpu_percents = "" 
    #     for i in images: 
    #         sys_io_nums += i + ", " + filesize[x] + ", "
    #         io_nums += i + ", " + filesize[x] + ", "
    #         cpu_percents += i + ", " + filesize[x] + ", "

        sys_io_nums, io_nums, cpu_percents = get_write_data(base_path, dir_name, path, end, target_device, sys_io_nums, io_nums, cpu_percents)
    #         sys_io_nums, io_nums, cpu_percents = get_read_data(base_path, dir_name, i, end, target_device, sys_io_nums, io_nums, cpu_percents)
    #         sys_io_nums, io_nums, cpu_percents = get_crop_data(base_path, dir_name, i, end, target_device, sys_io_nums, io_nums, cpu_percents)
    #         sys_io_nums, io_nums, cpu_percents = get_resize_data(base_path, dir_name, i, end, target_device, sys_io_nums, io_nums, cpu_percents)

    #         sys_io_nums += "\n"
    #         io_nums += "\n"
    #         cpu_percents += "\n"

        print sys_io_nums
        print io_nums
        print cpu_percents

    #     sys_iofile.write(sys_io_nums)        
    #     sys_iofile.close()
        
    #     iofile.write(io_nums)        
    #     iofile.close()
        
    #     cpufile.write(cpu_percents)        
    #     cpufile.close()
    
if __name__ == "__main__":
    main()

