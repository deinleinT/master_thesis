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

#include "ChannelLoadListenerEtsi.h"

ChannelLoadListenerEtsi::ChannelLoadListenerEtsi(
        omnetpp::cSimpleModule * source, double * NDL_refPacketInterval,
        bool useGateKeeping) :
        isIdle(false), isBusy(false) {
    this->source = source;
    this->firstCall = 0;
    this->totalBusyTime = 0;
    this->lastCbr = 0;
    this->egoTargetRate = 0;
    this->currentCBR = 0;
    this->lastR = 0;
    this->lastRJ = 0;
    this->testCaseNumber = 0;
    this->NDL_refPacketInterval = NDL_refPacketInterval;
    this->useGateKeeping = useGateKeeping;
}

ChannelLoadListenerEtsi::~ChannelLoadListenerEtsi() {

    addedTimes.clear();
    busyTimeVector.clear();
}

void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, bool busy, cObject *details) {

    if (busy) {
        // fill the vectors

        if (isBusy) {
            //do nothing
        } else {

            // busyVector.push_back(simTime());
            lastBusyTime = simTime();

        }

        isBusy = true;
        isIdle = false;

    } else {

        // fill the vectors
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

void ChannelLoadListenerEtsi::cleanVector() {

    simtime_t currentTime = simTime();

    for (int i = 0; i < busyTimeVector.size(); i++) {
        if (busyTimeVector[i].endBusytime < (currentTime - 6)) {
            busyTimeVector.erase(busyTimeVector.begin() + i);
        }
    }

    for (auto value : addedTimes) {
        if (value.first < (currentTime - 6)) {
            addedTimes.erase(value.first);
        }
    }

}

void ChannelLoadListenerEtsi::calculateCBR_G() {

    ASSERT(currentCBR >= 0);

    //
    cbrValues.CBR_L_0_Hop = currentCBR;

    std::vector<double> allCbrValues;
    allCbrValues.push_back(lastCbr);
    allCbrValues.push_back(cbrValues.CBR_L_1_Hop);
    allCbrValues.push_back(cbrValues.CBR_L_2_Hop);
    cbrValues.CBR_G = *std::max_element(allCbrValues.begin(),
            allCbrValues.end());

    lastCbrValues.CBR_L_0_Hop = cbrValues.CBR_L_0_Hop;
    lastCbrValues.CBR_R_0_Hop = cbrValues.CBR_R_0_Hop;
    lastCbrValues.CBR_L_1_Hop = cbrValues.CBR_L_1_Hop;
    lastCbrValues.CBR_L_2_Hop = cbrValues.CBR_L_2_Hop;
    lastCbrValues.CBR_R_1_Hop = cbrValues.CBR_R_1_Hop;
    lastCbrValues.CBR_G = cbrValues.CBR_G;

    allCbrValues.clear();
}

double ChannelLoadListenerEtsi::calculateChannelLoad(simtime_t period,
        simtime_t & totalBusyTime, cOutVector & busyTimeVec,
        cOutVector & cbpVec, State & state, bool downFlag,
        double testCaseNumber, simtime_t & startTime) {

    this->testCaseNumber = testCaseNumber;
    simtime_t busytime = 0;
    simtime_t intervalstart = simTime() - period;
    simtime_t intervalend = simTime();
    lastCbr = currentCBR;
    totalBusyTime = this->totalBusyTime;

    //for testCase 1.6
    if (testCaseNumber == 1.6) {
        // busyTime has to be 0.3
        busyTimeVector = performBusyTimeVecTest();
        intervalstart = 0.7;
        intervalend = 1.5;
        lastBusyTime = 1.4;
        isBusy = true;
        period = SimTime(intervalend - intervalstart);
    }

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

    double CBR = 0.0;

    if (busytime > 0) {

        CBR = busytime / period;

        if (state.stateType == ACTIVE) {

            if (!downFlag) {
                //1s interval
                addedTimes[simTime()] = busytime;
            } else {
                simtime_t tempBusyTime = 0.0;
                for (auto value : addedTimes) {
                    if (value.first > intervalstart
                            && value.first <= intervalend) {
                        tempBusyTime += value.second;
                    }
                }
                addedTimes[simTime()] = (busytime - tempBusyTime);
            }

        } else {
            addedTimes[simTime()] = busytime;
        }
    } else {
        busytime = 0.0;
        addedTimes[simTime()] = busytime;
    }
    //

    ASSERT(CBR >= 0.0);

    if (simTime() - firstCall >= 6) {
        cleanVector();
        firstCall = simTime();
    }

    //for testCase 1.6
    if (testCaseNumber == 1.6) {
        ASSERT(busytime == 0.5);
    } else if (testCaseNumber > 1 && testCaseNumber < 1.6) {
        CBR = TestCaseHandler::getTestValueETSI(testCaseNumber);
    }

    if (simTime() >= startTime + 6)
        busyTimeVec.record(busytime);
    if (CBR <= 1) {
        if (simTime() >= startTime + 6)
            cbpVec.record(CBR);
    }
    currentCBR = CBR;

    // see TS 102 687 V1.1.2
    calculateCBR_G();
    calculateTargetRate();
    CBRValues temp;
    cbrValues = temp;
    //

    return CBR;

}

CBRValues & ChannelLoadListenerEtsi::getLastCbrValues() {
    return lastCbrValues;
}

CBRValues & ChannelLoadListenerEtsi::getCbrValues() {
    return cbrValues;
}

double ChannelLoadListenerEtsi::getLastCbr() {
    return lastCbr;
}

double ChannelLoadListenerEtsi::getErgoTargetRate() {

    return egoTargetRate;
}

void ChannelLoadListenerEtsi::calculateTargetRate() {

    ASSERT(currentCBR >= 0);
    ASSERT(lastCbrValues.CBR_G >= 0);

    double CBR_DCC = fmax(currentCBR, lastCbrValues.CBR_G);
    double r = CBR_DCC * 2000.0;
    double rg = 0.8 * 2000; // is target_rate, constant for all vehicles, how successful reception of packets is

    int temp = 0;
    if ((rg - lastR) > 0)
        temp = 1;
    else
        temp = -1;

    //TS 102 687 V1.1.2 (Draft)
    double rj = (1.0 - 0.1) * lastRJ
            + temp * fmin(1.0, (1.0 / 150.0) * fabs(rg - lastR));

    egoTargetRate = rj;
    lastR = r;
    lastRJ = rj;

    if (useGateKeeping) {
        double T_GenPacket_DCC = 1 / egoTargetRate;
        if (T_GenPacket_DCC < 0) {
            *NDL_refPacketInterval = 0.1;
        } else if (T_GenPacket_DCC > 1) {
            *NDL_refPacketInterval = 1;
        } else {
            *NDL_refPacketInterval = T_GenPacket_DCC;
        }
    }

}

double ChannelLoadListenerEtsi::getMinChannelLoad(simtime_t interval,
        simtime_t period) {

    simtime_t intervalstart = simTime() - interval;
    std::vector<double> cbrVector;

    for (auto value : addedTimes) {
        if (value.first > intervalstart && value.first <= simTime()) {
            cbrVector.push_back(value.second / period);
        }
    }

    return *std::min_element(cbrVector.begin(), cbrVector.end());
}

double ChannelLoadListenerEtsi::getMaxChannelLoad(simtime_t interval,
        simtime_t period) {

    simtime_t intervalstart = simTime() - interval;
    std::vector<double> cbrVector;

    for (auto value : addedTimes) {
        if (value.first > intervalstart && value.first <= simTime()) {
            cbrVector.push_back(value.second / period);
        }
    }

    return *std::max_element(cbrVector.begin(), cbrVector.end());
}

std::vector<BusyTime> ChannelLoadListenerEtsi::performBusyTimeVecTest() {

    BusyTime one;
    one.startBusyTime = SimTime(0.5);
    one.endBusytime = SimTime(1.0);
    one.busyTime = one.endBusytime - one.startBusyTime;
    std::vector<BusyTime> vec;
    vec.push_back(one);

    BusyTime two;
    two.startBusyTime = SimTime(1.1);
    two.endBusytime = SimTime(1.2);
    two.busyTime = two.endBusytime - two.startBusyTime;
    vec.push_back(two);
    return vec;
}

void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, long l, cObject *details) {
}

void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, unsigned long l, cObject *details) {

}
void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, double d, cObject *details) {

}
void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, const SimTime& t, cObject *details) {

}
void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, const char *s, cObject *details) {

}
void ChannelLoadListenerEtsi::receiveSignal(cComponent *source,
        simsignal_t signalID, cObject *obj, cObject *details) {

}
