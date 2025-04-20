#!/bin/bash
make clean
make
export NODE_IP0='127.0.0.1'
export NODE_IP1='127.0.0.1'
export NODE_ID='0'
export NUM_NODES='1'
./search ./log/frontier/list