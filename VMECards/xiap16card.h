/*
 * xiap16card.h
 *
 *  Created on: Apr 5, 2013
 *      Author: daq
 */

#ifndef XIAP16CARD_H_
#define XIAP16CARD_H_
#include "TObject.h"
#include "xiap16.h"

class xiap16card : public TObject
{
public:
	xiap16card();
	virtual ~xiap16card();
    int AllocateBuffers(unsigned int buffersize = 0x64000);
    size_t WriteSpillToFile(FILE* fileraw);
    size_t ReadSpillFromFile(FILE* fileraw);
    static size_t readFileWaiting(void * ptr, size_t size, size_t count, FILE * stream);
    enum { maxdatabuffers = 2 };
private:
    unsigned int databuffersize[XIAP16_CHANNELS_PER_CARD];

    // Trigger stats read from card
    unsigned int triggerstatrun[XIAP16_RUNSTAT_SIZE];
    unsigned int databufferread[maxdatabuffers];
    //ROOT should not save the following raw data buffer
    unsigned int* databuffer[maxdatabuffers]; //!

    ClassDef(xiap16card,1)
};

#endif /* XIAP16CARD_H_ */
