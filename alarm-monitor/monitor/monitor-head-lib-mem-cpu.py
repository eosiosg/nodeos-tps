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


# 测试环境(包括集群1、集群2、TestNet)请使用47.75.198.231:8386, BOS 生产环境请使用47.75.198.231:8486
HOST_PORT = "47.75.198.231:8386"


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


def head_lib_stat(uflag, log_file):
    out = ""
    for i in range(5):
        cmd = "tail -20 %s |grep ' signed by '" % log_file
        with popen(cmd) as execute:
            result = execute.readlines()
        if len(result) == 0:
            continue
        producer_set = set()
        for line in result[:2:-1]:
            try:
                producer = line.split(' signed by ')[1].split()[0]
                if producer in producer_set:
                    continue
                producer_set.add(producer)
                lib = int(line.split('lib: ')[1].split(',')[0])
                head = int(line.split("#")[1].split()[0])
            except ValueError as e:
                print(e)
                continue
            diff = head - lib
            out = "stat,label=%s,producer=%s,type=headLibDiff p=%d" % (uflag, producer, diff)
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
            out += head_lib_stat(unique_flag, nodeos_log_file)
            requests.post(url=url, data=out)
        except Exception as e:
            print(e)
        sleep(1)


if '__main__' == __name__:
    main()
