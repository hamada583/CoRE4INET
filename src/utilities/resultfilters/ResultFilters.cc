#include "ResultFilters.h"

#include <cmessage.h>

using namespace TTEthernetModel;

Register_ResultFilter("innerMessage", InnerMessageFilter);

void InnerMessageFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object)
{
    if (dynamic_cast<cMessage *>(object))
    {
        cPacket *innerPacket = (cPacket *)object;
        cPacket *tmpPacket;
        while((tmpPacket = innerPacket->decapsulate()) != NULL){
            innerPacket = tmpPacket;
        }
        fire(this, t, innerPacket);
        delete(innerPacket);
    }
}