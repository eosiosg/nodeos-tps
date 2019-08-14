from influxdb import InfluxDBClient
from time import time, sleep
import requests
import json
from config_client import AUTH_CODE, REPORT_URL


MESSAGE_TEMPLATE = {
        'type': 1,
        'code': AUTH_CODE,
        'node': 'xxxxx',
        'extra': 'xxxxx'
    }

headers = {'content-type': "application/json"}


def get_all_node(cli):
    r = cli.query('show tag values from stat with key = label')
    data = list()
    for x in r.get_points():
        data.append(x['value'])
    print(data)
    return data


def node_avg_lib_diff_warn(cli, node):
    # 连续1分钟每10s的head-lib差值的平均值有50%大于100
    qstr = 'SELECT mean("p") FROM "stat" WHERE ("type" =' + "'headLibDiff'" + \
           'AND "label" = ' + "'%s'" % node + ') AND time >= now() - 1m GROUP BY time(10s)'
    r = cli.query(qstr)
    count = 0.0
    total_count = 0.0
    max_mean = 0
    for x in r.get_points():
        if x['mean'] is not None and x['mean'] >= 10:
            count += 1.0
            if x['mean'] > max_mean:
                max_mean = x['mean']
        total_count += 1.0
    MESSAGE_TEMPLATE['extra'] = 'diff is {}'.format(max_mean)
    if count/total_count > 0.1:
        return True
    return False


def report_to_server(node):
    print('report to server, ', time())
    MESSAGE_TEMPLATE['node'] = node
    requests.post(REPORT_URL, data=json.dumps(MESSAGE_TEMPLATE), headers=headers)
    sleep(1)


def detect_alarm(nodes, client):
    for node in nodes:
        if node_avg_lib_diff_warn(client, node):
            report_to_server(node)


def main():
    client = InfluxDBClient('47.75.198.231', '8486', database='boscore')
    nodes = get_all_node(client)
    prev_time = time()
    while True:
        try:
            detect_alarm(nodes, client)
        except Exception as e:
            print(e)
            client = InfluxDBClient('47.75.198.231', '8486', database='boscore')
            nodes = get_all_node(client)
            prev_time = time()
            print('exception.')
        if time() - prev_time > 600:
            nodes = get_all_node(client)
            prev_time = time()
        sleep(10)


if __name__ == '__main__':
    main()
