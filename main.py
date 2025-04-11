import os
import subprocess
from time import sleep
import codecs
import sys


# Retrieve Job-defined env vars
TASK_INDEX = os.getenv("CLOUD_RUN_TASK_INDEX", 0)
TASK_ATTEMPT = os.getenv("CLOUD_RUN_TASK_ATTEMPT", 0)
# Retrieve User-defined env vars
SLEEP_MS = os.getenv("SLEEP_MS", 0)
FAIL_RATE = os.getenv("FAIL_RATE", 0)


LOG_FILE = 'out'

def run():
    sys.stdout.reconfigure(encoding='utf-8')
    
    result = subprocess.Popen(
        ['./search', "./log/frontier/list", ">", LOG_FILE], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True
    )



def main():
    run()

if __name__ == '__main__':
    main()
    