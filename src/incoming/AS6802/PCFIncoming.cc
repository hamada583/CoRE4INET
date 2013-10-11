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
#include "PCFIncoming.h"
#include "PCFrame_m.h"

namespace TTEthernetModel{

Define_Module(PCFIncoming);

PCFIncoming::PCFIncoming(){}

void PCFIncoming::initialize(){
    pcfType=(PCFType)par("pcfType").longValue();
}

void PCFIncoming::handleMessage(cMessage *msg){
    if(msg->arrivedOn("in")){
        PCFrame *frame = dynamic_cast<PCFrame *>(msg);

        if(frame->getType() != (uint8_t)pcfType){
            ev<<"FRAME DROPPED, wrong type:"<<(int)frame->getType()<<" should be "<<pcfType<<endl;
            delete frame;
        }else{
            sendDelayed(frame,SimTime(getParentModule()->par("hardware_delay").doubleValue()),"out");
        }
    }
}

void PCFIncoming::handleParameterChange(const char* parname){
    pcfType = (PCFType)par("pcfType").longValue();
}

}