# -*- coding: utf-8-*-
# python3 history-head-lib-diff.py <UniqueFlags> <NodeosLogFile>

import sys
import requests
from datetime import datetime
from time import mktime


# 测试环境(包括集群1、集群2、TestNet)请使用47.75.198.231:8386, BOS 生产环境请使用47.75.198.231:8486
HOST_PORT = "127.0.0.1:8086"

UTC_8_TIME_DIFF = 8*60*60*(10**9)

last_time = 0


# 参数传入result[-1]
def get_time(line):
    try:
        time_str = line.split('@')[1].split()[0]
        t = datetime.strptime(time_str, '%Y-%m-%dT%H:%M:%S.%f')
        return True, int((mktime(t.timetuple())*(10**6) + t.microsecond)*(10**3) + UTC_8_TIME_DIFF)
    except Exception as e:
        print(e)
        return False, 0


def head_lib_stat_line(unique_flag, line):
    try:
        producer = line.split(' signed by ')[1].split()[0]
        if producer is None or producer == '':
            print('invalid line')
            return None
        _, t = get_time(line)
        lib = int(line.split('lib: ')[1].split(',')[0])
        head = int(line.split("#")[1].split()[0])
    except ValueError as e:
        print(e)
        return None
    diff = head - lib
    out = "stat,label=%s,producer=%s,type=headLibDiff p=%d %d" % (unique_flag, producer, diff, t)
    return out


def main():
    if len(sys.argv) != 3:
        print("Usage: python monitor-head-lib-mem-cpu.py <UniqueFlags> <NodeosLogFile>.\n")
        print("<UniqueFlags> may be hostname, ip or something that can be recongnized.")
        print("<NodeosLogFile> may be /var/log/nodeos/nodeos.log or other you specified.")
        exit(0)
    url = "http://%s/write?db=boscore" % HOST_PORT
    unique_flag = sys.argv[1]
    nodeos_log_file = sys.argv[2]
    with open(nodeos_log_file) as file:
        lines = file.readlines()
    count = 0
    out = ''
    for line in lines:
        try:
            out += head_lib_stat_line(unique_flag, line) + '\n'
            if out is None or out == '':
                continue
            count += 1
            if count % 100 == 0:
                requests.post(url=url, data=out)
                out = ''
                print(count)
        except Exception as e:
            print(e)


if '__main__' == __name__:
    main()
