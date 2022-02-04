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
#include "AnalysisProcess.h"

AnalysisProcess::AnalysisProcess(){
    word0 = 0;
    word1 = 0;

    for(auto i = 0; i < NUMBER_ADC_CHANNELS; i++){
        adcChannelOffset[i] = 0.;
        adcChannelGains[i] = 1.0;
    }
    for (auto i = 0; i < 16; i++){
        scalerBase[i] = 0;
        previousScaler[i] = 0;
    }
    dataLength = 0;
    positionInBlock = 0;
    eventNumber = 0;
    isPulserEvent = false;
    pulserNumber = 0;
    positionInFile = 0;
    fileSize = 0;
}

int AnalysisProcess::ReadParameters(std::string parameterFile) {
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
        if(newLine.size() > 0){
            std::istringstream iss(line,std::istringstream::in);
            iss >> dummyVar;
            std::cout << dummyVar << std::endl;
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
    return 1;
}

int AnalysisProcess::BeginAnalysis(std::list<std::string> alphaFiles) {
    alphaFileList = std::move(alphaFiles);
    while(!alphaFileList.empty()) {
        if (!OpenInputFile()) {
            return -1;
        }
        positionInFile = 0;
        while (positionInFile < fileSize) {
            ReadBlockHeader();
            positionInBlock = 0;
            while (positionInBlock < dataLength) {
                //Read event header
                word0 = (blockData[positionInBlock + 1] & 0xFF) | ((blockData[positionInBlock + 0] & 0xFF) << 8);
                word1 = (blockData[positionInBlock + 3] & 0xFF) | ((blockData[positionInBlock + 2] & 0xFF) << 8);

                if ((word0 ^ 0xffff)) {
                    std::cout << "Event header not read properly" << std::endl;
                    return -1;
                }
                eventLength = word1;
                if (eventLength == 0) {
                    positionInBlock += 4;
                }
                //Read in event
                for (int itrdata = positionInBlock + 4; itrdata < positionInBlock + eventLength; itrdata += 4) {
                    word0 = (blockData[itrdata + 1] & 0xFF) | ((blockData[itrdata + 0] & 0xFF) << 8);
                    word1 = (blockData[itrdata + 3] & 0xFF) | ((blockData[itrdata + 2] & 0xFF) << 8);
                    //std::cout << std::hex << word0 << " " << word1 << " " << std::dec << itrdata << std::endl;

                    dataWords.first = word0;
                    dataWords.second = word1;

                    dataWordsList.emplace_back(dataWords);
                }
                ProcessEvent();
                positionInBlock += eventLength;
                eventNumber++;

            }
            positionInFile += 0x10000;
        }
        inputFile.close();
    }

    return 1;
}

int AnalysisProcess::OpenInputFile() {
    currentInputFile = alphaFileList.front();
    alphaFileList.pop_front();
    inputFile.open(currentInputFile.data());

    if(!inputFile.is_open()){
        std::cout << "Problem opening: " << currentInputFile << std::endl;
        return 0;
    }
    else if(inputFile.is_open()){
        std::cout << "Input file: " << currentInputFile << " opened" << std::endl;

        inputFile.seekg(0, inputFile.end);
        auto endLocation = inputFile.tellg();
        inputFile.seekg(0, inputFile.beg);
        auto startLocation = inputFile.tellg();

        fileSize = endLocation - startLocation;
        std::cout << fileSize/0x10000 << " Number of blocks " <<std::endl;

        return 1;
    }
    else{
        std::cout << "Something strange happening in DataReader::OpenInputFile" << std::endl;
        return 0;
    }
}

int AnalysisProcess::ReadBlockHeader() {
    inputFile.read((char*)&blockHeader,sizeof(blockHeader));

    // http://npg.dl.ac.uk/MIDAS/MIDASDataAcquisition/base.html for documentation on header
    //if( strncmp(blockHeader, "EBYEDATA", 8) == 0){
    //    std::cout << "Block header not EBYEDATA" << std::endl;
    //    std::cout << blockHeader << " " << sizeof(blockHeader) << std::endl;
        //return -1;
   // }
    /*
   int32_t head_sequence =     dataLength = (blockHeader[8] & 0xFF) | (blockHeader[9] & 0xFF) << 8 |
                                         (blockHeader[10] & 0xFF) << 16 | ((blockHeader[11] ) << 24);
   std::cout << "Head sequence" << head_sequence << std::endl;
   std::cout << (blockHeader[8] & 0xFF) << " " << (blockHeader[9] & 0xFF) << " " << (blockHeader[10] & 0xFF) << " " << (blockHeader[11] & 0xFF) << " " << std::endl;
    std::cout << (blockHeader[12] & 0xFF) << " " << (blockHeader[13] & 0xFF) << " " << (blockHeader[14] & 0xFF) << " " << (blockHeader[15] & 0xFF) << " " << std::endl;
    std::cout << (blockHeader[16] & 0xFF) << " " << (blockHeader[17] & 0xFF) << " " << (blockHeader[18] & 0xFF) << " " << (blockHeader[19] & 0xFF) << " " << std::endl;

    int16_t header_data = (blockHeader[17] & 0xFF) | ((blockHeader[16] & 0xFF) << 8);
    std::cout << "header data " << header_data << std::endl;
    */
    dataLength = (blockHeader[23] & 0xFF) | (blockHeader[22] & 0xFF) << 8 |
                 (blockHeader[21] & 0xFF) << 16 | (blockHeader[20] << 24);
    //std::cout << dataLength << " Data length" << std::endl;


    inputFile.read((char*)&blockData, sizeof(blockData));

    return 0;
}

int AnalysisProcess::ProcessEvent() {
    outputEvent.ClearEvent();
    isPulserEvent = false;
    for(dataWordsListIt = dataWordsList.begin(); dataWordsListIt != dataWordsList.end(); dataWordsListIt++){
        unpackedItem.UpdateItem(*dataWordsListIt, eventNumber);
        if(unpackedItem.GetGroup() < 20 && unpackedItem.GetGroup() > 0){
            //Is adc event
            itemChannel = unpackedItem.GetItem() + ((unpackedItem.GetGroup() - 1) * 32);
            itemValue = ((double)unpackedItem.GetDataWord()  - adcChannelOffset[itemChannel]) * adcChannelGains[itemChannel];
            outputEvent.AddToEvent(true, itemChannel, itemValue);
            rawADCEnergyVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            calibratedADCEnergyVsChannel->Fill(itemChannel, itemValue);
            if(isPulserEvent){
                pulserVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            }
        }
        else if(unpackedItem.GetGroup() < 30 && unpackedItem.GetGroup() > 19) {
            //Is TDC event
            itemChannel = unpackedItem.GetItem() + (unpackedItem.GetGroup() - 20) * 64;
            //Common stop mode means first 16 channels of V767 module are not used
            itemChannel = itemChannel - 16 *(1+((unpackedItem.GetGroup()-20)/2));
            itemValue = (double)unpackedItem.GetDataWord();
            outputEvent.AddToEvent(false, itemChannel, itemValue);
            rawTDCVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            calibratedTDCVsChannel->Fill(itemChannel, itemValue);
        }
        else if(unpackedItem.GetGroup() == 30){
            //Is scaler event
            itemChannel = (unpackedItem.GetItem())/2;
            if(itemChannel > 16){
                std::cout << itemChannel << " " << unpackedItem.GetItem() << std::endl;
            }
            if( itemChannel < 16 && unpackedItem.GetDataWord()>0) {
                if (unpackedItem.GetDataWord() < previousScaler[itemChannel]) {
                    scalerBase[itemChannel] += 65536;
                }
                previousScaler[itemChannel] = unpackedItem.GetDataWord();
                outputEvent.AddScalerEvent(itemChannel, unpackedItem.GetDataWord()+scalerBase[itemChannel]);
            }
        }
        else continue;


    }
    if(outputEvent.SetADCMultiplicity() > 40){
        //Event is pulser event
        pulserNumber++;
    }
    outputEvent.SetTDCMultiplicity();
    outputEvent.SetPulserNumber(pulserNumber);
    outputEvent.SetEventNumber(unpackedItem.GetEventNumber());
    outputTree->Fill();
    dataWordsList.clear();
    return 0;
}

int AnalysisProcess::OpenOutputFile(std::string outputFile) {

    outF = TFile::Open(outputFile.c_str(), "RECREATE");
    if(!outF){
        std::cout << "Problem opening output file. Check input." << std::endl;
        return -1;
    }
    outputTree = new TTree("doubleAlpha","doubleAlpha");

    outputTree->Branch("double_alpha", &outputEvent);//, "eventNumber/l:adcChannels[128]/D:tdcChannels[128]/D:scalerChannels[16]/l:adcMultiplicity/I:tdcMultiplicity/I:pulserNumber/l");
    return 1;
}

int AnalysisProcess::DefineHistograms() {

    rawADCEnergyVsChannel = new TH2D("Raw_ADC_Energy_Vs_Channel","",NUMBER_ADC_CHANNELS,0,NUMBER_ADC_CHANNELS,4096,0,4096);
    calibratedADCEnergyVsChannel = new TH2D("Calibrated_ADC_Energy_Vs_Channel","",NUMBER_ADC_CHANNELS,0,NUMBER_ADC_CHANNELS,4096,0,4096);

    rawTDCVsChannel = new TH2D("Raw_TDC_Vs_Channel","",400,0,400,4096,0,4096);
    calibratedTDCVsChannel = new TH2D("Calibrated_TDC_Vs_Channel","",400,0,400,4096,0,4096);
    pulserVsChannel = new TH2D("Pulser_Vs_Channel","",NUMBER_ADC_CHANNELS,0,NUMBER_ADC_CHANNELS,4096,0,4096);

    return 1;
}

int AnalysisProcess::CloseAnalysisProcess() {


    std::cout << "Writing histograms" <<std::endl;
    rawADCEnergyVsChannel->Write();
    calibratedADCEnergyVsChannel->Write();
    rawTDCVsChannel->Write();
    calibratedTDCVsChannel->Write();
    pulserVsChannel->Write();
    outF->Write();

    outF->Close();

    return 0;
}
