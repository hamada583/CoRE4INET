#include <map>
#include <stdexcept>

#include "customWatch.h"
#include "HelperFunctions.h"

#include "SRPTable.h"

namespace CoRE4INET {

Define_Module(SRPTable);
Register_Class(SRPTable::TalkerEntry);
Register_Class(SRPTable::ListenerEntry);

std::ostream& operator<<(std::ostream& os, const SRPTable::TalkerEntry& entry)
{
    os << "{TalkerAddress=" << entry.address->str() << ", Module=" << entry.module->getFullName() << ", SRClass="
            << cEnum::get("CoRE4INET::SR_CLASS")->getStringFor(entry.srClass) << ", Bandwidth="
            << bandwidthFromSizeAndInterval(entry.framesize, entry.intervalFrames, getIntervalForClass(entry.srClass))
                    / (double) 1000000 << "Mbps, insertionTime=" << entry.insertionTime << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SRPTable::ListenerEntry& entry)
{
    os << "{InsertionTime=" << entry.insertionTime << "}";
    return os;
}

simsignal_t SRPTable::talkerRegisteredSignal = registerSignal("talkerRegistered");
simsignal_t SRPTable::talkerUpdatedSignal = registerSignal("talkerUpdated");
simsignal_t SRPTable::listenerRegisteredSignal = registerSignal("listenerRegistered");
simsignal_t SRPTable::listenerUpdatedSignal = registerSignal("listenerUpdated");
simsignal_t SRPTable::listenerUnregisteredSignal = registerSignal("listenerUnregistered");
simsignal_t SRPTable::listenerRegistrationTimeoutSignal = registerSignal("listenerRegistrationTimeout");

SRPTable::SRPTable()
{
    nextAging = 0;
}

void SRPTable::initialize()
{
    WATCH(nextAging);
    WATCH_MAPMAP(talkerTables);
    WATCH_LISTMAPMAP(listenerTables);
    updateDisplayString();
}

void SRPTable::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

std::list<cModule*> SRPTable::getListenersForStreamId(uint64_t streamId, unsigned int vid)
{
    Enter_Method
    ("SRPTable::getModulesForStreamId()");

    removeAgedEntriesIfNeeded();

    std::list<cModule*> modules;

    ListenerTable listenerTable = listenerTables[vid];

    ListenerTable::iterator iter = listenerTable.find(streamId);

    if (iter != listenerTable.end())
    {
        for (ListenerList::iterator entry = (*iter).second.begin(); entry != (*iter).second.end(); entry++)
        {
            modules.push_back((*entry).first);
        }
    }
    return modules;
}

cModule* SRPTable::getTalkerForStreamId(uint64_t streamId, unsigned int vid)
{
    removeAgedEntriesIfNeeded();

    TalkerTable talkerTable = talkerTables[vid];
    TalkerTable::iterator entry = talkerTable.find(streamId);
    if (entry != talkerTable.end())
    {
        return (*entry).second->module;
    }
    return NULL;
}

unsigned long SRPTable::getBandwidthForStream(uint64_t streamId, unsigned int vid)
{
    removeAgedEntriesIfNeeded();

    //get Talkers for this VLAN
    TalkerTable ttable = talkerTables[vid];
    TalkerEntry *tentry = ttable[streamId];
    return bandwidthFromSizeAndInterval(tentry->framesize, tentry->intervalFrames, getIntervalForClass(tentry->srClass));
}

unsigned long SRPTable::getBandwidthForModule(cModule *module)
{
    removeAgedEntriesIfNeeded();

    unsigned long bandwidth = 0;

    for (std::map<unsigned int, ListenerTable>::iterator i = listenerTables.begin(); i != listenerTables.end(); i++)
    {
        ListenerTable table = i->second;
        for (ListenerTable::iterator j = table.begin(); j != table.end(); j++)
        {
            ListenerList llist = (*j).second;
            for (ListenerList::iterator k = llist.begin(); k != llist.end(); k++)
            {
                if ((*k).first == module)
                {
                    //get Talkers for this VLAN
                    TalkerTable ttable = talkerTables[(*i).first];
                    TalkerEntry *tentry = ttable[(*j).first];
                    bandwidth += bandwidthFromSizeAndInterval(tentry->framesize, tentry->intervalFrames,
                            getIntervalForClass(tentry->srClass));
                }
            }
        }
    }

    return bandwidth;
}

bool SRPTable::updateTalkerWithStreamId(uint64_t streamId, cModule *module, MACAddress *address, SR_CLASS srClass,
        unsigned int framesize, unsigned int intervalFrames, unsigned int vid)
{
    Enter_Method
    ("SRPTable::updateTalkerWithStreamId()");

    bool updated = true;
    TalkerTable &talkerTable = talkerTables[vid];

    if (talkerTable.find(streamId) == talkerTable.end())
    {
        if (framesize == 0)
        {
            throw std::invalid_argument("cannot register talker with zero framesize");
        }
        if (intervalFrames == 0)
        {
            throw std::invalid_argument("cannot register talker with zero frameInterval");
        }
        if (address == NULL)
        {
            throw std::invalid_argument("cannot register talker without address");
        }
        if (module == NULL)
        {
            throw std::invalid_argument("cannot register talker without module");
        }
        updated = false;
        talkerTable[streamId] = new SRPTable::TalkerEntry();
    }
    else
    {
        if (talkerTable[streamId]->module != module)
        {
            throw std::invalid_argument("trying to update talker from wrong module");
        }
    }
    talkerTable[streamId]->streamId = streamId;
    talkerTable[streamId]->module = module;
    talkerTable[streamId]->srClass = srClass;
    if (framesize > 0)
    {
        talkerTable[streamId]->framesize = framesize;
    }
    if (intervalFrames > 0)
    {
        talkerTable[streamId]->intervalFrames = intervalFrames;
    }
    if (address != NULL)
    {
        talkerTable[streamId]->address = address;
    }
    talkerTable[streamId]->insertionTime = simTime();
    if (updated)
    {
        emit(talkerUpdatedSignal, talkerTable[streamId]);
    }
    else
    {
        emit(talkerRegisteredSignal, talkerTable[streamId]);
    }
    updateDisplayString();
    return updated;
}

bool SRPTable::removeTalkerWithStreamId(uint64_t streamId, cModule *module, MACAddress *address, unsigned int vid)
{

    TalkerTable &talkerTable = talkerTables[vid];

    TalkerTable::iterator talker = talkerTable.find(streamId);

    if (talker != talkerTable.end())
    {
        if (talkerTable[streamId]->module != module)
        {
            throw std::invalid_argument("trying to unregister talker from wrong module");
        }
        talkerTable.erase(talker);
        updateDisplayString();
        return true;
    }
    return false;
}

bool SRPTable::updateListenerWithStreamId(uint64_t streamId, cModule *module, unsigned int vid)
{
    Enter_Method
    ("SRPTable::updateListenerWithStreamId()");

    TalkerTable ttable = talkerTables[vid];
    if (ttable.find(streamId) == ttable.end())
    {
        std::ostringstream oss;
        oss << "no talker for stream with id " << streamId << " in vlan " << vid;
        throw std::invalid_argument(oss.str());
    }

    bool updated = true;

    ListenerTable &listenerTable = listenerTables[vid];
    ListenerList &llist = listenerTable[streamId];
    ListenerList::iterator listener = llist.find(module);
    if (listener == llist.end())
    {
        if (module == NULL)
        {
            throw std::invalid_argument("cannot register listener without module");
        }
        updated = false;
        llist[module] = new SRPTable::ListenerEntry();
    }
    llist[module]->streamId = streamId;
    llist[module]->module = module;
    llist[module]->insertionTime = simTime();

    if (updated)
    {
        emit(listenerUpdatedSignal, llist[module]);
    }
    else
    {
        emit(listenerRegisteredSignal, llist[module]);
    }
    updateDisplayString();
    if (nextAging == 0)
    {
        nextAging = simTime() + par("agingTime").doubleValue();
    }
    return updated;
}

bool SRPTable::removeListenerWithStreamId(uint64_t streamId, cModule *module, unsigned int vid)
{
    Enter_Method
    ("SRPTable::removeListenerWithStreamId()");
    ListenerTable &listenerTable = listenerTables[vid];

    ListenerTable::iterator listeners = listenerTable.find(streamId);

    if (listeners != listenerTable.end())
    {
        ListenerList::iterator listener = (*listeners).second.find(module);
        if (listener != (*listeners).second.end())
        {
            ListenerEntry *lentry = (*listener).second;
            emit(listenerUnregisteredSignal, lentry);
            (*listeners).second.erase(listener);
            updateDisplayString();
            return true;
        }
    }
    return false;
}

void SRPTable::printState()
{
    removeAgedEntriesIfNeeded();

    EV << "Talker Table" << endl;
    EV << "VLAN ID    StreamID    Port    Address    SRClass    Bandwidth(Mbps)    Inserted" << endl;
    for (std::map<unsigned int, TalkerTable>::iterator i = talkerTables.begin(); i != talkerTables.end(); i++)
    {
        TalkerTable table = i->second;
        for (TalkerTable::iterator j = table.begin(); j != table.end(); j++)
        {
            EV << (*i).first << "   " << (*j).first << "   " << (*j).second->module->getName() << "   "
                    << (*j).second->address->str() << "   "
                    << cEnum::get("CoRE4INET::SR_CLASS")->getStringFor((*j).second->srClass) << "    "
                    << bandwidthFromSizeAndInterval((*j).second->framesize, (*j).second->intervalFrames,
                            getIntervalForClass((*j).second->srClass)) / (double) 1000000 << "   "
                    << (*j).second->insertionTime << endl;
        }
    }

    EV << "Listener Table" << endl;
    EV << "VLAN ID    StreamID    Port    Inserted" << endl;
    for (std::map<unsigned int, ListenerTable>::iterator i = listenerTables.begin(); i != listenerTables.end(); i++)
    {
        ListenerTable table = i->second;
        for (ListenerTable::iterator j = table.begin(); j != table.end(); j++)
        {
            ListenerList llist = (*j).second;
            for (ListenerList::iterator k = llist.begin(); k != llist.end(); k++)
            {
                EV << (*i).first << "   " << (*j).first << "   " << (*k).first->getName() << "   "
                        << (*k).second->insertionTime << endl;
            }
        }
    }

}

void SRPTable::clear()
{
    for (std::map<unsigned int, TalkerTable>::iterator iter = talkerTables.begin(); iter != talkerTables.end(); iter++)
        (*iter).second.clear();
}

unsigned int SRPTable::getNumTalkerEntries()
{
    removeAgedEntriesIfNeeded();

    unsigned int entries = 0;
    for (std::map<unsigned int, TalkerTable>::iterator i = talkerTables.begin(); i != talkerTables.end(); ++i)
    {
        entries += (*i).second.size();
    }
    return entries;
}
unsigned int SRPTable::getNumListenerEntries()
{
    removeAgedEntriesIfNeeded();

    unsigned int entries = 0;
    for (std::map<unsigned int, ListenerTable>::iterator i = listenerTables.begin(); i != listenerTables.end(); ++i)
    {
        for (ListenerTable::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
        {
            entries += (*j).second.size();
        }
    }
    return entries;
}

void SRPTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d talkers\n%d listeners", getNumTalkerEntries(), getNumListenerEntries());
    getDisplayString().setTagArg("t", 0, buf);
}

void SRPTable::removeAgedEntries()
{
    simtime_t agingTime = par("agingTime").doubleValue();
    if(agingTime == 0){
        return;
    }
    simtime_t now = simTime();
    nextAging = 0;
    for (std::map<unsigned int, ListenerTable>::iterator listenerTable = listenerTables.begin();
            listenerTable != listenerTables.end(); ++listenerTable)
    {
        for (ListenerTable::iterator listenerList = (*listenerTable).second.begin();
                listenerList != (*listenerTable).second.end(); ++listenerList)
        {
            for (ListenerList::iterator listenerEntry = (*listenerList).second.begin();
                    listenerEntry != (*listenerList).second.end();)
            {
                simtime_t entryAging = ((*listenerEntry).second->insertionTime + agingTime);
                if (now >= entryAging)
                {
                    ListenerEntry *lentry = (*listenerEntry).second;
                    (*listenerList).second.erase(listenerEntry++);
                    emit(listenerRegistrationTimeoutSignal, lentry);
                    delete lentry;
                    updateDisplayString();
                }
                else
                {
                    ++listenerEntry;
                    if (nextAging > entryAging)
                    {
                        nextAging = entryAging;
                    }
                }
            }
        }
    }
}

void SRPTable::removeAgedEntriesIfNeeded()
{
    if (nextAging != 0 && simTime() >= nextAging)
    {
        removeAgedEntries();
    }
}

SRPTable::~SRPTable()
{
}

}
