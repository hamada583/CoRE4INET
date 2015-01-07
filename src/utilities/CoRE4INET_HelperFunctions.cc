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
#include "CoRE4INET_HelperFunctions.h"

//INET
#include "ModuleAccess.h"

namespace CoRE4INET {

std::string& replaceAll(std::string &string, const std::string &toFind, const std::string &replacement)
{
    size_t pos = string.find(toFind);
    while (pos != std::string::npos)
    {
        string.replace(pos, toFind.length(), replacement);
        pos = string.find(toFind);
    }
    return string;
}

void addPath(cPar &parameter, const std::string &pathToAdd)
{
    std::string path = parameter.stdstringValue();
    if (path.length() > 0)
        path.append(",");
    path.append(pathToAdd);
    parameter.setStringValue(path);
}

cGate* gateByFullPath(const std::string &path)
{
    std::size_t pos = path.rfind('.');
    if (pos != std::string::npos)
    {
        std::string modulePath = path.substr(0, pos);
        std::string gateName = path.substr(pos + 1);
        cModule* module = cSimulation::getActiveSimulation()->getModuleByPath(modulePath.c_str());
        if (module)
        {
            return module->gate(gateName.c_str());
        }
    }
    return nullptr;
}

cGate* gateByShortPath(const std::string &nameAndGate, cModule *from)
{
    std::size_t pos = nameAndGate.rfind('.');
    if (pos != std::string::npos)
    {
        std::string modulePath = nameAndGate.substr(0, pos);
        std::string gateName = nameAndGate.substr(pos + 1);
        cModule* module = findModuleWhereverInNode(modulePath.c_str(), from);
        if (module)
        {
            return module->gate(gateName.c_str());
        }
    }
    return nullptr;
}

uint64_t ticksToTransparentClock(uint64_t ticks, simtime_t tick)
{
    return secondsToTransparentClock(static_cast<double>(ticks) * tick);
}

uint64_t secondsToTransparentClock(simtime_t seconds)
{
    uint64_t div = 10;
    for (int i = 1; i < (SIMTIME_NS - seconds.getScaleExp()); ++i)
    {
        div *= 10;
    }
    return (static_cast<uint64_t>(seconds.raw()) * static_cast<uint64_t>(0x10000)) / div;
}

uint64_t transparentClockToTicks(uint64_t transparentClock, simtime_t tick)
{
    return transparentClock / secondsToTransparentClock(tick);
}

MACAddress generateAutoMulticastAddress()
{
    MACAddress multicastMAC = MACAddress::generateAutoAddress();
    multicastMAC.setAddressByte(1, (multicastMAC.getAddressByte(1) | 0x01));
    return multicastMAC;
}

#ifdef WITH_AS6802_COMMON
void setTransparentClock(PCFrame *pcf, double static_tx_delay, Timer* timer)
{
    uint64_t transparentClock = pcf->getTransparent_clock();

    //Add static delay for this port
    transparentClock += secondsToTransparentClock(static_tx_delay);

    //Add dynamic delay for the device
    cArray parlist = pcf->getParList();
    long start = -1;
    for (int i = 0; i < parlist.size(); i++)
    {
        cMsgPar *parameter = dynamic_cast<cMsgPar*>(parlist.get(i));
        if (parameter)
        {
            if (strncmp(parameter->getName(), "received_total", 15) == 0
                    || strncmp(parameter->getName(), "created_total", 15) == 0)
            {
                start = parameter->longValue();
            }
        }
    }
    if (start >= 0)
    {
        transparentClock += ticksToTransparentClock((timer->getTotalTicks() - static_cast<uint64_t>(start)),
                timer->getOscillator()->getPreciseTick());
    }

    //Set new transparent clock
    pcf->setTransparent_clock(transparentClock);
}
#endif

#ifdef WITH_AVB_COMMON

const_simtime_t second = 1;

unsigned long bandwidthFromSizeAndInterval(size_t framesize, size_t intervalFrames, simtime_t interval)
{
    return static_cast<unsigned long>(ceil(
            (second / interval) * static_cast<double>(intervalFrames)
                    * static_cast<double>(((framesize + PREAMBLE_BYTES + SFD_BYTES) * 8) + INTERFRAME_GAP_BITS)));
}

const simtime_t getIntervalForClass(SR_CLASS srClass)
{
    switch (srClass)
    {
        case SR_CLASS_A:
            return SR_CLASS_A_INTERVAL;
        case SR_CLASS_B:
            return SR_CLASS_B_INTERVAL;
    }
    return SR_CLASS_B_INTERVAL;
}
#endif

}
