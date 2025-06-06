#!/bin/bash


process_name="search" # Replace with the actual process name
memory_limit_mb=70000000 # Memory limit in MB
sleep_interval=30 # Check interval in seconds
reset_interval=300 


cleanup() {
  echo "Terminating other processes..."
  sleep $reset_interval
  pkill "$process_name"
  exit 0 # Exit the script after cleanup
}

trap cleanup EXIT

ulimit -n 65000
nohup ./run_script.sh & > nohup.out

while true; do
  sleep $sleep_interval
  if pgrep -x "$process_name" > /dev/null; then
    echo "$(date) - Process '$process_name' is running."
  else
    echo "$(date) - Process '$process_name' is not running. Restarting..."
    nohup ./run_script.sh & > nohup.out
    sleep $sleep_interval
    echo "$(date) - Process '$process_name' restarted."
  fi
  current_memory_usage=$(cat /proc/$(pgrep "$process_name")/status | grep VmRSS)
  current_memory_int=$(echo "$current_memory_usage" | sed 's/[^0-9]*//g' )

  if [[ -z "$current_memory_usage" ]]; then
    echo "Process '$process_name' not found. Exiting."
    exit 1
  fi

  if (( $(echo "$current_memory_int > $memory_limit_mb" | bc -l) )); then
    echo "$(date) - Memory usage of '$process_name' is $current_memory_usage MB, exceeding limit of $memory_limit_mb MB. Restarting..."

    pkill -SIGINT "$process_name"
    sleep $reset_interval
    pkill "$process_name"

    nohup ./run_script.sh & > nohup.out
    echo "$(date) - Process '$process_name' restarted."
  elif [ "$current_memory_int" = "$old_memory_int" ]; then
    echo "$(date) - '$process_name' stalled. Restarting..."

    pkill -SIGINT "$process_name"
    sleep $reset_interval
    pkill "$process_name"

    nohup ./run_script.sh & > nohup.out
    echo "$(date) - Process '$process_name' restarted."
  else
    echo "$(date) - Memory usage of '$process_name' is $current_memory_usage MB. Within limit."
  fi

  old_memory_int=$(echo "$current_memory_usage" | sed 's/[^0-9]*//g' )

  
done