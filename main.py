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

# def run_cpp_program():
#     sys.stdout.reconfigure(encoding='utf-8')
    
#     try:
#         print("Running subprocess.")
#         result = subprocess.Popen(
#             ['./search', "./log/frontier/list", "./log/frontier/bloomfilter.bin"], 
#             stdout=subprocess.PIPE, 
#             stderr=subprocess.PIPE, 
#             text=True
#         )

#         oldtext = ""
#         linesRead = 0
#         os.set_blocking(result.stdout.fileno(), False)

#         while True:
#             try:
#                 sleep(10)
#                 text = result.stdout.read()
#                 if text == oldtext:
#                     print("Program stalled. exiting...")
#                     result.kill()
#                     return 1
#                 linesRead += len([n for n in text.splitlines() if "http" in n])
#             except Exception as e:
#                 print(f"Error reading stdout: {e}")
#                 result.kill()
#                 break
#             oldtext = text
#             print(" | " + str(linesRead) + " sites indexed.")
#             if result.poll() is not None:
#                 break
            
#         print(f"Process finished with exit code: {result.returncode}")
#     except subprocess.CalledProcessError:
#         print("Error executing subprocess.")
#         return 1
#     except FileNotFoundError:
#         print("Error: subprocess file not found")
#         return 1
#     except Exception as e:
#         print(f"Unknown error: {e}")
#         return 1
#     return 0

# def loop():
#     while True:
#         run_cpp_program()



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
    