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

#include "MaximumInterTransmitTimeCalculator.h"

/*MaximumInterTransmitTimeCalculator::MaximumInterTransmitTimeCalculator(
 omnetpp::cSimpleModule * source) :
 vMax_ITT(600.0), vDensityWeightFactor(0.05), vDensityCoefficient(25.0) {
 this->source = source;
 }*/

MaximumInterTransmitTimeCalculator::MaximumInterTransmitTimeCalculator(
        omnetpp::cSimpleModule * source, double vMax_ITT,
        double vDensityWeightFactor, double vDensityCoefficient) {
    this->source = source;
    this->vMax_ITT = vMax_ITT;
    this->vDensityWeightFactor = vDensityWeightFactor;
    this->vDensityCoefficient = vDensityCoefficient;
    this->lastSmoothVehicleDensityInRange = 0.0;
}

MaximumInterTransmitTimeCalculator::~MaximumInterTransmitTimeCalculator() {
}

double MaximumInterTransmitTimeCalculator::calculateSmoothVehicleDensityInRange(
        double vehicleDensityInRange,
        omnetpp::cOutVector & smoothVehicleDensVec) {

//formula taken from SAE J2945, Page 68

    double smoothVehicleDensityInRange = 0.0;

    smoothVehicleDensityInRange = vDensityWeightFactor * vehicleDensityInRange
            + (1 - vDensityWeightFactor) * lastSmoothVehicleDensityInRange;

    lastSmoothVehicleDensityInRange = smoothVehicleDensityInRange;
    smoothVehicleDensVec.record(smoothVehicleDensityInRange);

    return smoothVehicleDensityInRange;
}

//see SAE J2945, Page 68/69
// return Value is divided through 1000 to get the correct double value in ms for handling the timer
double MaximumInterTransmitTimeCalculator::calculateMaximumInterTransmission(
        double vehicleDensityInRange,
        omnetpp::cOutVector & smoothVehicleDensVec, double testCaseNumber) {

    ASSERT(vehicleDensityInRange >= 0.0);

    //test smoothVehicleDensityInRange
    if (testCaseNumber == 6.1) {
        vehicleDensityInRange = 0;
    }
    //

    double smoothVehicleDensityInRange = calculateSmoothVehicleDensityInRange(
            vehicleDensityInRange, smoothVehicleDensVec);

    //assert smoothVehicleDensityInRange
    if (testCaseNumber == 6.1) {
        ASSERT(smoothVehicleDensityInRange == 0);
    }
    //

    double max_ITT = 0.0;

    //test Max_ITT
    if (testCaseNumber == 6.2) {
        smoothVehicleDensityInRange = 25;
    } else if (testCaseNumber == 6.3) {
        smoothVehicleDensityInRange = 26;
    } else if (testCaseNumber == 6.4) {
        smoothVehicleDensityInRange = 149;
    } else if (testCaseNumber == 6.5) {
        smoothVehicleDensityInRange = 150;
    }
    //

    if (smoothVehicleDensityInRange <= vDensityCoefficient) {
        max_ITT = 100;
        return max_ITT / 1000;
    }

    if (vDensityCoefficient < smoothVehicleDensityInRange
            && smoothVehicleDensityInRange
                    < ((vMax_ITT / 100) * vDensityCoefficient)) {
        max_ITT = 100 * (smoothVehicleDensityInRange / vDensityCoefficient);
        return max_ITT / 1000;
    }

    if (((vMax_ITT / 100) * vDensityCoefficient)
            <= smoothVehicleDensityInRange) {
        return vMax_ITT / 1000;
    }

    return max_ITT;
}
