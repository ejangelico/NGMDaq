# NGMDaq
Code used to control struck digitizer systems, for our nEXO stanford teststand


# Installing dependencies

1. The multithreading component of data aquisition requires TBB to be installed. An open source version can be found found here: https://github.com/oneapi-src/oneTBB. After compiling, add the following to your bashrc


```
export TBBROOT="/tmp/my_installed_onetbb" #
tbb_bin="/path/to/oneTBB/build/<architecture>" #
if [ -z "$CPATH" ]; then #
    export CPATH="${TBBROOT}/include" #
else #
    export CPATH="${TBBROOT}/include:$CPATH" #
fi #
if [ -z "$LIBRARY_PATH" ]; then #
    export LIBRARY_PATH="${tbb_bin}" #
else #
    export LIBRARY_PATH="${tbb_bin}:$LIBRARY_PATH" #
fi #
if [ -z "$LD_LIBRARY_PATH" ]; then #
    export LD_LIBRARY_PATH="${tbb_bin}" #
else #
    export LD_LIBRARY_PATH="${tbb_bin}:$LD_LIBRARY_PATH" #
fi #
```

2. Install cern root using the conda installer. Presently, 6.22 root has been tested. 

# Building NGMDaq
For ROOT 6-6.24, we need to compile with c++17 it seems? 

If your directory structure is assumed to be 
```
---software/ngm_sw/
------ NGMDaq
``` 

then perform the following steps
```
cd software/ngm_sw/
mkdir build_ngm_daq
mkdir install_ngm_daq
```

and insert (and source) an export line in your ~/.bashrc that reads
```
export NGM=/path/to/software/ngm_sw/install_ngm_daq/
export LD_LIBRARY_PATH=$NGM/lib:$LD_LIBRARY_PATH
```

Build the code with
```
cd build_ngm_daq/
cmake -DCMAKE_INSTALL_PREFIX=/path/to/software/ngm_sw/install_ngm_daq /path/to/software/ngm_sw/NGMDaq
make install
```
