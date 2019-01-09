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

#include "TransmissionProbabilityCalculator.h"

TransmissionProbabilityCalculator::TransmissionProbabilityCalculator(
        omnetpp::cSimpleModule * source, double vTrackingErrMin,
        double vTrackingErrMax, double vErrSensitivity) {
    this->source = source;
    this->vTrackingErrMin = vTrackingErrMin;
    this->vTrackingErrMax = vTrackingErrMax;
    this->vErrSensitivity = vErrSensitivity;
}

TransmissionProbabilityCalculator::~TransmissionProbabilityCalculator() {
}

//see SAE J2945, 6.3.8.3, Page 68
double TransmissionProbabilityCalculator::calculateTransmissionProbability(
        double trackingError) {

    ASSERT(trackingError >= 0.0);
    double transmissionProbability = 0.0;

    if ((vTrackingErrMin <= trackingError)
            && (trackingError < vTrackingErrMax)) {

        // p(k) = 1 - exp(-alpha x |e(k) - T|²)
        transmissionProbability = 1
                - exp(
                        -vErrSensitivity
                                * pow(fabs(trackingError - vTrackingErrMin),
                                        2));

    } else if (trackingError >= vTrackingErrMax) {

        transmissionProbability = 1.0;

    } else {

        transmissionProbability = 0.0;

    }

    ASSERT(transmissionProbability >= 0.0);

    return transmissionProbability;
}
