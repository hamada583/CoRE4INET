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

#include "MessageNumProfiler.h"

#include "Compat.h"

#include <ostream>

namespace CoRE4INET {

Define_Module(MessageNumProfiler);

MessageNumProfiler::MessageNumProfiler()
{
}

void MessageNumProfiler::initialize()
{
    scheduleAt(SimTime(par("interval").doubleValue()), new cMessage("profile"));
}

void MessageNumProfiler::handleMessage(cMessage *msg)
{
    cModule *sysmod = cSimulation::getActiveSimulation()->getSystemModule();

    if (par("print_cerr").boolValue())
    {
        std::cerr << "Now profiling modules with more than " << par("max_messages").longValue() << " messages" << endl;
        std::cerr << printMessages(sysmod, par("max_messages").longValue(), par("print_msgs").boolValue());
        std::cerr << "Profiling done!" << endl;
    }
    else
    {
        EV << "Now profiling modules with more than " << par("max_messages").longValue() << " messages" << endl;
        EV << printMessages(sysmod, par("max_messages").longValue(), par("print_msgs").boolValue());
        EV << "Profiling done!" << endl;
    }

    if (par("throw_error").boolValue() && overModules(sysmod, par("max_messages").longValue()))
    {
        delete msg;
        throw cRuntimeError("Profiler found %d modules with more than the configured maximum of %d messages",
                overModules(sysmod, par("max_messages").longValue()), par("max_messages").longValue());
    }

    if (par("max_live_messages").longValue() > 0 && msg->getLiveMessageCount() > par("max_live_messages").longValue())
    {
        if (par("throw_error").boolValue())
        {
            delete msg;
            throw cRuntimeError("Profiler found %d live messages, more than the configured maximum of %d messages",
                    msg->getLiveMessageCount(), par("max_live_messages").longValue());
        }
        else
        {
            //TODO
        }
    }
    /*if (par("max_scheduled_messages").longValue() > 0 &&  > par("max_scheduled_messages").longValue())
     {
     if (par("throw_error").boolValue())
     {
     delete msg;
     throw cRuntimeError("Profiler found %d live messages, more than the configured maximum of %d messages",
     msg->getLiveMessageCount(), par("max_live_messages").longValue());
     }
     else
     {
     //TODO
     }
     }*/

    scheduleAt(simTime() + SimTime(par("interval").doubleValue()), msg);
}

size_t MessageNumProfiler::sumRecursiveMessages(cModule *root, bool onlyChild)
{
    size_t sumMsg = 0;
    if (!onlyChild)
    {
        sumMsg += numMessages(root);
    }
    for (cModule::SubmoduleIterator i(root); !i.end(); i++)
    {
        cModule *submod = i();
        sumMsg += sumRecursiveMessages(submod, false);
    }
    return sumMsg;
}

size_t MessageNumProfiler::maxRecursiveMessages(cModule *root, bool onlyChild)
{
    size_t maxMsg = 0;
    if (!onlyChild)
    {
        maxMsg = numMessages(root);
    }
    for (cModule::SubmoduleIterator i(root); !i.end(); i++)
    {
        cModule *submod = i();
        size_t childMax = maxRecursiveMessages(submod, false);
        if (childMax > maxMsg)
        {
            maxMsg = childMax;
        }
    }
    return maxMsg;
}

size_t MessageNumProfiler::numMessages(cModule *module)
{
    size_t numMsg = 0;
    for (int i = 0; i < module->defaultListSize(); i++)
    {
        cOwnedObject *owned = module->defaultListGet(i);
        if (dynamic_cast<cMessage*>(owned))
        {
            numMsg++;
        }
    }
    return numMsg;
}

size_t MessageNumProfiler::overModules(cModule *root, size_t limit)
{
    size_t over_modules = 0;
    if (numMessages(root) > limit)
    {
        over_modules++;
    }
    for (cModule::SubmoduleIterator i(root); !i.end(); i++)
    {
        cModule *submod = i();
        over_modules += overModules(submod, limit);
    }
    return over_modules;
}

std::string MessageNumProfiler::printMessages(cModule *root, size_t limit, bool print_msgs, size_t level)
{
    std::ostringstream oss;
    if (maxRecursiveMessages(root, false) > limit)
    {
        oss << std::string(level * 4, ' ') << root->getFullName() << ": " << numMessages(root) << " ("
                << sumRecursiveMessages(root, true) << " in children)" << endl;
        if (print_msgs)
        {
            for (int i = 0; i < root->defaultListSize(); i++)
            {
                cOwnedObject *owned = root->defaultListGet(i);
                if (cMessage *msg = dynamic_cast<cMessage*>(owned))
                {
                    oss << "***" << msg->getClassName() << "(" << msg->getFullName() << ")"<< endl;
                }
            }
        }
        for (cModule::SubmoduleIterator i(root); !i.end(); i++)
        {
            cModule *submod = i();
            oss << printMessages(submod, print_msgs, level + 1);
        }
    }
    return oss.str();
}

} //namespace
