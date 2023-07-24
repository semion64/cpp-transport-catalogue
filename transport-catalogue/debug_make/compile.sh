#!/bin/bash
cmake ../src/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/admn/Documents/git_repos/current_tasks/serialization/protobuf_lib/Protobuf/
cmake --build .
#./transport_catalogue
#sudo g++ -fsanitize=address -fsanitize=undefined -std=c++17 main.cpp
