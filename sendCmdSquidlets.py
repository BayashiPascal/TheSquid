# Import necessary modules
import os
import sys
try:
  from fabric2 import Connection
except Exception as exc:
  print("Fabric not installed. Try: sudo pip3 install fabric2")
  exit(0)

# Base directory
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Function to print exceptions
def print_exc(exc):
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno, str(exc))

# Hook for the main function
if __name__ == '__main__':
    try:

        result = Connection('web1.example.com').run('uname -s', hide=True)
        msg = "Ran {0.command!r} on {0.connection.host}, got stdout:\n{0.stdout}"
        print(msg.format(result))

    except Exception as exc:
        print_exc(exc)
