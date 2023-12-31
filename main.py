import ctypes
import argparse
import os
import utils
from dataclasses import dataclass


import rust_formatter

MAX_LEN = 256


@dataclass
class File:
    path: str
    new_name: str


class RenamedFile(ctypes.Structure):
    _fields_ = [
        ("path", ctypes.c_char * MAX_LEN),
        ("new_name", ctypes.c_char * MAX_LEN)
    ]


def c_solution(directory: str, start_date: str, end_date: str) -> list[File]:
    result = []
    # Load the shared library
    sorter = ctypes.CDLL('./c_formatter/c_formatter.so')  # Specify the correct path to your shared library

    # Define the function signature
    process_directory = sorter.process_directory
    process_directory.argtypes = [ctypes.c_char_p, ctypes.c_long, ctypes.c_long]
    process_directory.restype = ctypes.POINTER(RenamedFile)

    # Call the C function
    num_renamed_files = ctypes.c_int(0)
    renamed_files_ptr = process_directory(bytes(directory, 'utf-8'), start_date,
                                          end_date, ctypes.byref(num_renamed_files))

    # Access the returned struct array in Python
    num_files = num_renamed_files.value
    renamed_files = [renamed_files_ptr[i] for i in range(num_files)]

    # Print the renamed files
    for file in renamed_files:
        path = file.path.decode('utf-8').strip('\x00')
        new_name = file.new_name.decode('utf-8').strip('\x00')
        result.append(File(path, new_name))

    # Free the allocated memory in C
    sorter.free_renamed_files(renamed_files_ptr)
    return result

def rust_solution(directory: str, start_date: str, end_date: str) -> list[File]:
    files = rust_formatter.get_sorted_files(directory, start_date, end_date)
    print(files[0].file_path)



def main() -> None:
    solutions = {
        "c": c_solution,
        "rust": rust_solution
    }

    parser = argparse.ArgumentParser(description='Process directory and dates.')
    parser.add_argument('solution', type=str, choices=["c", "rust"], help='Chose solution')
    parser.add_argument('directory', type=str, help='Path to the directory')
    parser.add_argument('start_date', type=str, help='Start date in YYYYMMDD format')
    parser.add_argument('end_date', type=str, help='End date in YYYYMMDD format')

    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print('Error: Invalid directory path.')
        return

    if not utils.is_valid_date(args.start_date) or not utils.is_valid_date(args.end_date):
        print('Error: Invalid date format. Please use YYYYMMDD format.')
        return

    print('Directory:', args.directory)
    print('Start Date:', args.start_date)
    print('End Date:', args.end_date)
    # Define start_time and end_time variables
    start_date = utils.convert_to_seconds(args.start_date)
    end_date = utils.convert_to_seconds(args.end_date)

    files: list[File] = solutions[args.solution](args.directory, start_date, end_date)

    # for el in files:
    #     new_path = "/".join(el.path.split("/")[:-1]) + el.new_name
    #     utils.rename_file(el.path, new_path)



if __name__ == "__main__":
    main()
