//
// Created by oscar on 30/11/2021.
//

#include <string>
#include <list>
#include <fstream>
#include <sstream>
#include <utility>
#include <iostream>
#include <cstring>
#include <TH2D.h>
#include "AnalysisProcess.h"

AnalysisProcess::AnalysisProcess(){
    word0 = 0;
    word1 = 0;

    for(auto i = 0; i < NUMBER_ADC_CHANNELS; i++){
        adcChannelOffset[i] = 0.;
        adcChannelGains[i] = 1.0;
    }
    dataLength = 0;
    positionInBlock = 0;
    eventNumber = 0;
    isPulserEvent = false;
}

int AnalysisProcess::ReadParameters(std::string &parameterFile) {
    std::string line;
    std::ifstream parameters(parameterFile.data());
    int channel;
    double value;
    if (parameters.bad()){
        return -1;
    }
    while (parameters.good()){
        getline(parameters, line);
        auto commentLine=line.find("#");
        std::string dummyVar;
        auto newLine = line.substr(0,commentLine);
        if(newLine.empty()){
            std::istringstream iss(line,std::istringstream::in);
            iss >> dummyVar;

            if(dummyVar == "adcOffset"){
                iss >> channel;
                iss >> value;
                adcChannelOffset[channel] = value;
            }
            else if(dummyVar == "adcGain"){
                iss >> channel;
                iss >> value;
                adcChannelGains[channel] = value;
            }
            else continue;
        }
    }
    parameters.close();
    return 0;
}

int AnalysisProcess::BeginAnalysis(std::list<std::string> alphaFiles) {
    alphaFileList = std::move(alphaFiles);
    if(!OpenInputFile()){
        return -1;
    }
    ReadBlockHeader();
    positionInBlock = 0;
    while( positionInBlock < dataLength){
        //Read event header
        word0 = (blockData[positionInBlock + 0] & 0xFF) | ((blockData[positionInBlock + 1] & 0xFF) << 8);
        word1 = (blockData[positionInBlock + 2] & 0xFF) | ((blockData[positionInBlock + 3] & 0xFF) << 8);
        if (!(word0 ^ 0xffff)){
            std::cout << "Event header not read properly" << std::endl;
            return -1;
        }
        eventLength = word1;
        //Read in event
        for(int itrdata = positionInBlock+4; itrdata < positionInBlock+eventLength; itrdata += 4){
            word0 = (blockData[itrdata] & 0xFF) | ((blockData[itrdata + 1] & 0xFF) << 8);
            word1 = (blockData[itrdata + 2] & 0xFF) | ((blockData[itrdata + 3] & 0xFF) << 8);

            dataWords.first = word0;
            dataWords.second = word1;

            dataWordsList.emplace_back(dataWords);
        }
        positionInBlock += eventLength;
        eventNumber++;


    }

    return 0;
}

int AnalysisProcess::OpenInputFile() {
    currentInputFile = alphaFileList.front();
    alphaFileList.pop_front();
    inputFile.open(currentInputFile.data());

    if(!inputFile.is_open()){
        std::cout << "Problem opening: " << currentInputFile << std::endl;
        return -1;
    }
    else if(inputFile.is_open()){
        std::cout << "Input file: " << currentInputFile << " opened" << std::endl;
        return 0;
    }
    else{
        std::cout << "Something strange happening in DataReader::OpenInputFile" << std::endl;
        return -1;
    }
}

int AnalysisProcess::ReadBlockHeader() {
    inputFile.read((char*)&blockHeader,sizeof(blockHeader));

    // http://npg.dl.ac.uk/MIDAS/MIDASDataAcquisition/base.html for documentation on header
    if( strncmp(blockHeader, "EBYEDATA", 8) == 0){
        std::cout << "Block header not EBYEDATA" << std::endl;
        return -1;
    }

    dataLength = (blockHeader[20] & 0xFF) | (blockHeader[21] & 0xFF) << 8 |
                 (blockHeader[22] * 0xFF) << 16 | (blockHeader[23] << 24);

    inputFile.read((char*)&blockData, dataLength);
}

int AnalysisProcess::ProcessEvent() {
    outputEvent.ClearEvent();
    isPulserEvent = false;
    if (dataWordsList.size()>64){
        //Over half the channels fire is a pulser event
        isPulserEvent = true;
    }
    for(dataWordsListIt = dataWordsList.begin(); dataWordsListIt != dataWordsList.end(); dataWordsListIt++){
        unpackedItem.UpdateItem(*dataWordsListIt, eventNumber);
        if(unpackedItem.GetGroup() < 20){
            //Is adc event
            itemChannel = unpackedItem.GetItem() + (unpackedItem.GetGroup() - 1) * 64;
            itemValue = (double)unpackedItem.GetDataWord() * adcChannelGains[itemChannel] - adcChannelOffset[itemChannel];
            outputEvent.AddToEvent(true, itemChannel, itemValue);

            rawADCEnergyVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            calibratedADCEnergyVsChannel->Fill(itemChannel, itemValue);
            if(isPulserEvent){
                pulserVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            }
        }
        else {
            //Is TDC event
            itemChannel = unpackedItem.GetItem() + (unpackedItem.GetGroup() - 1) * 64;
            itemValue = (double)unpackedItem.GetDataWord();
            outputEvent.AddToEvent(false, itemChannel, itemValue);
            rawTDCVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            calibratedTDCVsChannel->Fill(itemChannel, itemValue);
        }


    }
    outputTree->Fill();
    return 0;
}

int AnalysisProcess::OpenOutputFile(std::string outputFile) {

    outF = TFile::Open(outputFile.c_str(), "RECREATE");
    if(!outF){
        std::cout << "Problem opening output file. Check input." << std::endl;
        return -1;
    }
    outputTree = new TTree("doubleAlpha","doubleAlpha");
    outputTree->Branch("double_alpha", &outputEvent, "eventNumber/l:adcChannels[96]/D:tdcChannels/D");
    return 0;
}

int AnalysisProcess::DefineHistograms() {

    rawADCEnergyVsChannel = new TH2D("Raw_ADC_Energy_Vs_Channel","",96,0,96,4096,0,4096);
    calibratedADCEnergyVsChannel = new TH2D("Calibrated_ADC_Energy_Vs_Channel","",96,0,96,4096,0,4096);

    rawTDCVsChannel = new TH2D("Raw_TDC_Vs_Channel","",128,0,128,4096,0,4096);
    calibratedTDCVsChannel = new TH2D("Calibrated_TDC_Vs_Channel","",128,0,128,4096,0,4096);
    pulserVsChannel = new TH2D("Pulser_Vs_Channel","",96,0,96,4096,0,4096);

    return 0;
}

int AnalysisProcess::CloseAnalysisProcess() {

    rawADCEnergyVsChannel->Write();
    calibratedADCEnergyVsChannel->Write();
    rawTDCVsChannel->Write();
    calibratedTDCVsChannel->Write();
    pulserVsChannel->Write();
    outF->Write();

    outF->Close();

    return 0;
}
