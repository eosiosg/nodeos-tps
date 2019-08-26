# -*- coding: utf-8-*-
# python3 monitor-head-lib-mem-cpu.py <UniqueFlags> <NodeosLogFile>
"""
1. 监控CPU/MEM
2. 监控HEAD - LIB的差值
"""

import psutil
import sys
import requests
from time import time, sleep
from os import popen
from datetime import datetime
from time import mktime


# 测试环境(包括集群1、集群2、TestNet)请使用47.75.198.231:8386, BOS 生产环境请使用47.75.198.231:8486
HOST_PORT = "47.75.198.231:8386"

UTC_8_TIME_DIFF = 8*60*60*(10**9)

last_time = 0


def cpu_mem_stat(uflag):
    cpu = psutil.cpu_percent()
    mem = psutil.virtual_memory()
    mem_available = mem.available*100/float(mem.total)
    mem_used = mem.used*100/float(mem.total)
    mem_free = mem.free*100/float(mem.total)
    mem_buffer = mem.buffers*100/float(mem.total)
    mem_cached = mem.cached*100/float(mem.total)
    mem_shared = mem.shared*100/float(mem.total)
    now = int(time()*(10**6))*(10**3)
    out = "stat,label=%s,type=cpu p=%f %d\n" % (uflag, cpu, now)
    out += "stat,label=%s,type=memAvailable p=%f %d\n" % (uflag, mem_available, now)
    out += "stat,label=%s,type=memUsed p=%f %d\n" % (uflag, mem_used, now)
    out += "stat,label=%s,type=memFree p=%f %d\n" % (uflag, mem_free, now)
    out += "stat,label=%s,type=memBuffer p=%f %d\n" % (uflag, mem_buffer, now)
    out += "stat,label=%s,type=memCached p=%f %d\n" % (uflag, mem_cached, now)
    out += "stat,label=%s,type=memShared p=%f %d\n" % (uflag, mem_shared, now)
    return out


# 参数传入result[-1]
def get_time(line):
    try:
        time_str = line.split('@')[1].split()[0]
        t = datetime.strptime(time_str, '%Y-%m-%dT%H:%M:%S.%f')
        return True, int((mktime(t.timetuple())*(10**6) + t.microsecond)*(10**3) + UTC_8_TIME_DIFF)
    except Exception as e:
        print(e)
        return False, 0


def head_lib_stat(uflag, log_file):
    out = ""
    global last_time
    for i in range(5):
        cmd = "tail -20 %s |grep ' signed by '" % log_file
        with popen(cmd) as execute:
            result = execute.readlines()
        if len(result) == 0:
            continue
        producer_set = set()
        r, this_time = get_time(result[-1])
        # 获取时间失败，或者说这次的时间和上次的时间相同(即日志没有写入)
        if not r or this_time == last_time:
            break
        last_time = this_time
        for line in result[::-1]:
            try:
                producer = line.split(' signed by ')[1].split()[0]
                if producer in producer_set:
                    continue
                producer_set.add(producer)
                _, t = get_time(result[-1])
                lib = int(line.split('lib: ')[1].split(',')[0])
                head = int(line.split("#")[1].split()[0])
            except ValueError as e:
                print(e)
                continue
            diff = head - lib
            out = "stat,label=%s,producer=%s,type=headLibDiff p=%d %d" % (uflag, producer, diff, t)
        return out
    return ""  # tail -100 连续5次都未拿到数据


def main():
    if len(sys.argv) != 3:
        print("Usage: python monitor-head-lib-mem-cpu.py <UniqueFlags> <NodeosLogFile>.\n")
        print("<UniqueFlags> may be hostname, ip or something that can be recongnized.")
        print("<NodeosLogFile> may be /var/log/nodeos/nodeos.log or other you specified.")
        exit(0)
    url = "http://%s/write?db=boscore" % HOST_PORT
    unique_flag = sys.argv[1]
    nodeos_log_file = sys.argv[2]
    while True:
        try:
            out = cpu_mem_stat(unique_flag)
            r = head_lib_stat(unique_flag, nodeos_log_file)
            out += r
            requests.post(url=url, data=out)
            print(r)
        except Exception as e:
            print(e)
        sleep(1)


if '__main__' == __name__:
    main()
