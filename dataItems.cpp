//
// Created by oscar on 30/11/2021.
//
#include <utility>
#include <iostream>
#include "dataItems.h"
ClassImp(OutputEvent);

UnpackedItem::UnpackedItem() {
    item = 0;
    group = 0;
    dataWord = 0;
    eventNumber = 0;
}

int UnpackedItem::UpdateItem(std::pair<uint16_t, uint16_t> itemIn, unsigned long eventNumberIn) {
    group = itemIn.first & 0xff;
    item = (itemIn.first >> 8) & 0x3f;
    dataWord = itemIn.second;
    eventNumber = eventNumberIn;

    return 0;
}

int UnpackedItem::GetGroup() {
    return group;
}

int UnpackedItem::GetItem() {
    return item;
}

unsigned long UnpackedItem::GetEventNumber() {
    return eventNumber;
}

uint16_t UnpackedItem::GetDataWord() {
    return dataWord;
}

OutputEvent::OutputEvent() {
    eventNumber = 0;
    for(double & adcChannel : adcChannels){
        adcChannel = 0.0;
    }
    for(double & tdcChannel : tdcChannels){
        tdcChannel = 0.0;
    }
}

int OutputEvent::ClearEvent() {
    eventNumber = 0;
    for(double & adcChannel : adcChannels){
        adcChannel = 0.0;
    }
    for(double & tdcChannel : tdcChannels){
        tdcChannel = 0.0;
    }
    adcMultiplicity = 0;
    tdcMultiplicity = 0;
    pulserNumber = 0;
    return 0;
}

int OutputEvent::AddToEvent(bool adcEvent, int channel, double value) {
    if(adcEvent){
        if(channel < 128) {
            adcChannels[channel] = value;
            return 0;
        }
        else{
            std::cout << "ADC channel out of output range" << std::endl;
            return -1;
        }
    }
    else{
        if(channel < 128) {
            tdcChannels[channel] = value;
            return 0;
        }
        else{
            std::cout << "TDC channel out of output range" << std::endl;
            return -1;
        }
    }

    return 0;
}

int OutputEvent::SetEventNumber(unsigned long eventNumberIn) {
    eventNumber = eventNumberIn;
    return 0;
}

int OutputEvent::SetADCMultiplicity() {
    adcMultiplicity = 0;
    for(double adcChannel : adcChannels){
        if( adcChannel > 0){
            adcMultiplicity++;
        }
    }
    return adcMultiplicity;
}

int OutputEvent::SetTDCMultiplicity() {
    tdcMultiplicity = 0;
    for(double tdcChannel : tdcChannels){
        if( tdcChannel > 0){
            tdcMultiplicity++;
        }
    }
    return tdcMultiplicity;
}

int OutputEvent::SetPulserNumber(int pulserNumberIn) {
    pulserNumber = pulserNumberIn;
    return pulserNumber;
}

int OutputEvent::AddScalerEvent(int channel, unsigned long value) {
    scalerChannels[channel] = value;
    return 0;
}
