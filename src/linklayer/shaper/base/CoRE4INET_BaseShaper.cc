#include "CoRE4INET_BaseShaper.h"

//Std
#include <algorithm>

using namespace CoRE4INET;

void BaseShaper::initialize(int stage)
{
    if (stage == 0)
    {
        if (getParentModule() && getParentModule()->getSubmodule("mac")
                && getParentModule()->getSubmodule("mac")->gate("phys$o"))
        {
            cGate *physOutGate = getParentModule()->getSubmodule("mac")->gate("phys$o");
            outChannel = physOutGate->findTransmissionChannel();
        }
        else{
            throw cRuntimeError("A shaper can only be used within a port module with an attached EtherMac");
        }
        WATCH(framesRequested);
    }
}

int BaseShaper::numInitStages() const
{
    return 1;
}

void BaseShaper::addListener(inet::IPassiveQueueListener *listener)
{
    std::list<inet::IPassiveQueueListener*>::const_iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}
void BaseShaper::removeListener(inet::IPassiveQueueListener *listener)
{
    std::list<inet::IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void BaseShaper::notifyListeners()
{
    for (std::list<inet::IPassiveQueueListener*>::const_iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->packetEnqueued(this);
}

void BaseShaper::handleMessage(cMessage *msg)
{
    EV_WARN << "Shaper has no method to handle frametype. Dropping frame" << endl;
    delete msg;
}

void BaseShaper::enqueueMessage(cMessage *msg)
{
    EV_WARN << "Shaper has no method to enqueue frametype. Dropping frame" << endl;
    delete msg;
}
