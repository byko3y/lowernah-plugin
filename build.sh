#!/bin/sh

echo 'mkdir build && cd build'
echo 'cmake .. -DPSI_PLUGIN_INCLUDE=../psi/src/plugins/include'
echo 'make'
