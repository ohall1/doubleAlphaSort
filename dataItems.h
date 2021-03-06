//
// Created by oscar on 30/11/2021.
//

#include <cstdint>
#include "TObject.h"
#include "TCollection.h"

#ifndef DOUBLEALPHA_DATAITEMS_H
#define DOUBLEALPHA_DATAITEMS_H

#endif //DOUBLEALPHA_DATAITEMS_H
class UnpackedItem{
private:
    int group;
    int item;
    unsigned long eventNumber;
    uint16_t dataWord;

public:
    UnpackedItem();
    ~UnpackedItem(){};
    int UpdateItem(std::pair< uint16_t, uint16_t> itemIn, unsigned long eventNumberIn);
    int GetGroup();
    int GetItem();
    unsigned long GetEventNumber();
    uint16_t GetDataWord();

};

class OutputEvent: public TObject{
public:
    unsigned long eventNumber;
    double adcChannels[128];
    double tdcChannels[128];
    unsigned long scalerChannels[16];
    int adcMultiplicity;
    int tdcMultiplicity;
    unsigned long pulserNumber;
    OutputEvent();
    int ClearEvent();
    int AddToEvent(bool adcEvent, int channel, double value);
    int SetEventNumber(unsigned long eventNumberIn);
    int SetADCMultiplicity();
    int SetTDCMultiplicity();
    int SetPulserNumber(int pulserNumberIn);
    int AddScalerEvent(int channel, unsigned long value);

ClassDef(OutputEvent,1);  //Simple event class

};