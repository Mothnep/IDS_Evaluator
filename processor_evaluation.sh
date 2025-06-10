arm-linux-gnueabi-g++ -std=c++17 -static -Wall -Wextra -O2 -o iforest_arm algorithms/Iforets_on_OPS-SAT.cpp ../helper/helper_functions.cpp -I../lib/LibIsolationForest/cpp -I..

simulators/gem5/build/ARM/gem5.opt simulators/simulator_configs/gem5/ARM/prog.py