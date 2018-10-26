#!/bin/bash

BASE_DIR=${HOME}/prog/thread_manager


export LD_LIBRARY_PATH=${BASE_DIR}/threadmgr:${BASE_DIR}/threadmgrpp

${BASE_DIR}/testpp/testpp

