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
SLEEP_INTERVAL = 30
MAX_CHUNKS = 100

def opensp():
    return subprocess.Popen(
        ['./search', "./log/frontier/list", "./log/frontier/bloomfilter.bin", ">", LOG_FILE], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True
    )

def run():
    sys.stdout.reconfigure(encoding='utf-8')
    result = opensp()

    oldln = 0
    newln = 0
    
    while True:
        sleep(SLEEP_INTERVAL)
        newln = len(os.listdir("./log/chunks"))
        if newln == oldln:
            print("Process stalled. Restarting...")
            result.terminate()
            sleep(SLEEP_INTERVAL)
            if result.poll() is None: # if subprocess hasn't shut down
                result.kill()
            result = opensp()
        elif newln > MAX_CHUNKS:
            print("Transferring chunks...")
        oldln = newln




def main():
    run()

if __name__ == '__main__':
    main()
    