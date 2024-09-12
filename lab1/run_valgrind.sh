#!/bin/bash

# Run Valgrind with specified options and save output to valgrind_out
valgrind --leak-check=full --show-leak-kinds=all --verbose ./sim &> valgrind_out.yaml

# Optional: Print a message when done
echo "Valgrind has finished running. Output saved to valgrind_out.yaml"
