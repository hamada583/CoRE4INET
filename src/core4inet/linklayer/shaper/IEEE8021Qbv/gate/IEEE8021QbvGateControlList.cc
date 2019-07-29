//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "IEEE8021QbvGateControlList.h"

//std
#include <limits>
//CoRE4INET
#include "core4inet/utilities/ConfigFunctions.h"
#include "core4inet/linklayer/shaper/IEEE8021Qbv/gate/IEEE8021QbvGate.h"
//Auto-generated Messages
#include "core4inet/scheduler/SchedulerMessageEvents_m.h"

namespace CoRE4INET {

Define_Module(IEEE8021QbvGateControlList);

void IEEE8021QbvGateControlList::initialize(int stage)
{
    if (stage == 1)
    {
        Scheduled::initialize();
        this->handleParameterChange(nullptr);
        if (this->gateControlList.size() > 0)
        {
            this->scheduleCurrentGateControlElementTime();
        }
    }
}

int IEEE8021QbvGateControlList::numInitStages() const
{
    return 2;
}

void IEEE8021QbvGateControlList::handleParameterChange(const char* parname)
{
    Scheduled::handleParameterChange(parname);
    if (!parname || !strcmp(parname, "numGates"))
    {
        this->numGates = parameterULongCheckRange(par("numGates"), 1, std::numeric_limits<unsigned int>::max());
    }
    if (!parname || !strcmp(parname, "controlList"))
    {
        gateControlList.clear();
        vector<string> controlRows = cStringTokenizer(par("controlList"), ";").asVector();
        for (vector<string>::const_iterator controlRow = controlRows.begin(); controlRow != controlRows.end(); ++controlRow)
        {
            vector<string> controlRowTupel = cStringTokenizer((*controlRow).c_str(), ":").asVector();
            vector<string> controlRowGates = cStringTokenizer(controlRowTupel[0].c_str(), ",").asVector();
            if (controlRowGates.size() != this->numGates)
            {
                throw cRuntimeError("controlList row size is not %d", this->numGates);
            }
            for (size_t i=0; i<controlRowGates.size(); i++)
            {
                if (strcmp(controlRowGates[i].c_str(), "o") && strcmp(controlRowGates[i].c_str(), "C"))
                {
                    throw cRuntimeError("controlList contains unexpected character \'%s\'. Allowed are \'o' (OPEN) or \'C\' (CLOSED).", controlRowGates[i].c_str());
                }
            }
            double controlRowTime = stod(controlRowTupel[1]);
            gateControlList.push_back(make_pair(controlRowGates, controlRowTime));
        }
        this->gateControlElement = this->gateControlList.begin();
    }
}

void IEEE8021QbvGateControlList::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("schedulerIn") && msg->getKind() == ACTION_TIME_EVENT)
    {
        this->propagteGateControlElement((*(this->gateControlElement)).first);
        if (this->gateControlList.size() > 1)
        {
            this->switchToNextGateControlElement();
            this->scheduleCurrentGateControlElementTime();
        }
    }
    delete msg;
}

void IEEE8021QbvGateControlList::scheduleCurrentGateControlElementTime()
{

    SchedulerActionTimeEvent* actionTimeEvent = new SchedulerActionTimeEvent("Gate Control List Scheduler Event", ACTION_TIME_EVENT);
    uint32_t actionTime = ceil((*(this->gateControlElement)).second / getOscillator()->getPreciseTick());
    if (actionTime >= getPeriod()->getCycleTicks())
    {
        throw cRuntimeError("The send window (%d ticks) starts outside of the period (%d ticks)",
                actionTime, getPeriod()->getCycleTicks());
    }
    actionTimeEvent->setAction_time(actionTime);
    actionTimeEvent->setDestinationGate(this->gate("schedulerIn"));
    getPeriod()->registerEvent(actionTimeEvent);
}

void IEEE8021QbvGateControlList::propagteGateControlElement(vector<string> gateStates)
{
    for (long i=static_cast<long>(numGates)-1; i>=0; i--)
    {
        IEEE8021QbvGate* tg = dynamic_cast<IEEE8021QbvGate*>(this->getParentModule()->getSubmodule("transmissionGate", i));
        if ( !strcmp(gateStates[i].c_str(), "o"))
        {
            tg->open();
        }
        if ( !strcmp(gateStates[i].c_str(), "C"))
        {
            tg->close();
        }
    }
}

void IEEE8021QbvGateControlList::switchToNextGateControlElement()
{
    ++(this->gateControlElement);
    if (this->gateControlElement == this->gateControlList.end())
    {
        this->gateControlElement = this->gateControlList.begin();
    }
}


} //namespace
