#!/bin/bash

# Default configuration
k_values=(3 4 5 7 9 10)
nn_start=10
nn_end=16
nn_step=2

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -start)
            nn_start="$2"
            shift 2
            ;;
        -end)
            nn_end="$2"
            shift 2
            ;;
        -step)
            nn_step="$2"
            shift 2
            ;;
        *)
            echo "Error: Unknown option $1"
            exit 1
            ;;
    esac
done

# Validate numeric values
validate_number() {
    local num="$1"
    if ! [[ "$num" =~ ^[0-9]+$ ]]; then
        echo "Error: Invalid numeric value: $num"
        exit 1
    fi
}

validate_number "$nn_start"
validate_number "$nn_end"
validate_number "$nn_step"

generate_nn_sequence() {
    seq $nn_start $nn_step $nn_end
}

print_config() {
    echo "=== Test Configuration ==="
    echo "Participant counts  : ${k_values[@]}"
    echo "Dataset exponents   : [$nn_start, $nn_end] step $nn_step"
    echo "Total test cases    : $(( ${#k_values[@]} * $(generate_nn_sequence | wc -w) ))"
    echo "=========================="
    echo
}

run_test_phase() {
    local phase=$1
    local k=$2
    local nn=$3
    
    echo "[$phase] Starting $k processes..."
    for ((r=0; r<k; r++)); do
        ./main -$phase -k $k -nn $nn -r $r &
    done
    wait
    echo "[$phase] All processes completed"
}

main() {
    print_config
    
    for k in "${k_values[@]}"; do
        for nn in $(generate_nn_sequence); do
            echo -e "\n=== TEST CASE: k=$k nn=$nn ==="
            
            # Phase 1: preGen
            run_test_phase "preGen" $k $nn
            
            # Phase 2: psu
            run_test_phase "psu" $k $nn
            
            echo -e "=== END CASE: k=$k nn=$nn ===\n"
        done
    done
}

# Entry point
main
