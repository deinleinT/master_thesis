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

#ifndef SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_
#define SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_

#include <omnetpp.h>
#include <map>
#include <vector>
#include <assert.h>
#include <fstream>
#include <iostream>
#include "veins/masterthesis/util/BusyTime.h"
#include "veins/masterthesis/util/TestCaseHandler.h"

using namespace omnetpp;

class ChannelBusyListenerWave: public omnetpp::cIListener {
public:
    ChannelBusyListenerWave(omnetpp::cSimpleModule * source);
    virtual ~ChannelBusyListenerWave();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool b,
            cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long l,
            cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            unsigned long l, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            double d, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            const SimTime& t, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            const char *s, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            cObject *obj, cObject *details) override;
    double calculateChannelBusyPercentage(simtime_t period,
            double vCBPWeightFactor, cOutVector & busyTimeVec,
            cOutVector & standardCBPVec, simtime_t & totalBusyTime,
            double testCaseNumber, simtime_t & startTime);

protected:
    bool isIdle;
    bool isBusy;
    omnetpp::cSimpleModule * source;
    double lastCBP;
    double vCBPWeightFactor;
    std::vector<BusyTime> busyTimeVector;
    simtime_t lastBusyTime;
    simtime_t lastIdleTime;
    simtime_t firstCall;
    simtime_t totalBusyTime;

private:
    double calculateRawChannelBusyPercentage(simtime_t period,
            cOutVector & busyTimeVec, cOutVector & standardCBPVec,
            simtime_t & totalBusyTime, double testCaseNumber,
            simtime_t & startTime);
    void cleanVector();
    std::vector<BusyTime> performBusyTimeVecTest();

};

#endif /* SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_ */
