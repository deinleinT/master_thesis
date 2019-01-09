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

#ifndef SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_MAXIMUMINTERTRANSMITTIME_MAXIMUMINTERTRANSMITTIMECALCULATOR_H_
#define SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_MAXIMUMINTERTRANSMITTIME_MAXIMUMINTERTRANSMITTIMECALCULATOR_H_

#include <omnetpp.h>

//see SAE J2945, Page 68/68, 6.3

class MaximumInterTransmitTimeCalculator {

private:
    omnetpp::cSimpleModule * source;
    double vMax_ITT; // maximum Threshold in msec
    double vDensityWeightFactor; // Ns(k) in formula
    double vDensityCoefficient; // B in formula
    double lastSmoothVehicleDensityInRange; // saves the value for Ns(k-1)

    double calculateSmoothVehicleDensityInRange(double vehicleDensityInRange, omnetpp::cOutVector & smoothVehicleDensVec);

public:
    double calculateMaximumInterTransmission(double vehicleDensityInRange, omnetpp::cOutVector & smoothVehicleDensVec, double testCaseNumber);
    MaximumInterTransmitTimeCalculator(omnetpp::cSimpleModule * source,
            double vMax_ITT, double vDensityWeightFactor,
            double vDensityCoefficient);
    virtual ~MaximumInterTransmitTimeCalculator();
};

#endif /* SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_MAXIMUMINTERTRANSMITTIME_MAXIMUMINTERTRANSMITTIMECALCULATOR_H_ */
