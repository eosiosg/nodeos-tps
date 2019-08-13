from flask import Flask, request, jsonify
from config import *
from time import time
from twilio.rest import Client as TwilioClient

app = Flask(__name__)

# error code to client
SUCCESS = 0
PARAMETER_ERROR = 1
AUTH_FAILED_ERROR = 2
INVALID_TYPE_ERROR = 3

# alarm type from client
HEAD_LIB_DIFF_HIGH = 1

# alarm message to user
alarm_service_name_map = {
    HEAD_LIB_DIFF_HIGH: ('Head-lib-diff', 'head和lib的差值过大')
}

# key: 'type:{type}:node:{node}', value: last send time
last_notify_time_map = {
    HEAD_LIB_DIFF_HIGH: 0.0
}

twilio_client = TwilioClient(TWILIO_ACCESS_KEY_ID, TWILIO_ACCESS_KEY_SECRET)


def notify(type_, node, extra=None):
    body = 'service name:{}, \nnode:{}'.format(alarm_service_name_map[type_][0], node)
    if extra is not None:
        body += ' ,\nextra:{}.\n'.format(extra)
    for phone in PHONE_NUMBER_LIST:
        twilio_client.messages.create(
            body=body,
            from_=TWILIO_CALLER,
            to=phone[1])
    print(body)


@app.route('/alarm_update', methods=['POST'])
def alarm_update():
    req_data = request.get_json()
    print(req_data)
    if 'type' not in req_data or 'node' not in req_data or 'code' not in req_data:
        return jsonify({"code": PARAMETER_ERROR})
    if req_data['code'] != AUTH_CODE:
        return jsonify({"code": AUTH_FAILED_ERROR})
    if req_data['type'] not in last_notify_time_map:
        return jsonify({"code": INVALID_TYPE_ERROR})
    if 'extra' in req_data:
        extra = req_data['extra']
    else:
        extra = None
    now = time()
    key = 'type:{}:node:{}'.format(req_data['type'], req_data['node'])
    if key not in last_notify_time_map or now - last_notify_time_map[key] > 60:
        notify(req_data['type'], req_data['node'], extra)
        last_notify_time_map[key] = now

    return jsonify({"code": SUCCESS})


if __name__ == '__main__':
    app.run(debug=False)
