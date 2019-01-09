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

#ifndef SRC_VEINS_MASTERTHESIS_UTIL_BUSYTIME_H_
#define SRC_VEINS_MASTERTHESIS_UTIL_BUSYTIME_H_

#include <omnetpp.h>

struct BusyTime {

    omnetpp::simtime_t startBusyTime;
    omnetpp::simtime_t endBusytime;
    omnetpp::simtime_t busyTime;

};

#endif /* SRC_VEINS_MASTERTHESIS_UTIL_BUSYTIME_H_ */
