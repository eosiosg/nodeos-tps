from flask import Flask, request, jsonify
from config import *
from time import time
from twilio.rest import Client as TwilioClient

app = Flask(__name__)

# key: 'type:{type}:node:{node}', value: last send time
last_notify_time_map = dict()

twilio_client = TwilioClient(TWILIO_ACCESS_KEY_ID, TWILIO_ACCESS_KEY_SECRET)


def notify(type_, node, extra=None):
    body = 'service name:{}, \nnode:{}'.format(alarm_service_name_map[type_], node)
    if extra is not None:
        body += ' ,\nextra:{}.\n'.format(extra)

    for phone in PHONE_NUMBER_DICT[type_]:
        twilio_client.messages.create(
            body=body,
            from_=TWILIO_CALLER,
            to=phone)
    print(body)


@app.route('/alarm_upload', methods=['POST'])
def alarm_update():
    req_data = request.get_json()
    print(req_data)
    if 'type' not in req_data or 'node' not in req_data or 'code' not in req_data:
        return jsonify({"code": PARAMETER_ERROR})
    if req_data['code'] != AUTH_CODE:
        return jsonify({"code": AUTH_FAILED_ERROR})
    if req_data['type'] not in alarm_service_name_map:
        return jsonify({"code": INVALID_TYPE_ERROR})
    if 'extra' in req_data:
        extra = req_data['extra']
    else:
        extra = None
    now = time()
    k = 'type:{}:node:{}'.format(req_data['type'], req_data['node'])
    if k not in last_notify_time_map or now - last_notify_time_map[k] > 60:
        notify(req_data['type'], req_data['node'], extra)
        last_notify_time_map[k] = now

    return jsonify({"code": SUCCESS})


if __name__ == '__main__':
    app.run(debug=False)
