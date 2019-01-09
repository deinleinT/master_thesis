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

#include "TrackingErrorCalculator.h"

TrackingErrorCalculator::TrackingErrorCalculator(
        omnetpp::cSimpleModule * source, double vHVLocalPosEstIntMin,
        double vHVLocalPosEstIntMax, double vHVRemotePosEstIntMin,
        double vHVRemotePosEstIntMax) {
    this->source = source;
    this->vHVLocalPosEstIntMin = vHVLocalPosEstIntMin;
    this->vHVLocalPosEstIntMax = vHVLocalPosEstIntMax;
    this->vHVRemotePosEstIntMin = vHVRemotePosEstIntMin;
    this->vHVRemotePosEstIntMax = vHVRemotePosEstIntMax;
}

TrackingErrorCalculator::~TrackingErrorCalculator() {
}

double TrackingErrorCalculator::getHvLocalPosEstIntMax() const {
    return vHVLocalPosEstIntMax;
}

double TrackingErrorCalculator::getHvLocalPosEstIntMin() const {
    return vHVLocalPosEstIntMin;
}

double TrackingErrorCalculator::getHvRemotePosEstIntMax() const {
    return vHVRemotePosEstIntMax;
}

double TrackingErrorCalculator::getHvRemotePosEstIntMin() const {
    return vHVRemotePosEstIntMin;
}

double TrackingErrorCalculator::calculateTrackingError(
        simtime_t timePositionChangedHVLocalEstimate, Coord curPosition,
        Move & move, BasicSafetyMessage * lastHVStatus,
        Move * lastMoveInHVStatus, double testCaseNumber) {

    Coord currentEstimatedPositionHVLocalEstimate;
    Coord currentEstimatedPositionHVRemoteEstimate;
    double trackingError = 0.0;
    simtime_t deltaTimeHVLocalEstimate = simTime()
            - timePositionChangedHVLocalEstimate;
    simtime_t deltaTimeHVRemoteEstimate = simTime()
            - lastHVStatus->getTimeOfPositionUpdate();


//HVLocalEstimate
    if (deltaTimeHVLocalEstimate > this->getHvLocalPosEstIntMax()) {
        trackingError = 0.0;
        // no other steps --> RETURN
        return trackingError;
    }

    //HVRemoteEstimate
    //latest status is the same as above
    if (deltaTimeHVRemoteEstimate > this->getHvRemotePosEstIntMax()) {
        trackingError = 0.0;
        //no other steps --> RETURN
        return trackingError;
    }

    //HVLocalEstimate -->  Appendix A.3
    if (deltaTimeHVLocalEstimate < this->getHvLocalPosEstIntMin()) {
        //curPosition is ok, do nothing
        currentEstimatedPositionHVLocalEstimate = curPosition;
    } else if (deltaTimeHVLocalEstimate >= this->getHvLocalPosEstIntMin()
            && deltaTimeHVLocalEstimate <= this->getHvLocalPosEstIntMax()) {
        //position extrapolation!!!
        // not using the specified algorithm in SAE J2945, calculating the estimated current position as follows
        currentEstimatedPositionHVLocalEstimate = move.getPositionAt(
                timePositionChangedHVLocalEstimate + deltaTimeHVLocalEstimate);
    }

    //HVRemoteEstimate --> Appendix A.3
    if (deltaTimeHVRemoteEstimate < this->getHvRemotePosEstIntMin()) {
        //getSenderPos is curPosition of last sent beacon
        currentEstimatedPositionHVRemoteEstimate = lastHVStatus->getSenderPos();
    } else if (deltaTimeHVRemoteEstimate >= this->getHvRemotePosEstIntMin()
            && deltaTimeHVRemoteEstimate <= this->getHvRemotePosEstIntMax()) {
        currentEstimatedPositionHVRemoteEstimate =
                lastMoveInHVStatus->getPositionAt(
                        lastHVStatus->getTimeOfPositionUpdate() + deltaTimeHVRemoteEstimate);
    }

    trackingError = currentEstimatedPositionHVLocalEstimate.distance(
            currentEstimatedPositionHVRemoteEstimate);

    return trackingError;
}
