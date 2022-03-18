//
// Created by oscar on 30/11/2021.
//

#ifndef DOUBLEALPHA_ANALYSISPROCESS_H
#define DOUBLEALPHA_ANALYSISPROCESS_H


#include <cstdint>
#include <deque>
#include <TFile.h>
#include <TTree.h>
#include "TH2D.h"
#include "dataItems.cpp"

class AnalysisProcess {
private:
    int OpenInputFile();
    int ReadBlockHeader();
    int ProcessEvent();

    // variables for reading in data
    static const int HEADER_SIZE = 24; // Number of bytes in the header block
    char blockHeader[HEADER_SIZE]{};
    char blockData[0x10000-24]{}; //max size of block is 64kB
    int dataLength;
    unsigned long fileSize;
    unsigned long positionInFile;
    std::pair<uint16_t , uint16_t> dataWords;
    int positionInBlock;
    std::deque<std::pair<int16_t, int16_t>> dataWordsList;
    std::deque<std::pair<int16_t, int16_t>>::iterator dataWordsListIt;
    uint16_t word0, word1;
    uint16_t eventLength{};

    std::ifstream inputFile;
    std::list <std::string> alphaFileList; //List of the files to be sorted
    std::string currentInputFile;
    unsigned long eventNumber;
    // variables for unpacking and calibrating data
    UnpackedItem unpackedItem;
    OutputEvent outputEvent;
    int itemChannel{};
    double itemValue{};
    static const int NUMBER_ADC_CHANNELS = 200;
    double adcChannelGains[NUMBER_ADC_CHANNELS]{};
    double adcChannelOffset[NUMBER_ADC_CHANNELS]{};

    unsigned long scalerBase[16];
    unsigned long previousScaler[16];
    unsigned long scalerWords[32] = {0};


    // Variables for outputting data
    TFile * outF{};
    bool isPulserEvent;
    TTree * outputTree;
    unsigned long pulserNumber; //Tracks number of pulsers seen in the analysis


    // Variables for histograms
    TH2D * rawADCEnergyVsChannel;
    TH2D * rawTDCVsChannel;
    TH2D * calibratedADCEnergyVsChannel;
    TH2D * calibratedTDCVsChannel;
    TH2D * pulserVsChannel;


public:
    AnalysisProcess();
    ~AnalysisProcess()= default;
    int ReadParameters(std::string parameterFile);
    int BeginAnalysis(std::list<std::string> alphaFiles);
    int OpenOutputFile(std::string outputFile);
    int DefineHistograms();
    int CloseAnalysisProcess();

};


#endif //DOUBLEALPHA_ANALYSISPROCESS_H
