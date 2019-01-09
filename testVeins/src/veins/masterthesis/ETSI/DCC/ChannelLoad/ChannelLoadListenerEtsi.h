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

#ifndef SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_ETSI
#define SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_ETSI

#include <omnetpp.h>
#include <map>
#include <vector>
#include <assert.h>
#include <algorithm>
#include "veins/masterthesis/ETSI/DCC/StateMachine/States.h"
#include "veins/masterthesis/util/BusyTime.h"
#include "CBRValues.h"
#include "veins/masterthesis/util/TestCaseHandler.h"

using namespace omnetpp;

class ChannelLoadListenerEtsi: public omnetpp::cIListener {
public:
    ChannelLoadListenerEtsi(omnetpp::cSimpleModule * source,
            double * NDL_refPacketInterval, bool useGateKeeping);
    virtual ~ChannelLoadListenerEtsi();
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
    double calculateChannelLoad(simtime_t period, simtime_t & totalBusyTime,
            cOutVector & busyTimeVec, cOutVector & cbpVec, State & state,
            bool downFlag, double testCaseNumber);
    double getMinChannelLoad(simtime_t interval, simtime_t period);
    double getMaxChannelLoad(simtime_t interval, simtime_t period);
    double getLastCbr();
    double getErgoTargetRate();
    CBRValues & getCbrValues();
    CBRValues & setCbrValues();
    void calculateCBR_G();

protected:
    bool isIdle;
    bool isBusy;
    omnetpp::cSimpleModule * source;
    simtime_t lastBusyTime;
    simtime_t lastIdleTime;
    simtime_t firstCall;
    std::map<simtime_t, simtime_t> addedTimes;
    simtime_t totalBusyTime;
    std::vector<BusyTime> busyTimeVector;
    CBRValues cbrValues;
    CBRValues lastCbrValues;
    double lastCbr;
    double egoTargetRate;
    double currentCBR;
    double lastRJ;
    double lastR;
    double testCaseNumber;
    double * NDL_refPacketInterval;
    bool useGateKeeping;
private:
    void cleanVector();
    std::vector<BusyTime> performBusyTimeVecTest();
    void calculateTargetRate();

};

#endif /* SRC_VEINS_MASTERTHESIS_CHANNELBUSYLISTENER_H_ */
