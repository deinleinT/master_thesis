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

#include "ChannelBusyListenerWave.h"

ChannelBusyListenerWave::ChannelBusyListenerWave(
        omnetpp::cSimpleModule * source) :
        isIdle(false), isBusy(false), lastCBP(0.0) {
    this->source = source;
    this->lastBusyTime = 0;
    this->lastIdleTime = 0;
    firstCall = 0;
    this->totalBusyTime = 0;

}

ChannelBusyListenerWave::~ChannelBusyListenerWave() {

    busyTimeVector.clear();
}

void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, bool b, cObject *details) {

    if (b) {
        // fill the vectors

        if (isBusy) {
            //do nothing
        } else {

            lastBusyTime = simTime();

        }

        isBusy = true;
        isIdle = false;

    } else {

        //fill the vectors
        if (isIdle) {
            //do nothing
        } else {

            if (lastBusyTime > 0) {

                BusyTime temp;
                temp.startBusyTime = lastBusyTime;
                temp.endBusytime = simTime();
                temp.busyTime = simTime() - lastBusyTime;
                busyTimeVector.push_back(temp);

                this->totalBusyTime += simTime() - lastBusyTime;
            }
        }

        isBusy = false;
        isIdle = true;

    }

}

// see SAE J2945, 6.3
double ChannelBusyListenerWave::calculateRawChannelBusyPercentage(
        simtime_t period, cOutVector & busyTimeVec, cOutVector & standardCBCVec,
        simtime_t & totalBusyTime, double testCaseNumber,
        simtime_t & startTime) {

    simtime_t busytime = 0;
    simtime_t intervalstart = simTime() - period;
    simtime_t intervalend = simTime();
    totalBusyTime = this->totalBusyTime;

    // the channel is currently busy
    if (isBusy) {
        busytime += intervalend - lastBusyTime;
    }

    //all busytime within the intervalstart and intervalend
    for (int i = 0; i < busyTimeVector.size(); i++) {
        if (busyTimeVector[i].startBusyTime >= intervalstart
                && busyTimeVector[i].startBusyTime <= intervalend) {

            busytime += busyTimeVector[i].busyTime;

        }
    }

    //all busytimes which overlapps the intervalstart
    for (int i = 0; i < busyTimeVector.size(); i++) {
        if (busyTimeVector[i].startBusyTime < intervalstart
                && busyTimeVector[i].endBusytime > intervalstart) {

            busytime += busyTimeVector[i].endBusytime - intervalstart;

        }
    }

    double rawCBP = 0.0;

    //add to vector for statistics
    if (busytime >= 0.0) {
        rawCBP = busytime * 100 / period;
        if (simTime() >= startTime + 6) {
            busyTimeVec.record(busytime);
            standardCBCVec.record(busytime / period);
        }

    } else {
        busytime = 0.0;
        if (simTime() >= startTime + 6)
            standardCBCVec.record(busytime / period);
    }

    ASSERT(rawCBP >= 0.0);

    if (simTime() - firstCall >= 6) {
        cleanVector();
        firstCall = simTime();
    }

    return rawCBP;

}

void ChannelBusyListenerWave::cleanVector() {

    simtime_t currentTime = simTime();

    for (int i = 0; i < busyTimeVector.size(); i++) {
        if (busyTimeVector[i].endBusytime < (currentTime - 6)) {
            busyTimeVector.erase(busyTimeVector.begin() + i);
        }
    }

}

double ChannelBusyListenerWave::calculateChannelBusyPercentage(simtime_t period,
        double vCBPWeightFactor, cOutVector & busyTimeVec,
        cOutVector & standardCBPVec, simtime_t & totalBusyTime,
        double testCaseNumber, simtime_t & startTime) {

    double CBP = 0.0;
    if (period > 0) {

        ASSERT(vCBPWeightFactor > 0.0);

        this->vCBPWeightFactor = vCBPWeightFactor;
        double rawCBP = this->calculateRawChannelBusyPercentage(period,
                busyTimeVec, standardCBPVec, totalBusyTime, testCaseNumber,
                startTime);

        // CBP(k) = vCBPWeightFactor x RawCBP(k) + (1 - vCBPWeightFactor) x CBP(k-1)
        double CBP = (vCBPWeightFactor * rawCBP
                + (1 - vCBPWeightFactor) * lastCBP); // --> result is Percentage!!!
        //

        //lastCBP is CBP(k-1)
        lastCBP = CBP;

        ASSERT(CBP > 0.0);

        return CBP;
    } else {
        totalBusyTime = this->totalBusyTime;
        lastCBP = CBP;
        return CBP;
    }
}



void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, long l, cObject *details) {
}

void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, unsigned long l, cObject *details) {

}
void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, double d, cObject *details) {

}
void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, const SimTime& t, cObject *details) {

}
void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, const char *s, cObject *details) {

}
void ChannelBusyListenerWave::receiveSignal(cComponent *source,
        simsignal_t signalID, cObject *obj, cObject *details) {
}
