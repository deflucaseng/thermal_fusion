#!/bin/bash

# Create a new tmux session named 'thermal'
tmux new-session -d -s thermal

# Run sender in first pane
tmux send-keys -t thermal 'make clean && make && make run_sender' C-m

# Split window horizontally and run receiver in second pane
tmux split-window -h -t thermal
tmux send-keys -t thermal 'sleep 8 & make run_receiver' C-m

# Attach to the session
tmux attach-session -t thermal