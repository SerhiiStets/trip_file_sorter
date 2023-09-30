import datetime
import os


def is_valid_date(date_string):
    try:
        datetime.datetime.strptime(date_string, '%Y%m%d')
        return True
    except ValueError:
        return False


def rename_file(old_name, new_name):
    try:
        os.rename(old_name, new_name)
        print(f"File renamed from {old_name} to {new_name}")
    except Exception as e:
        print(f"Failed to rename file: {e}")


def convert_to_seconds(date_string):
    date_object = datetime.datetime.strptime(date_string, '%Y%m%d')
    return int(date_object.timestamp())
