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

#ifndef SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRACKINGERROR_TRACKINGERRORCALCULATOR_H_
#define SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRACKINGERROR_TRACKINGERRORCALCULATOR_H_

#include <omnetpp.h>
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Move.h"
#include "veins/masterthesis/WAVE/message/BasicSafetyMessage_m.h"

class TrackingErrorCalculator {

private:
    //these params taken from SAE J2945, values initialized with default constructor
    double vHVLocalPosEstIntMin;
    double vHVLocalPosEstIntMax;
    double vHVRemotePosEstIntMin;
    double vHVRemotePosEstIntMax;
    omnetpp::cSimpleModule * source;

public:
    TrackingErrorCalculator(omnetpp::cSimpleModule * source,
            double vHVLocalPosEstIntMin, double vHVLocalPosEstIntMax,
            double vHVRemotePosEstIntMin, double vHVRemotePosEstIntMax);
    virtual ~TrackingErrorCalculator();
    double getHvLocalPosEstIntMax() const;
    double getHvLocalPosEstIntMin() const;
    double getHvRemotePosEstIntMin() const;
    double getHvRemotePosEstIntMax() const;
    double calculateTrackingError(simtime_t timePositionChangedHVLocalEstimate,
            /*simtime_t timePositionChangedHVRemoteEstimate,*/ Coord curPosition,
            Move & move, BasicSafetyMessage * lastHVStatus,
            Move * lastMoveInHVStatus, double testCaseNumber);

};

#endif /* SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRACKINGERROR_TRACKINGERRORCALCULATOR_H_ */
