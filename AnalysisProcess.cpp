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
    std::vector<double> temp;
    for(auto i = 0; i < 2; i++){
        blindRegionLow.push_back(temp);
        blindRegionHigh.push_back(temp);
    }
}

int AnalysisProcess::ReadParameters(std::string parameterFile) {
    std::string line;
    std::ifstream parameters(parameterFile.data());
    int channel;
    double value;
    double value_high;
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
            //std::cout << dummyVar << std::endl;
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
            else if(dummyVar == "blindRegion"){
                iss >> channel;
                iss >> value;
                iss >> value_high;
                
                std::cout << "Blinding region: Det " << channel << ", Lower bound " << value
                          << ", Upper bound " << value_high << std::endl;
                blindRegionLow.at(channel).push_back(value);
                blindRegionHigh.at(channel).push_back(value_high);
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
    int det = 2;
    isPulserEvent = false;
    bool isBlinded = false;
    //bool channelRead[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
    for(int i = 0; i < 32; i++){
        scalerWords[i] = 0;
    }
    for(dataWordsListIt = dataWordsList.begin(); dataWordsListIt != dataWordsList.end(); dataWordsListIt++){
        unpackedItem.UpdateItem(*dataWordsListIt, eventNumber);
        isBlinded = false;
        if(unpackedItem.GetGroup() < 20 && unpackedItem.GetGroup() > 0){
            //Is adc event
            itemChannel = unpackedItem.GetItem() + ((unpackedItem.GetGroup() - 1) * 32);
            itemValue = ((double)unpackedItem.GetDataWord()  - adcChannelOffset[itemChannel]) * adcChannelGains[itemChannel];
            if(itemChannel < 32){
                det = 0;
            }
            else if(itemChannel < 64)
            {
                det = 1;
            }
            else{
                det = 2;
            }
            if(blindingProcess == 0 && det == 1){
                isBlinded = true;
            }
            else if(blindingProcess == 1 && det == 0){
                isBlinded = true;
            }
            else if(blindingProcess == 2){
                if(det < 2){
                    for(int i = 0; i < blindRegionLow.at(det).size(); i++){
                        if(itemValue >= blindRegionLow.at(det).at(i) && itemValue <= blindRegionHigh.at(det).at(i)){
                            isBlinded = true;
                        }
                    }
                }
            }
            if(!isBlinded){
                outputEvent.AddToEvent(true, itemChannel, itemValue);
                rawADCEnergyVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
                calibratedADCEnergyVsChannel->Fill(itemChannel, itemValue);
            }
            if(isPulserEvent){
                pulserVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            }
        }
        else if(unpackedItem.GetGroup() < 30 && unpackedItem.GetGroup() > 19) {
            //Is TDC event
            itemChannel = unpackedItem.GetItem() + (unpackedItem.GetGroup() - 20) * 64;
            //Common stop mode means first 16 channels of V767 module are not used
            itemChannel = itemChannel - 16 *(1+((unpackedItem.GetGroup()-20)/2));
            if(itemChannel == -16){
                itemChannel = 127;
            }
            itemValue = (double)unpackedItem.GetDataWord();
            outputEvent.AddToEvent(false, itemChannel, itemValue);
            rawTDCVsChannel->Fill(itemChannel, (double)unpackedItem.GetDataWord());
            calibratedTDCVsChannel->Fill(itemChannel, itemValue);
        }
        else if(unpackedItem.GetGroup() == 30){
            //Is scaler event
            itemChannel = (unpackedItem.GetItem());
            //if (channelRead[itemChannel]) continue;
            //channelRead[itemChannel] = true;
            unsigned int value = unpackedItem.GetDataWord();
           // if(itemChannel == 2 || itemChannel == 3) std::cout << "Event: " <<  itemChannel << " " << value <<std::endl;

            if(itemChannel > 32){
                std::cout << itemChannel << " " << unpackedItem.GetItem() << std::endl;
            }
            if( itemChannel<32) {
                if (value < previousScaler[itemChannel]) {
                    //std::cout << itemChannel << " " << value << " " << previousScaler[itemChannel] << " " << scalerBase[itemChannel] << std::endl;
                    //scalerBase[itemChannel] += 65536;
                    //previousScaler[itemChannel] = 0;
                }
                //std::cout << itemChannel << " " << value << std::endl;
                //std::cout << "Before " << previousScaler[itemChannel] << " " << value << " " << itemChannel << std::endl;
                //previousScaler[itemChannel] = value;
                //std::cout << "After " << previousScaler[itemChannel] << " " << value << " " << itemChannel << std::endl;
                //outputEvent.AddScalerEvent(itemChannel, value+scalerBase[itemChannel]);
                scalerWords[itemChannel] = value;
                //if(itemChannel == 1 || itemChannel == 0) std::cout << "Array: " <<  itemChannel << " " << scalerWords[itemChannel] <<std::endl;
            }
        }
        else continue;


    }
    for(int i = 0; i < 32; i+=2){
        outputEvent.AddScalerEvent(i/2, (scalerWords[i+1] << 16 | scalerWords[i]));
        //std::cout << i << " " << i/2 << " " << (scalerWords[i+1] << 16 | scalerWords[i]) << " " <<  scalerWords[i+1] << " " << scalerWords[i] << std::endl;
        //if(i == 10){
          //  std::cout << (scalerWords[i+1] << 16 | scalerWords[i]) << std::endl;
       //}
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
    std::cout << "raw ADC" <<std::endl;
    calibratedADCEnergyVsChannel->Write();
    std::cout << "calibrated ADC" <<std::endl;
    rawTDCVsChannel->Write();
    std::cout << "raw TDC" <<std::endl;
    calibratedTDCVsChannel->Write();
    std::cout << "calibrated ADC" <<std::endl;
    pulserVsChannel->Write();
    std::cout << "pulser ADC" <<std::endl;
    outF->Write();
    std::cout << "File written" << std::endl;

    outF->Close();
    std::cout << "File closed" << std::endl;

    return 0;
}

int AnalysisProcess::DefineBlindProcess(int option){
    blindingProcess = option;
    return blindingProcess;
}
