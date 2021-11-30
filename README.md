# doubleAlphaSort
*A sort program written to unpack the data for the doubleAlpha experiment*
## Program aims
- Take the MIDAS data files produced by the DSSSD DAQ and convert this from the MIDAS data format into ROOT events
## Prerequisites
- C++14
- [CERN ROOT](https://root.cern/) (Tested with ROOT 6.24) 
## Build and run instructions
1. Create build directory and enter it`mkdir build && cd build`
2. Run cmake `cmake ..`
3. make `make`
4. Run executable `./doubleAlpha -c configFile -o outputFile`
