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

package core4inet.utilities.modules;

import core4inet.buffer.base.IBuffer;
import core4inet.incoming.base.IIncoming;

//
// Module for Buffer with Incoming Module for TT and RC traffic
//
// @see ~IIncoming, IBuffer
//
// @author Till Steinbach
//
module CheckedBuffer
{
    submodules:
        // Implementing the conformance checks of incoming traffic
        incoming: <default("Incoming")> like IIncoming {
            @display("p=34,30");
        }
        
        // Representing a buffer module
        buffer: <default("Buffer")> like IBuffer {
            @display("p=108,30");
        }
        
    connections:
        incoming.out --> buffer.in;
}
