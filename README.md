# doubleAlphaSort
*A sort program written to unpack the data for the doubleAlpha experiment*
## Program aims
- Take the MIDAS data files produced by the DSSSD DAQ and convert them into ROOT events.
- ADC channels start from 0 with the first 64 belonging to the DSSD. Then there will be one channel for the photodiode.
- TDC channels start from 16 with the same ordering.
  - There is no TDC channel for the photodiode
## Program usage
- Once the program has been built it can be run with ./doubleAlpha -c ExampleConfig.csv -o outputFile.root
- Calibration parameters for the ADC channels can be provided in ExampleParameters.csv
  - This allows you to provide gain and offset per ADC channel
- To obtain the rate in a given channel it can be done by producing a plot of the scaler channel which contains the pulser (To be added later) and placing a condition requiring and ADC value greater than 0 in your chosen channel.
- Further instructions with some example histogram commands will be put on the mattermost channel.
## Prerequisites
- [CERN ROOT](https://root.cern/) (Tested with ROOT 6.24 and C++14)
> May work with C++11 and other versions of ROOT. Just make sure that the C++ standard defined in CMakeLists.txt is the same as the version that ROOT was compiled with on your system.
> 
## Build and run instructions
1. Create build directory and enter it`mkdir build && cd build`
2. Run cmake `cmake ..`
3. make `make`
4. Run executable `./doubleAlpha -c configFile -o outputFile`
