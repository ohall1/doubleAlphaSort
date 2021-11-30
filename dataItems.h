//
// Created by oscar on 30/11/2021.
//

#include <cstdint>

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

};

class OutputEvent{
public:
    unsigned long eventNumber;
    double adcChannels[96];
    double tdcChannels[128];
    OutputEvent();
    int ClearEvent();
    int AddToEvent(bool adcEvent, int channel, double value);

};