import datetime


def is_valid_date(date_string):
    try:
        datetime.datetime.strptime(date_string, '%Y%m%d')
        return True
    except ValueError:
        return False


def convert_to_seconds(date_string):
    date_object = datetime.datetime.strptime(date_string, '%Y%m%d')
    return int(date_object.timestamp())
