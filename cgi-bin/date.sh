#!/bin/bash
# system_info.sh

echo "Content-type: text/plain"
echo ""
echo "System Information:"
echo "-------------------"
echo "Date: $(date)"
echo "Operating System: $(uname -a)"
echo "Uptime: $(uptime)"
echo "Disk Usage:"
df -h
