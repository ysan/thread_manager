#!/bin/bash

#BASE_DIR=${HOME}/prog/thread_manager
BASE_DIR=./


export LD_LIBRARY_PATH=\
${BASE_DIR}/../threadmgr:\
${BASE_DIR}/../threadmgrpp:\
${BASE_DIR}/../testpp/module_a:\
${BASE_DIR}/../testpp/module_b:\
${BASE_DIR}/../testpp/module_c:\


${BASE_DIR}/testpp

