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

#ifndef __CORE4INET_SRPETHERLLC_H
#define __CORE4INET_SRPETHERLLC_H

#include "EtherLLC.h"

#include "SRPFrame_m.h"
#include "EtherFrame_m.h"

namespace CoRE4INET {

class SRPEtherLLC : public EtherLLC
{
  protected:
    virtual void handleMessage(cMessage *msg);

    void dispatchSRP(SRPFrame * srp);
    void deliverSRP(EtherFrame * frame);

};

}
#endif