import ctypes
import sys
import argparse
import os
import re
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


def main():
    parser = argparse.ArgumentParser(description='Process directory and dates.')
    parser.add_argument('directory', type=str, help='Path to the directory')
    parser.add_argument('start_date', type=str, help='Start date in YYYYMMDD format')
    parser.add_argument('end_date', type=str, help='End date in YYYYMMDD format')

    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print('Error: Invalid directory path.')
        return

    if not is_valid_date(args.start_date) or not is_valid_date(args.end_date):
        print('Error: Invalid date format. Please use YYYYMMDD format.')
        return

    print('Directory:', args.directory)
    print('Start Date:', args.start_date)
    print('End Date:', args.end_date)

    # Load the shared library
    sorter = ctypes.CDLL('./sorter.so')  # Specify the correct path to your shared library

    # Define the function signature
    process_directory = sorter.process_directory
    process_directory.argtypes = [ctypes.c_char_p, ctypes.c_long, ctypes.c_long]
    process_directory.restype = ctypes.c_int

    # Define start_time and end_time variables
    start_time = convert_to_seconds(args.start_date)
    end_time = convert_to_seconds(args.end_date)

    # Call the C function
    result = process_directory(bytes(args.directory, 'utf-8'), start_time, end_time)

    print(f'Result from C function: {result}')


if __name__ == "__main__":
    main()
