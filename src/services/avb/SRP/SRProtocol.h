//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CORE4INET_SRPROTOCOL_H_
#define __CORE4INET_SRPROTOCOL_H_

#include "clistener.h"
#include "csimplemodule.h"

#include "SRPTable.h"

namespace CoRE4INET {

class SRProtocol : public cSimpleModule, public cListener
{
    protected:
        int localSAP;
        int remoteSAP;
        SRPTable *srpTable;

        virtual void initialize();


        virtual void handleMessage(cMessage *msg);

        virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
};

} /* namespace CoRE4INET */
#endif /* SRPTRAFFICHANDLE_H_ */
