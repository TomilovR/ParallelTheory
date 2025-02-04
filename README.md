# ConcurrencyTheory
To change the precision type, modify line 10 in CMakeLists.txt
file. Currently, it reads: COMMAND main f
This line runs your program with the argument "f" for float precision. If you want to use double, change it to:  COMMAND main d
After making this change, rebuild the project to run with the updated precision setting.