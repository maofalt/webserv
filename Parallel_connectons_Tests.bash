#!/bin/bash

# Define colors using tput
bold=$(tput bold)
normal=$(tput sgr0)
blue=$(tput setaf 4)
red=$(tput setaf 1)
separator="${bold}----------------------------------${normal}"

# Number of requests to be made
NUM_REQUESTS=100

# Initialize variables to track times
declare -A min_times
declare -A max_times
total_taken_time=0
num_successful_requests=0

perform_request() {
    local url="$1"
    local id="$2"

    # Capture start time
    start_time=$(date +%s%3N)  # Time with milliseconds in pure numeric format

    result=$(curl -s -w "%{time_total}" -o /dev/null "$url" 2>&1)

    # Capture end time
    end_time=$(date +%s%3N)

    # Print the separator
    echo -e "$separator"

    if [ $? -ne 0 ]; then
        echo "${red}Request $id failed.${normal}"
    else
        # Update timings
        ((num_successful_requests++))
        total_taken_time=$(echo "$total_taken_time + $result" | bc)
        
        min_times["start"]=$( [ ! -v min_times["start"] ] && echo "$start_time" || echo $(min ${min_times["start"]} $start_time) )
        max_times["start"]=$( [ ! -v max_times["start"] ] && echo "$start_time" || echo $(max ${max_times["start"]} $start_time) )
        
        min_times["end"]=$( [ ! -v min_times["end"] ] && echo "$end_time" || echo $(min ${min_times["end"]} $end_time) )
        max_times["end"]=$( [ ! -v max_times["end"] ] && echo "$end_time" || echo $(max ${max_times["end"]} $end_time) )

        # Display results
        echo "Request $id: Started at $(date -d @"$(echo "$start_time/1000" | bc)" +"%T.%3N"), Ended at $(date -d @"$(echo "$end_time/1000" | bc)" +"%T.%3N"). ${bold}${blue}Time taken: $result seconds${normal}"
    fi
}

min() {
    echo $(($1<$2?$1:$2))
}

max() {
    echo $(($1>$2?$1:$2))
}

for i in $(seq 1 $NUM_REQUESTS)
do
   perform_request "http://localhost:8694/" "$i" &
done

wait

# Print summary
echo -e "$separator"
echo "Summary:"
echo "Earliest Start Time: $(date -d @"$(echo "${min_times["start"]}/1000" | bc)" +"%T.%3N")"
echo "Latest Start Time:   $(date -d @"$(echo "${max_times["start"]}/1000" | bc)" +"%T.%3N")"
echo "Earliest End Time:   $(date -d @"$(echo "${min_times["end"]}/1000" | bc)" +"%T.%3N")"
echo "Latest End Time:     $(date -d @"$(echo "${max_times["end"]}/1000" | bc)" +"%T.%3N")"
if [ $num_successful_requests -ne 0 ]; then
    average_time=$(echo "scale=3; $total_taken_time/$num_successful_requests" | bc)
    echo "${bold}${blue}Average Time Taken: $average_time seconds${normal}"
else
    echo "${red}No successful requests to compute average time.${normal}"
fi
