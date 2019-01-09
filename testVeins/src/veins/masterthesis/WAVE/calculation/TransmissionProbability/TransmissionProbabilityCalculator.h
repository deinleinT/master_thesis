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

#ifndef SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRANSMISSIONPROBABILITY_TRANSMISSIONPROBABILITYCALCULATOR_H_
#define SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRANSMISSIONPROBABILITY_TRANSMISSIONPROBABILITYCALCULATOR_H_

#include <omnetpp.h>
#include <math.h>

// taken from SAE J2945, page 68

class TransmissionProbabilityCalculator {

private:
    omnetpp::cSimpleModule * source;
    double vTrackingErrMin; // the minimum communications-induced tracking error threshold (T in formula)
    double vTrackingErrMax; // the communications-induced tracking error saturation value vTrackingErrMax (S in formula)
    double vErrSensitivity; // is the error sensitivity (alpha in formula)

public:
    TransmissionProbabilityCalculator(omnetpp::cSimpleModule * source,
            double vTrackingErrMin, double vTrackingErrMax,
            double vErrSensitivity);
    virtual ~TransmissionProbabilityCalculator();
    double calculateTransmissionProbability(double trackingError);
};

#endif /* SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_TRANSMISSIONPROBABILITY_TRANSMISSIONPROBABILITYCALCULATOR_H_ */
