# 请联系郑万刚获取
# 新增报警类型需要操作的步骤:
#   1. 添加"alarm type from client";
#   2. 在alarm_service_name_map中添加对应的项;
#   3. 在PHONE_NUMBER_DICT中添加对应的项;

# error code to client
SUCCESS = 0
PARAMETER_ERROR = 1
AUTH_FAILED_ERROR = 2
INVALID_TYPE_ERROR = 3

# alarm type from client
TEST_ALARM = 0
HEAD_LIB_DIFF_HIGH = 1
IBC_ALARM = 2
WPS_ALARM = 3
ES_ALARM = 4

# alarm message to user
alarm_service_name_map = {
    TEST_ALARM: 'test-alarm',
    HEAD_LIB_DIFF_HIGH: 'Head-lib-diff',
    IBC_ALARM: 'ibc-alarm',
    WPS_ALARM: 'wps-alarm',
    ES_ALARM: 'es-hypersion-alarm',
}


TWILIO_ACCESS_KEY_ID = ''
TWILIO_ACCESS_KEY_SECRET = ''
TWILIO_CALLER = ''

AUTH_CODE = ''

pn = {
}

PHONE_NUMBER_DICT = {
    # alarm_type : [phone_number_1, phone_number_2,]),
    TEST_ALARM:                    [],
    HEAD_LIB_DIFF_HIGH: [],
    IBC_ALARM:          [],
    WPS_ALARM:          [],
    ES_ALARM:           [],
}
