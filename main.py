import psutil
import subprocess
from time import sleep
import sys
import os
import signal

THRESHOLD = 70

def kill(pid):
    os.kill(pid, signal.SIGINT)
    sleep(60)
    os.kill(pid)

def run():
    sys.stdout.reconfigure(encoding='utf-8')
    process = subprocess.Popen(
        ['nohup'] + ["./run_script.sh"],
        stdout=open('nohup.out', 'w'),
        stderr=open('nohup.out', 'a'),
        preexec_fn=lambda: signal.signal(signal.SIGHUP, signal.SIG_IGN),
        start_new_session=True
    )
    # TODO: live output of nohup.out, kill process when python process is killed, also nohup python process
    file_path = "nohup.out"
    pid = process.pid

    old_size = 0
    new_size = 0
    new_mem = 0
    
    while True:
        sleep(30)
        new_size = os.path.getsize(file_path)
        if (new_size < old_size + 20):
            print("Crawler stalled. Restarting...")
            kill(pid)
        process_psutil = psutil.Process(pid)
        memory_info = process_psutil.memory_info()
        new_mem = memory_info.rss / (1024**3)
        if (new_mem > THRESHOLD):
            print("Crawler exceeded memory threshold. Restarting...")
            kill(pid)
            return

def main():
    while True:
        subprocess.run(["make", "clean"])
        subprocess.run(["make"])
        print("Running crawler.")
        run()

if __name__ == '__main__':
    main()
    