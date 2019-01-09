//
// Copyright (C) 2012 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include <iterator>

#include "veins/modules/phy/DeciderResult80211.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/modules/messages/PhyControlMessage_m.h"

//#if OMNETPP_VERSION >= 0x500
#define OWNER owner->
//#else
//#define OWNER
//#endif

#define DBG_MAC EV
//#define DBG_MAC std::cerr << "[" << simTime().raw() << "] " << myId << " "

Define_Module(Mac1609_4);

ChannelBusyListenerWave * Mac1609_4::getChannelBusyWaveListener() {
    return listenerWAVE;
}

ChannelLoadListenerEtsi * Mac1609_4::getChannelLoadListenerETSI() {
    return listenerNewMethod;
}

void Mac1609_4::initialize(int stage) {
    BaseMacLayer::initialize(stage);
    if (stage == 0) {

        phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(
                getParentModule());
        assert(phy11p);

        //this is required to circumvent double precision issues with constants from CONST80211p.h
        assert(simTime().getScaleExp() == -12);

        sigChannelBusy = registerSignal("sigChannelBusy");
        sigCollision = registerSignal("sigCollision");

        n_dbps = 0;

        //mac-adresses
        myMacAddress = intuniform(0, 0xFFFFFFFE);
        myId = getParentModule()->getParentModule()->getFullPath();

        //create two edca systems
        //  WAVE --> in SAE J2945, Page 58, are other Values
        beaconWAVE =
                getParentModule()->getParentModule()->par("beaconWAVE").boolValue();
        beaconETSI =
                getParentModule()->getParentModule()->par("beaconETSI").boolValue();

        testCaseNumber = getParentModule()->getParentModule()->par(
                "testCaseNumber").doubleValue();

        myEDCA[type_CCH] = new EDCA(this, type_CCH,
                par("NDL_queueLength").longValue());

        myEDCA[type_CCH]->myId = myId;
        myEDCA[type_CCH]->myId.append(" CCH");

        myEDCA[type_SCH] = new EDCA(this, type_SCH,
                par("queueSize").longValue());
        myEDCA[type_SCH]->myId = myId;
        myEDCA[type_SCH]->myId.append(" SCH");

        if (beaconWAVE) {

            //create frequency mappings
            frequency.insert(
                    std::pair<int, double>(Channels::CRIT_SOL, 5.86e9));
            frequency.insert(std::pair<int, double>(Channels::SCH1, 5.87e9));
            frequency.insert(std::pair<int, double>(Channels::SCH2, 5.88e9));
            frequency.insert(std::pair<int, double>(Channels::CCH, 5.89e9));
            frequency.insert(std::pair<int, double>(Channels::SCH3, 5.90e9));
            frequency.insert(std::pair<int, double>(Channels::SCH4, 5.91e9));
            frequency.insert(std::pair<int, double>(Channels::HPPS, 5.92e9));

            //
            //params set as described in SAE J2945, Page 58, 6.3.4
            //BSM with critical flag is AC_VO, else it is AC_VI
            //CCH
            myEDCA[type_CCH]->createQueue(2, 3, 7, AC_VO);
            myEDCA[type_CCH]->createQueue(4, CWMIN_11P, CWMAX_11P, AC_VI);
            myEDCA[type_CCH]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
            myEDCA[type_CCH]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

            //SCH --> no changes
            myEDCA[type_SCH]->createQueue(2, 3, 7, AC_VO);
            myEDCA[type_SCH]->createQueue(4, CWMIN_11P, CWMAX_11P, AC_VI);
            myEDCA[type_SCH]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
            myEDCA[type_SCH]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

            txPower = FWMath::dBm2mW(par("txPowerWAVE").doubleValue());
            bitrate = par("bitrateWAVE").doubleValue();

            //changed, needed for calculation CBP in BaseWaveAppLayer
            listenerWAVE = new ChannelBusyListenerWave(this);
            subscribe(sigChannelBusy, listenerWAVE);
            listenerNewMethod = nullptr;
            //

            txPowerInDBMVec.setName("TXPower in dBm");
            txPowerInMWVec.setName("TXPower in mW");
            carrierSenseVec.setName("CarrierSense in dBm");
            datarateVec.setName("Datarate in Mbps");
            nextMacEventTime.setName("Next Mac Event Time");

            //Validation
            TestCaseHandler::getDescription(testCaseNumber);
            if (testCaseNumber > 9) {
                std::string text = "Unknown TestCaseNumber "
                        + omnetpp::double_to_str(testCaseNumber);
                throw omnetpp::cRuntimeError(text.c_str());
            }
            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {

                std::cerr << " WAVE Mode " << testCaseNumber << " "
                        << TestCaseHandler::getDescription(testCaseNumber)
                        << std::endl;

                if (useSCH) {
                    std::cerr << "Service Channel ON" << std::endl;
                } else {
                    std::cerr << "Service Channel OFF" << std::endl;
                }
            }

        } else if (beaconETSI) {

            //create frequency mappings
            frequency.insert(
                    std::pair<int, double>(Channels::SCH1_ETSI, 5.86e9));
            frequency.insert(
                    std::pair<int, double>(Channels::SCH2_ETSI, 5.87e9));
            frequency.insert(
                    std::pair<int, double>(Channels::SCH3_ETSI, 5.88e9));
            frequency.insert(
                    std::pair<int, double>(Channels::SCH4_ETSI, 5.89e9));
            frequency.insert(
                    std::pair<int, double>(Channels::CCH_ETSI, 5.90e9));

            listenerWAVE = nullptr;

            bitrate = par("NDL_defDatarate").doubleValue();
            txPower = par("NDL_defTxPower").doubleValue();
            NDL_defPacketInterval = par("NDL_defPacketInterval").doubleValue();
            NDL_timeUp = par("NDL_timeUp").doubleValue();
            NDL_timeDown = par("NDL_timeDown").doubleValue();

            NDL_minTxPower = par("NDL_minTxPower").doubleValue();
            NDL_maxTxPower = par("NDL_maxTxPower").doubleValue();
            NDL_defDatarate = par("NDL_defDatarate").doubleValue();
            NDL_minDatarate = par("NDL_minDatarate").doubleValue();
            NDL_maxDatarate = par("NDL_maxDatarate").doubleValue();
            NDL_minPacketInterval = par("NDL_minPacketInterval").doubleValue();
            NDL_maxPacketInterval = par("NDL_maxPacketInterval").doubleValue();
            NDL_maxPacketDuration = par("NDL_maxPacketDuration").doubleValue();
            NDL_defCarrierSense = par("NDL_defCarrierSense").doubleValue();
            NDL_minCarrierSense = par("NDL_minCarrierSense").doubleValue();
            NDL_maxCarrierSense = par("NDL_maxCarrierSense").doubleValue();
            NDL_minChannelLoad = par("NDL_minChannelLoad").doubleValue();
            NDL_maxChannelLoad = par("NDL_maxChannelLoad").doubleValue();
            NDL_activeTxPower = par("NDL_activeTxPower").doubleValue();
            NDL_refPacketInterval = NDL_defPacketInterval;
            NDL_queueLength = par("NDL_queueLength").longValue();

            timer_NDL_timeUp = new cMessage("timeUp");
            timer_NDL_timeDown = new cMessage("timeDown");
            timer_DCC_Toff = new cMessage("dccToff");

            totalChannelLoadTime = 0.0;
            lastTimeChannelLoadTimeUpCalculated = 0;
            lastTimePacketSent = 0;

            cbpVec.setName("ChannelLoad");
            busyTimeVec.setName("busyTime in Mac");
            txPowerInDBMVec.setName("TXPower in dBm");
            txPowerInMWVec.setName("TXPower in mW");
            carrierSenseVec.setName("CarrierSense in dBm");
            datarateVec.setName("Datarate in Mbps");
            packetIntervalVec.setName("PaketInterval in s");
            sentPacketVec.setName("Sent Packets time");
            stateVec.setName("State Vec, 0=RELAXED, 1=ACTIVE, 2=RESTRICTIVE");
            nextMacEventTime.setName("Next Mac Event Time");
            useGateKeeping = par("useGateKeeping").boolValue();
            if (useGateKeeping && par("useDCCProfileCAM").boolValue()) {
                throw cRuntimeError(
                        "Not possible to run gatekeeping and dccprofile!");
            }
            countRelaxed = 0;
            countActive = 0;
            countRestrictive = 0;

            //init DCC States
            relaxed = State(StateType::RELAXED, NDL_maxTxPower, NDL_minDatarate,
                    NDL_minPacketInterval, NDL_minCarrierSense);
            active = State(StateType::ACTIVE, NDL_activeTxPower,
                    NDL_minDatarate, NDL_minPacketInterval,
                    NDL_minCarrierSense);
            restrictive = State(StateType::RESTRICTIVE, NDL_minTxPower,
                    NDL_maxDatarate, NDL_maxPacketInterval,
                    NDL_maxCarrierSense);
            currentState = nullptr;

            //use DCCProfileCAM
            if (par("useDCCProfileCAM").boolValue()) {
                relaxed.packetInterval = par(
                        "NDL_maxPacketIntervalDCCProfileRELAXED").doubleValue();
                active.packetInterval = par(
                        "NDL_maxPacketIntervalDCCProfileACTIVE").doubleValue();
                restrictive.packetInterval =
                        par("NDL_maxPacketIntervalDCCProfileRESTRICTIVE").doubleValue();
                NDL_minPacketInterval = relaxed.packetInterval;
                NDL_maxPacketInterval = restrictive.packetInterval;
            }
            //

            // EN 302 663 V1.2.1 (2013-07), TS 102 636-4-2 V1.1.1 (2013-10)
            myEDCA[type_CCH]->createQueue(2, 3, 7, AC_VO);
            myEDCA[type_CCH]->createQueue(3, 7, 15, AC_VI);
            myEDCA[type_CCH]->createQueue(6, 15, 1023, AC_BE);
            myEDCA[type_CCH]->createQueue(9, 15, 1023, AC_BK);

            //no changes to SCH
            myEDCA[type_SCH]->createQueue(2, (((CWMIN_11P + 1) / 4) - 1),
                    (((CWMIN_11P + 1) / 2) - 1), AC_VO);
            myEDCA[type_SCH]->createQueue(3, (((CWMIN_11P + 1) / 4) - 1),
                    (((CWMIN_11P + 1) / 2) - 1), AC_VI);
            myEDCA[type_SCH]->createQueue(6, (((CWMIN_11P + 1) / 2) - 1),
                    CWMIN_11P, AC_BE);
            myEDCA[type_SCH]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

            timer_NDL_channelLoad = new cMessage("ChannelLoadTimer");
            channelLoadTimer = par("channelLoadTimer").doubleValue();
            listenerNewMethod = new ChannelLoadListenerEtsi(this,
                    &NDL_refPacketInterval, useGateKeeping);
            subscribe(sigChannelBusy, listenerNewMethod);
            scheduleAt(simTime() + channelLoadTimer, timer_NDL_channelLoad);
            //

            if ((testCaseNumber >= 2 && testCaseNumber < 8)
                    || testCaseNumber >= 10) {
                std::string text = "Unknown TestCaseNumber "
                        + omnetpp::double_to_str(testCaseNumber);
                throw omnetpp::cRuntimeError(text.c_str());
            }
            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {

                std::cerr << "ETSI Mode " << testCaseNumber
                        << TestCaseHandler::getDescription(testCaseNumber)
                        << std::endl;

                if (useSCH) {
                    std::cerr << "Service Channel ON" << std::endl;
                } else {
                    std::cerr << "Service Channel OFF channelLoadTimer "
                            << channelLoadTimer << std::endl;
                }
                if (testCaseNumber > 0) {
                    std::cerr << " Test Case with Number " << testCaseNumber
                            << std::endl;
                }
            }

            setDCCState(0.0, false);

        } else {

            //create frequency mappings
            frequency.insert(
                    std::pair<int, double>(Channels::CRIT_SOL, 5.86e9));
            frequency.insert(std::pair<int, double>(Channels::SCH1, 5.87e9));
            frequency.insert(std::pair<int, double>(Channels::SCH2, 5.88e9));
            frequency.insert(std::pair<int, double>(Channels::CCH, 5.89e9));
            frequency.insert(std::pair<int, double>(Channels::SCH3, 5.90e9));
            frequency.insert(std::pair<int, double>(Channels::SCH4, 5.91e9));
            frequency.insert(std::pair<int, double>(Channels::HPPS, 5.92e9));

            //CCH
            myEDCA[type_CCH]->createQueue(2, (((CWMIN_11P + 1) / 4) - 1),
                    (((CWMIN_11P + 1) / 2) - 1), AC_VO);
            myEDCA[type_CCH]->createQueue(3, (((CWMIN_11P + 1) / 2) - 1),
                    CWMIN_11P, AC_VI);
            myEDCA[type_CCH]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
            myEDCA[type_CCH]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);

            //SCH
            myEDCA[type_SCH]->createQueue(2, (((CWMIN_11P + 1) / 4) - 1),
                    (((CWMIN_11P + 1) / 2) - 1), AC_VO);
            myEDCA[type_SCH]->createQueue(3, (((CWMIN_11P + 1) / 2) - 1),
                    CWMIN_11P, AC_VI);
            myEDCA[type_SCH]->createQueue(6, CWMIN_11P, CWMAX_11P, AC_BE);
            myEDCA[type_SCH]->createQueue(9, CWMIN_11P, CWMAX_11P, AC_BK);
            txPower = par("txPower").doubleValue();
            bitrate = par("bitrate").longValue();
        }
        setParametersForBitrate(bitrate);

        useSCH = par("useServiceChannel").boolValue();
        if (useSCH) {
            //set the initial service channel
            switch (par("serviceChannel").longValue()) {
            case 1:
                mySCH = Channels::SCH1;
                break;
            case 2:
                mySCH = Channels::SCH2;
                break;
            case 3:
                mySCH = Channels::SCH3;
                break;
            case 4:
                mySCH = Channels::SCH4;
                break;
            default:
                throw cRuntimeError("Service Channel must be between 1 and 4");
                break;
            }
        }

        headerLength = par("headerLength");
        nextMacEvent = new cMessage("next Mac Event");

        if (useSCH) {
            // introduce a little asynchronization between radios, but no more than .3 milliseconds
            uint64_t currenTime = simTime().raw();
            uint64_t switchingTime = SWITCHING_INTERVAL_11P.raw();
            double timeToNextSwitch = (double) (switchingTime
                    - (currenTime % switchingTime)) / simTime().getScale();
            if ((currenTime / switchingTime) % 2 == 0) {
                setActiveChannel(type_CCH);
            } else {
                setActiveChannel(type_SCH);
            }

            // channel switching active
            nextChannelSwitch = new cMessage("Channel Switch");
            simtime_t offset = dblrand() * par("syncOffset").doubleValue();
            scheduleAt(simTime() + offset + timeToNextSwitch,
                    nextChannelSwitch);
        } else {
            // no channel switching
            nextChannelSwitch = 0;
            setActiveChannel(type_CCH);
        }

        //stats
        statsReceivedPackets = 0;
        statsReceivedBroadcasts = 0;
        statsSentPackets = 0;
        statsTXRXLostPackets = 0;
        statsSNIRLostPackets = 0;
        statsDroppedPackets = 0;
        statsNumTooLittleTime = 0;
        statsNumInternalContention = 0;
        statsNumBackoff = 0;
        statsSlotsBackoff = 0;
        statsTotalBusyTime = 0;

        idleChannel = true;
        lastBusy = simTime();
        channelIdle(true);

    }

}

void Mac1609_4::cancelNDLTimerUpAndDown() {
    //timer 1 und 5 s notwendig
    if (timer_NDL_timeUp->isScheduled()) {
        cancelEvent(timer_NDL_timeUp);
    }
    if (timer_NDL_timeDown->isScheduled()) {
        cancelEvent(timer_NDL_timeDown);
    }
}

void Mac1609_4::setStateParams(State & state) {

    currentState = &state;
    this->setCCAThreshold(currentState->carrierSense);
    this->setTxPower(FWMath::dBm2mW(currentState->txPower));
    this->bitrate = currentState->datarate;
    NDL_refPacketInterval = currentState->packetInterval;

    txPowerInDBMVec.record(currentState->txPower);
    carrierSenseVec.record(currentState->carrierSense);
    if (!useGateKeeping)
        packetIntervalVec.record(NDL_refPacketInterval);

    BaseWaveApplLayer * appLayer =
            FindModule<BaseWaveApplLayer*>::findSubModule(
                    getParentModule()->getParentModule());

    if (!useGateKeeping)
        appLayer->setPaketInterval(NDL_refPacketInterval, false);

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0) {
        std::cerr << getParentModule()->getParentModule()->getFullName()
                << " setStateParams " << simTime() << " currentState "
                << currentState->stateType << " carrierSense "
                << currentState->carrierSense << " txPower in dBm "
                << currentState->txPower << " txPower in mW "
                << FWMath::dBm2mW(currentState->txPower) << " bitrate "
                << currentState->datarate << " packetInterval "
                << NDL_refPacketInterval << std::endl;
    }
}

void Mac1609_4::setDCCState(double channelLoad, bool downFlag) {

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0) {
        if (downFlag) {
            std::cerr << getParentModule()->getParentModule()->getFullName()
                    << "\nSetDCCState Time " << simTime() << " MAXchannelLoad "
                    << channelLoad << " downFlag " << downFlag << std::endl;
        } else {
            std::cerr << getParentModule()->getParentModule()->getFullName()
                    << "\nSetDCCState Time " << simTime() << " MINchannelLoad "
                    << channelLoad << " downFlag " << downFlag << std::endl;
        }
    }
//set initial values
    if (currentState == nullptr) {

        cancelNDLTimerUpAndDown();

        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0) {
            std::cerr << getParentModule()->getParentModule()->getFullName()
                    << "currentState is nullptr, set state to RELAXED "
                    << std::endl;
        }
        setStateParams(relaxed);
        stateVec.record(0);

        scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
        //set the timer

    } else if (currentState->stateType == RELAXED) {
        countRelaxed++;
        stateVec.record(0);

        //check channelload in the next second
        ASSERT(!downFlag);

        //downFlag --> true, if method is called after NDL_timeDown interval
        if (channelLoad >= NDL_minChannelLoad) {

            //cancel NDL_timers
            cancelNDLTimerUpAndDown();

            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {
                std::cerr << getParentModule()->getParentModule()->getFullName()
                        << "currentState is RELAXED, set state to ACTIVE "
                        << std::endl;
            }
            // set params to ACTIVE State
            setStateParams(active);
            stateVec.record(1);
            //

            //schedule timers
            scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
            scheduleAt(simTime() + NDL_timeDown, timer_NDL_timeDown);

        } else {
            //else do nothing, Stay in relaxed State --> timeUp interval remains
            cancelNDLTimerUpAndDown();
            scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
        }

    } else if (currentState->stateType == ACTIVE) {

        countActive++;
        stateVec.record(1);

        //5s and lower than minChannelLoad --> set State to RELAXED
        if (downFlag && channelLoad < NDL_minChannelLoad) {

            cancelNDLTimerUpAndDown();

            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {
                std::cerr << getParentModule()->getParentModule()->getFullName()
                        << "currentState is ACTIVE, set state to RELAXED "
                        << std::endl;
            }
            // set State to RELAXED
            setStateParams(relaxed);
            stateVec.record(0);
            //
            scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
            return;
        }

        //--> new State is RESTRICTIVE
        //after 1 s channelLoad is larger than maxChannelload
        else if (!downFlag && channelLoad >= NDL_maxChannelLoad) {

            cancelNDLTimerUpAndDown();

            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {
                std::cerr << getParentModule()->getParentModule()->getFullName()
                        << "currentState is ACTIVE, set state to RESTRICTIVE "
                        << std::endl;
            }
            //set State to RESTRICTIVE
            setStateParams(restrictive);
            stateVec.record(2);
            //schedule timers

            scheduleAt(simTime() + NDL_timeDown, timer_NDL_timeDown);
            return;
        }

        else if (downFlag) {
            //after 5 seconds stay in active state --> reschedule

            if (timer_NDL_timeDown->isScheduled()) {
                cancelEvent(timer_NDL_timeDown);
            }

            scheduleAt(simTime() + NDL_timeDown, timer_NDL_timeDown);
            return;

        } else if (!downFlag) {

            //after 1 s
            if (timer_NDL_timeUp->isScheduled()) {
                cancelEvent(timer_NDL_timeUp);
            }
            scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
            return;

        }
    }

    else if (currentState->stateType == RESTRICTIVE) {

        countRestrictive++;
        stateVec.record(2);

        ASSERT(downFlag);

        // after 5s channelLoad lower than maxchannelload --> switch to ACTIVE State
        if (downFlag && channelLoad < NDL_maxChannelLoad) {

            cancelNDLTimerUpAndDown();

            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0) {
                std::cerr << getParentModule()->getParentModule()->getFullName()
                        << "currentState is RESTRICTIVE, set state to ACTIVE "
                        << std::endl;
            }
            //set ACTIVE State
            setStateParams(active);
            stateVec.record(1);

            //schedule  timer
            scheduleAt(simTime() + NDL_timeUp, timer_NDL_timeUp);
            scheduleAt(simTime() + NDL_timeDown, timer_NDL_timeDown);

        } else {
            cancelNDLTimerUpAndDown();
            scheduleAt(simTime() + NDL_timeDown, timer_NDL_timeDown);
        }

    } else {
        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0) {
            std::cerr << getParentModule()->getParentModule()->getFullName()
                    << "ERROR SetDCCState Time " << simTime() << " channelLoad "
                    << channelLoad << " downFlag " << downFlag
                    << " currentState " << currentState->stateType << std::endl;
        }
    }

}

void Mac1609_4::handleSelfMsg(cMessage* msg) {

    if (msg == nextChannelSwitch) {
        ASSERT(useSCH);

        scheduleAt(simTime() + SWITCHING_INTERVAL_11P, nextChannelSwitch);

        switch (activeChannel) {
        case type_CCH:
            DBG_MAC << "CCH --> SCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_SCH);
            channelIdle(true);
            phy11p->changeListeningFrequency(frequency[mySCH]);
            break;
        case type_SCH:
            DBG_MAC << "SCH --> CCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_CCH);
            channelIdle(true);
            phy11p->changeListeningFrequency(frequency[Channels::CCH]);
            break;
        }
        //schedule next channel switch in 50ms

    } else if (msg == nextMacEvent) {
        // save bitrate in WAVE Mode
        if (beaconWAVE) {
            datarateVec.record(bitrate);

            BaseWaveApplLayer * appLayer =
                    FindModule<BaseWaveApplLayer*>::findSubModule(
                            getParentModule()->getParentModule());
            setCCAThreshold(appLayer->getRxSens());
        }

        //we actually came to the point where we can send a packet

        channelBusySelf(true);
        WaveShortMessage* pktToSend = myEDCA[activeChannel]->initiateTransmit(
                lastIdle, beaconETSI);

        lastAC = mapPriority(pktToSend->getPriority());

        DBG_MAC << "MacEvent received. Trying to send packet with priority"
                       << lastAC << std::endl;

        //send the packet
        Mac80211Pkt* mac = new Mac80211Pkt(pktToSend->getName(),
                pktToSend->getKind());
        mac->setDestAddr(LAddress::L2BROADCAST());
        mac->setSrcAddr(myMacAddress);
        mac->encapsulate(pktToSend->dup());

        enum PHY_MCS mcs;
        double txPower_mW;
        uint64_t datarate;
        PhyControlMessage *controlInfo =
                dynamic_cast<PhyControlMessage *>(pktToSend->getControlInfo());
        if (controlInfo) {
            //if MCS is not specified, just use the default one
            mcs = (enum PHY_MCS) controlInfo->getMcs();
            if (mcs != MCS_DEFAULT) {
                datarate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
            } else {
                datarate = bitrate;
            }
            //apply the same principle to tx power
            txPower_mW = controlInfo->getTxPower_mW();
            if (txPower_mW < 0) {
                txPower_mW = txPower;
            }
        } else {
            mcs = MCS_DEFAULT;
            txPower_mW = txPower;
            datarate = bitrate;
        }

        //
        ASSERT(mcs == MCS_DEFAULT);
        //read txPower and datarate from packet
        txPower_mW = pktToSend->getTxPower();

        if (beaconETSI) {
            datarate = pktToSend->getBitrate();
            datarateVec.record(currentState->datarate);
        } else if (beaconWAVE) {
            datarateVec.record(datarate);
        }

        ASSERT(txPower_mW > 0 && datarate > 0);
        txPowerInMWVec.record(txPower_mW);

        sendingDuration = RADIODELAY_11P
                + getFrameDuration(mac->getBitLength(), mcs);

        DBG_MAC << "Sending duration will be" << sendingDuration << std::endl;

        if ((!useSCH) || (timeLeftInSlot() > sendingDuration)) {
            if (useSCH) {
                DBG_MAC << " Time in this slot left: " << timeLeftInSlot()
                               << std::endl;
            }
            // give time for the radio to be in Tx state before transmitting
            phy->setRadioState(Radio::TX);

            double freq = 0.0;

            if (beaconETSI) {
                freq = (activeChannel == type_CCH) ?
                        frequency[Channels::CCH_ETSI] : frequency[mySCH];
            } else {
                freq = (activeChannel == type_CCH) ?
                        frequency[Channels::CCH] : frequency[mySCH];
            }

            attachSignal(mac, simTime() + RADIODELAY_11P, freq, datarate,
                    txPower_mW);
            MacToPhyControlInfo* phyInfo =
                    dynamic_cast<MacToPhyControlInfo*>(mac->getControlInfo());
            assert(phyInfo);
            DBG_MAC << "Sending a Packet. Frequency " << freq << " Priority"
                           << lastAC << std::endl;

            sendDelayed(mac, RADIODELAY_11P, lowerLayerOut);

            if (beaconETSI) {
                lastTimePacketSent = simTime();
                sentPacketVec.record(simTime());
            }
            statsSentPackets++;

            //
            if (strcmp(getParentModule()->getParentModule()->getFullName(),
                    "node[0]") == 0 && testCaseNumber > 0) {
                std::cout << getParentModule()->getParentModule()->getFullName()
                        << " NextMacEvent at " << simTime() << " activeChannel "
                        << activeChannel << " " << pktToSend->getName()
                        << std::endl;
            }

        } else {   //not enough time left now
            DBG_MAC
                           << "Too little Time left. This packet cannot be send in this slot."
                           << std::endl;

            statsNumTooLittleTime++;
            //revoke TXOP
            myEDCA[activeChannel]->revokeTxOPs();
            delete mac;
            channelIdle();
            //do nothing. contention will automatically start after channel switch
        }
    } else if (msg == timer_NDL_timeUp) {

        //
        //check state
        //calculate channelLoad

        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0) {
            std::cout << getParentModule()->getParentModule()->getFullName()
                    << "\nTimeUp after 1s call MinChannelLoad at " << simTime()
                    << std::endl;
        }

        if (lastTimeChannelLoadTimeUpCalculated == 0) {
            lastTimeChannelLoadTimeUpCalculated = simTime();
        }

        // changed

        double channelLoad = 0.0;

        if (testCaseNumber == 0.0 || testCaseNumber == 1.6
                || (testCaseNumber >= 8 && testCaseNumber <= 9.3)) {
            channelLoad = listenerNewMethod->getMinChannelLoad(NDL_timeUp,
                    channelLoadTimer);
        } else {

            channelLoad = TestCaseHandler::getTestValueETSI(testCaseNumber);
        }

        setDCCState(channelLoad, false);

        if (testCaseNumber == 1.1)
            ASSERT(currentState->stateType == RELAXED);

    } else if (msg == timer_NDL_timeDown) {

        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0) {
            std::cout << getParentModule()->getParentModule()->getFullName()
                    << "\nTimer down after 5s call MaxChannelLoad at "
                    << simTime() << std::endl;
        }

        double channelLoad = 0.0;

        if (testCaseNumber == 0.0 || testCaseNumber == 1.6
                || (testCaseNumber >= 8 && testCaseNumber <= 9.3)) {
            channelLoad = listenerNewMethod->getMaxChannelLoad(NDL_timeDown,
                    channelLoadTimer);
        } else {

            channelLoad = TestCaseHandler::getTestValueETSI(testCaseNumber);
        }

        setDCCState(channelLoad, true);

        if (testCaseNumber == 1.1)
            ASSERT(currentState->stateType == RELAXED);
        else if (testCaseNumber == 1.2 || testCaseNumber == 1.3)
            ASSERT(currentState->stateType == ACTIVE);

    } else if (msg == timer_NDL_channelLoad) {

        // addedTimesMap filled with busytimes
        listenerNewMethod->calculateChannelLoad(channelLoadTimer,
                totalChannelLoadTime, busyTimeVec, cbpVec, *currentState, false,
                testCaseNumber);
        lastTimeChannelLoadTimeUpCalculated = simTime();

        //set the current Toff Value
        BaseWaveApplLayer * appLayer =
                FindModule<BaseWaveApplLayer*>::findSubModule(
                        getParentModule()->getParentModule());

        if (useGateKeeping) {
            appLayer->setPaketInterval(NDL_refPacketInterval, false);
        }
        //

        scheduleAt(simTime() + channelLoadTimer, timer_NDL_channelLoad);

    } else if (msg == timer_DCC_Toff) {

        simtime_t currentTime = simTime();

        if (testCaseNumber == 9.1) {
            while (dccQueue.size() > 0) {
                dccQueue.pop();
            }
            WaveShortMessage * temp = new WaveShortMessage("test");
            temp->setTimestamp(0.1);
            lastTimePacketQueued = 1;
            NDL_refPacketInterval = 0.1;
            currentTime = 1.11;
            dccQueue.push(temp);
        }

        //DccFlowControl
        //Toff is over, we're allowed to queue next packet
        if (lastTimePacketQueued + NDL_refPacketInterval <= currentTime) {
            if (testCaseNumber == 9.1) {
                ASSERT(dccQueue.size() == 1);
            }

            //queue packet to edca, if lifetime not exceeded
            while (dccQueue.size() > 0) {

                WaveShortMessage * thisMsg = dccQueue.front();

                if (thisMsg->getTimestamp() + 1 >= currentTime) {

                    t_access_category ac = mapPriority(thisMsg->getPriority());

                    t_channel chan;
                    //put this packet in its queue
                    if (thisMsg->getChannelNumber() == Channels::CCH
                            || thisMsg->getChannelNumber()
                                    == Channels::CCH_ETSI) {
                        chan = type_CCH;
                    }

                    int num = myEDCA[chan]->queuePacket(ac, thisMsg);

                    //packet was dropped in Mac, cause EDCA Queue is full
                    if (num == -1) {
                        statsDroppedPackets++;
                        break;
                    }

                    //if this packet is not at the front of a new queue we dont have to reevaluate times
                    DBG_MAC << "sorted packet into queue of EDCA " << chan
                                   << " this packet is now at position: " << num
                                   << std::endl;

                    if (chan == activeChannel) {
                        DBG_MAC
                                       << "this packet is for the currently active channel"
                                       << std::endl;
                    } else {
                        DBG_MAC
                                       << "this packet is NOT for the currently active channel"
                                       << std::endl;
                    }

                    if (num == 1 && idleChannel == true
                            && chan == activeChannel) {

                        nextEvent = myEDCA[chan]->startContent(lastIdle,
                                guardActive());
                        nextMacEventTime.record(nextEvent);

                        if (nextEvent != -1) {
                            if ((!useSCH)
                                    || (nextEvent
                                            <= nextChannelSwitch->getArrivalTime())) {

                                if (nextMacEvent->isScheduled()) {
                                    cancelEvent(nextMacEvent);
                                }

                                scheduleAt(nextEvent, nextMacEvent);

                                DBG_MAC << "Updated nextMacEvent:"
                                               << nextMacEvent->getArrivalTime().raw()
                                               << std::endl;
                            } else {
                                DBG_MAC
                                               << "Too little time in this interval. Will not schedule nextMacEvent"
                                               << std::endl;

                                //it is possible that this queue has an txop. we have to revoke it
                                myEDCA[activeChannel]->revokeTxOPs();
                                statsNumTooLittleTime++;
                            }
                        } else {
                            cancelEvent(nextMacEvent);
                        }
                    }
                    if (num == 1 && idleChannel == false
                            && myEDCA[chan]->myQueues[ac].currentBackoff == 0
                            && chan == activeChannel) {
                        myEDCA[chan]->backoff(ac, this);
                    }

                    //shows that this part is not executed in TestCase 9.1
                    if (testCaseNumber == 9.1) {
                        ASSERT(false);
                    }

                    dccQueue.pop();
                    lastTimePacketQueued = simTime();

                    std::cerr
                            << getParentModule()->getParentModule()->getFullName()
                            << " queue packet EDCA " << thisMsg->getTimestamp()
                            << " now is " << simTime() << " dccQueue.size "
                            << dccQueue.size() << std::endl;

                    break;
                } else {

                    std::cerr
                            << getParentModule()->getParentModule()->getFullName()
                            << " drop packet cause lifetime is over "
                            << thisMsg->getTimestamp() << " now is "
                            << simTime() << " dccQueue.size " << dccQueue.size()
                            << std::endl;

                    statsLifeTimeDropPackets++;
                    dccQueue.pop();
                    delete thisMsg;

                    if (testCaseNumber == 9.1) {
                        ASSERT(dccQueue.size() == 0);
                    }
                }
            }
        } else {
            std::cerr << simTime() << " "
                    << getParentModule()->getParentModule()->getFullName()
                    << " do nothing size is " << dccQueue.size()
                    << " lastTimeQueue " << lastTimePacketQueued
                    << " NDL_refPacketInterval " << NDL_refPacketInterval
                    << std::endl;
        }

        if (testCaseNumber == 9.1) {
            ASSERT(dccQueue.size() == 0);
        }

        if (dccQueue.size() > 0) {
            scheduleAt(lastTimePacketQueued + NDL_refPacketInterval,
                    timer_DCC_Toff);
        }
    }
}

void Mac1609_4::handleUpperControl(cMessage* msg) {
    assert(false);
}

void Mac1609_4::handleUpperMsg(cMessage* msg) {

    if (testCaseNumber == 9.2) {
        WaveShortMessage * temp1 = new WaveShortMessage();
        WaveShortMessage * temp2 = new WaveShortMessage();

        while (dccQueue.size() > 0) {
            dccQueue.pop();
        }

        dccQueue.push(temp1);
        dccQueue.push(temp2);
    }

    WaveShortMessage* thisMsg;
    if ((thisMsg = dynamic_cast<WaveShortMessage*>(msg)) == NULL) {
        error("WaveMac only accepts WaveShortMessages");
    }

    if (!beaconETSI) {

        t_access_category ac = mapPriority(thisMsg->getPriority());

        DBG_MAC << "Received a message from upper layer for channel "
                       << thisMsg->getChannelNumber()
                       << " Access Category (Priority):  " << ac << std::endl;

        t_channel chan;
        //put this packet in its queue
        if (thisMsg->getChannelNumber() == Channels::CCH
                || thisMsg->getChannelNumber() == Channels::CCH_ETSI) {
            chan = type_CCH;
        }

        int num = myEDCA[chan]->queuePacket(ac, thisMsg);

        //packet was dropped in Mac
        if (num == -1) {
            statsDroppedPackets++;
            return;
        }

        //if this packet is not at the front of a new queue we dont have to reevaluate times
        DBG_MAC << "sorted packet into queue of EDCA " << chan
                       << " this packet is now at position: " << num
                       << std::endl;

        if (chan == activeChannel) {
            DBG_MAC << "this packet is for the currently active channel"
                           << std::endl;
        } else {
            DBG_MAC << "this packet is NOT for the currently active channel"
                           << std::endl;
        }

        if (num == 1 && idleChannel == true && chan == activeChannel) {

            nextEvent = myEDCA[chan]->startContent(lastIdle, guardActive());
            nextMacEventTime.record(nextEvent);

            if (nextEvent != -1) {
                if ((!useSCH)
                        || (nextEvent <= nextChannelSwitch->getArrivalTime())) {

                    if (nextMacEvent->isScheduled()) {
                        cancelEvent(nextMacEvent);
                    }

                    scheduleAt(nextEvent, nextMacEvent);

                    DBG_MAC << "Updated nextMacEvent:"
                                   << nextMacEvent->getArrivalTime().raw()
                                   << std::endl;
                } else {
                    DBG_MAC
                                   << "Too little time in this interval. Will not schedule nextMacEvent"
                                   << std::endl;

                    //it is possible that this queue has an txop. we have to revoke it
                    myEDCA[activeChannel]->revokeTxOPs();
                    statsNumTooLittleTime++;
                }
            } else {
                cancelEvent(nextMacEvent);
            }
        }
        if (num == 1 && idleChannel == false
                && myEDCA[chan]->myQueues[ac].currentBackoff == 0
                && chan == activeChannel) {
            myEDCA[chan]->backoff(ac, this);
        }

        //ETSI uses DCCQueue and FlowControl
    } else {

        if (dccQueue.size() < NDL_queueLength) {
            dccQueue.push(thisMsg);
            if (timer_DCC_Toff->isScheduled()) {
                cancelEvent(timer_DCC_Toff);
            }
            //DccFlowControl
            scheduleAt(simTime(), timer_DCC_Toff);
        } else {
            statsDroppedPackets++;
            delete thisMsg;
            delete msg;
            thisMsg = nullptr;
            ASSERT(thisMsg == nullptr);
            ASSERT(dccQueue.size() == 2);
        }
    }
}

void Mac1609_4::handleLowerControl(cMessage* msg) {
    if (msg->getKind() == MacToPhyInterface::TX_OVER) {

        DBG_MAC << "Successfully transmitted a packet on " << lastAC
                       << std::endl;

        phy->setRadioState(Radio::RX);

        //message was sent
        //update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
        myEDCA[activeChannel]->postTransmit(lastAC, this);
        //channel just turned idle.
        //don't set the chan to idle. the PHY layer decides, not us.

        if (guardActive()) {
            throw cRuntimeError("We shouldnt have sent a packet in guard!");
        }
    } else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_BUSY) {
        channelBusy();
    } else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_IDLE) {
        channelIdle();
    } else if (msg->getKind() == Decider80211p::BITERROR
            || msg->getKind() == Decider80211p::COLLISION) {
        statsSNIRLostPackets++;
        DBG_MAC << "A packet was not received due to biterrors" << std::endl;

        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0 && testCaseNumber > 0.0) {
            std::cerr << "A packet was not received due to biterrors at "
                    << simTime() << std::endl;
        }

    } else if (msg->getKind() == Decider80211p::RECWHILESEND) {
        statsTXRXLostPackets++;
        DBG_MAC
                       << "A packet was not received because we were sending while receiving"
                       << std::endl;

        if (strcmp(getParentModule()->getParentModule()->getFullName(),
                "node[0]") == 0 && testCaseNumber > 0.0) {
            std::cerr
                    << "A packet was not received because we were sending while receiving at "
                    << simTime() << std::endl;
        }

    } else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
        DBG_MAC << "Phylayer said radio switching is done" << std::endl;

    } else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
        phy->setRadioState(Radio::RX);
        DBG_MAC << "Phylayer said packet was dropped" << std::endl;

    } else {
        DBG_MAC << "Invalid control message type (type=NOTHING) : name="
                       << msg->getName() << " modulesrc="
                       << msg->getSenderModule()->getFullPath() << "."
                       << std::endl;

        assert(false);
    }

    if (msg->getKind() == Decider80211p::COLLISION) {
        emit(sigCollision, true);
    }

    delete msg;
}

void Mac1609_4::setActiveChannel(t_channel state) {
    activeChannel = state;
    assert(state == type_CCH || (useSCH && state == type_SCH));
}

void Mac1609_4::finish() {
//clean up queues.

    for (std::map<t_channel, EDCA*>::iterator iter = myEDCA.begin();
            iter != myEDCA.end(); iter++) {
        statsNumInternalContention += iter->second->statsNumInternalContention;
        statsNumBackoff += iter->second->statsNumBackoff;
        statsSlotsBackoff += iter->second->statsSlotsBackoff;
        iter->second->cleanUp();
        delete iter->second;
    }

    myEDCA.clear();

//stats
    recordScalar("ReceivedUnicastPackets", statsReceivedPackets);
    recordScalar("ReceivedBroadcasts", statsReceivedBroadcasts);
    recordScalar("SentPackets", statsSentPackets);
    recordScalar("SNIRLostPackets", statsSNIRLostPackets);
    recordScalar("RXTXLostPackets", statsTXRXLostPackets);
    recordScalar("TotalLostPackets",
            statsSNIRLostPackets + statsTXRXLostPackets);
    recordScalar("DroppedPacketsInMac",
            statsDroppedPackets + statsLifeTimeDropPackets);
    recordScalar("TooLittleTime", statsNumTooLittleTime);
    recordScalar("TimesIntoBackoff", statsNumBackoff);
    recordScalar("SlotsBackoff", statsSlotsBackoff);
    recordScalar("NumInternalContention", statsNumInternalContention);
    recordScalar("totalBusyTime", statsTotalBusyTime.dbl());

    if (nextMacEvent->isScheduled()) {
        cancelAndDelete(nextMacEvent);
    } else {
        delete nextMacEvent;
    }

    if (nextChannelSwitch && nextChannelSwitch->isScheduled())
        cancelAndDelete(nextChannelSwitch);

    if (beaconWAVE) {

        unsubscribe(sigChannelBusy, listenerWAVE);
        delete listenerWAVE;

    } else if (beaconETSI) {

        listenerNewMethod->calculateChannelLoad(
                simTime() - lastTimeChannelLoadTimeUpCalculated,
                totalChannelLoadTime, busyTimeVec, cbpVec, *currentState, false,
                testCaseNumber);

        if (timer_NDL_timeDown->isScheduled()) {
            cancelAndDelete(timer_NDL_timeDown);
        } else {
            delete timer_NDL_timeDown;
        }

        if (timer_NDL_timeUp) {
            cancelAndDelete(timer_NDL_timeUp);
        } else {
            delete timer_NDL_timeUp;
        }

        if (timer_NDL_channelLoad->isScheduled()) {
            cancelAndDelete(timer_NDL_channelLoad);
        } else {
            delete timer_NDL_channelLoad;
        }

        if (timer_DCC_Toff->isScheduled()) {
            cancelAndDelete(timer_DCC_Toff);
        } else {
            delete timer_DCC_Toff;
        }

        unsubscribe(sigChannelBusy, listenerNewMethod);
        delete listenerNewMethod;

        recordScalar("TotalChannelLoadTime", totalChannelLoadTime.dbl());
        recordScalar("Count RELAXED state", countRelaxed);
        recordScalar("Count ACTIVE state", countActive);
        recordScalar("Count RESTRICTIVE state", countRestrictive);
        recordScalar("DroppedPacketsLifeTime", statsLifeTimeDropPackets);
    }

}

void Mac1609_4::attachSignal(Mac80211Pkt* mac, simtime_t startTime,
        double frequency, uint64_t datarate, double txPower_mW) {

    simtime_t duration = getFrameDuration(mac->getBitLength());

    Signal* s = createSignal(startTime, duration, txPower_mW, datarate,
            frequency);
    MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);

    mac->setControlInfo(cinfo);

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0 && testCaseNumber > 1.0 && beaconETSI) {
        std::cout << "attachSignal at " << simTime() << " currentState "
                << currentState->stateType << " datarate " << datarate
                << " frequency " << frequency << " txPowerInMW " << txPower_mW
                << std::endl;
    }

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0 && testCaseNumber > 2.0 && beaconWAVE) {
        std::cout << "attachSignal at " << simTime() << " datarate " << datarate
                << " frequency " << frequency << " txPowerInMW " << txPower_mW
                << std::endl;
    }

}

Signal* Mac1609_4::createSignal(simtime_t start, simtime_t length, double power,
        uint64_t bitrate, double frequency) {
    simtime_t end = start + length;
//create signal with start at current simtime and passed length
    Signal* s = new Signal(start, length);

//create and set tx power mapping
    ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end,
            frequency, 5.0e6, power);
    s->setTransmissionPower(txPowerMapping);

    Mapping* bitrateMapping = MappingUtils::createMapping(
            DimensionSet::timeDomain(), Mapping::STEPS);

    Argument pos(start);
    bitrateMapping->setValue(pos, bitrate);

    pos.setTime(phyHeaderLength / bitrate);
    bitrateMapping->setValue(pos, bitrate);

    s->setBitrate(bitrateMapping);

    return s;
}

/* checks if guard is active */
bool Mac1609_4::guardActive() const {
    if (!useSCH)
        return false;
    if (simTime().dbl() - nextChannelSwitch->getSendingTime()
            <= GUARD_INTERVAL_11P)
        return true;
    return false;
}

/* returns the time until the guard is over */
simtime_t Mac1609_4::timeLeftTillGuardOver() const {
    ASSERT(useSCH);
    simtime_t sTime = simTime();
    if (sTime - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P) {
        return GUARD_INTERVAL_11P
                - (sTime - nextChannelSwitch->getSendingTime());
    } else
        return 0;
}

/* returns the time left in this channel window */
simtime_t Mac1609_4::timeLeftInSlot() const {
    ASSERT(useSCH);
    return nextChannelSwitch->getArrivalTime() - simTime();
}

/* Will change the Service Channel on which the mac layer is listening and sending */
void Mac1609_4::changeServiceChannel(int cN) {
    ASSERT(useSCH);
    if (cN != Channels::SCH1 && cN != Channels::SCH2 && cN != Channels::SCH3
            && cN != Channels::SCH4) {
        throw cRuntimeError("This Service Channel doesnt exit: %d", cN);
    }

    mySCH = cN;

    if (activeChannel == type_SCH) {
        //change to new chan immediately if we are in a SCH slot,
        //otherwise it will switch to the new SCH upon next channel switch
        phy11p->changeListeningFrequency(frequency[mySCH]);
    }
}

void Mac1609_4::setTxPower(double txPower_mW) {

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0 && beaconETSI) {
        std::cerr << getParentModule()->getParentModule()->getFullName()
                << "setTXPower TXPower is set in mW " << txPower_mW
                << std::endl;
    }
    txPower = txPower_mW;

// save TXPower in WAVE Mode
    if (beaconWAVE) {
        txPowerInMWVec.record(txPower);
    }

}
void Mac1609_4::setMCS(enum PHY_MCS mcs) {
    ASSERT2(mcs != MCS_DEFAULT, "invalid MCS selected");
    bitrate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
    setParametersForBitrate(bitrate);
}

void Mac1609_4::setCCAThreshold(double ccaThreshold_dBm) {

    if (strcmp(getParentModule()->getParentModule()->getFullName(), "node[0]")
            == 0 && beaconETSI) {
        std::cerr << getParentModule()->getParentModule()->getFullName()
                << "setCCAThreshold cca is set in dBm " << ccaThreshold_dBm
                << std::endl;
    }
    phy11p->setCCAThreshold(ccaThreshold_dBm);

    if (beaconWAVE) {
        carrierSenseVec.record(ccaThreshold_dBm);
    }
}

void Mac1609_4::handleLowerMsg(cMessage* msg) {
    Mac80211Pkt* macPkt = static_cast<Mac80211Pkt*>(msg);
    ASSERT(macPkt);

    WaveShortMessage* wsm =
            dynamic_cast<WaveShortMessage*>(macPkt->decapsulate());

//pass information about received frame to the upper layers
    DeciderResult80211 *macRes =
            dynamic_cast<DeciderResult80211 *>(PhyToMacControlInfo::getDeciderResult(
                    msg));
    ASSERT(macRes);
    DeciderResult80211 *res = new DeciderResult80211(*macRes);
    wsm->setControlInfo(new PhyToMacControlInfo(res));

    long dest = macPkt->getDestAddr();

    DBG_MAC << "Received frame name= " << macPkt->getName() << ", myState="
                   << " src=" << macPkt->getSrcAddr() << " dst="
                   << macPkt->getDestAddr() << " myAddr=" << myMacAddress
                   << std::endl;

    if (macPkt->getDestAddr() == myMacAddress) {
        DBG_MAC << "Received a data packet addressed to me." << std::endl;
        statsReceivedPackets++;
        sendUp(wsm);
    } else if (dest == LAddress::L2BROADCAST()) {

        if (std::string(wsm->getName()).find("beacon") != std::string::npos) {
            statsReceivedBroadcasts++;
        }

        sendUp(wsm);

    } else {
        DBG_MAC << "Packet not for me, deleting..." << std::endl;
        delete wsm;
    }
    delete macPkt;
}

int Mac1609_4::EDCA::queuePacket(t_access_category ac, WaveShortMessage* msg) {

    if (maxQueueSize && myQueues[ac].queue.size() >= maxQueueSize) {
        delete msg;
        return -1;
    }
    myQueues[ac].queue.push(msg);
    return myQueues[ac].queue.size();
}

int Mac1609_4::EDCA::createQueue(int aifsn, int cwMin, int cwMax,
        t_access_category ac) {

    if (myQueues.find(ac) != myQueues.end()) {
        throw cRuntimeError(
                "You can only add one queue per Access Category per EDCA subsystem");
    }

    EDCAQueue newQueue(aifsn, cwMin, cwMax, ac);
    myQueues[ac] = newQueue;

    return ++numQueues;
}

Mac1609_4::t_access_category Mac1609_4::mapPriority(int prio) {
//dummy mapping function
    switch (prio) {
    case 0:
        return AC_BK;
    case 1:
        return AC_BE;
    case 2:
        return AC_VI;
    case 3:
        return AC_VO;
    default:
        throw cRuntimeError("MacLayer received a packet with unknown priority");
        break;
    }
    return AC_VO;
}

WaveShortMessage* Mac1609_4::EDCA::initiateTransmit(simtime_t lastIdle,
        bool beaconETSI) {

//iterate through the queues to return the packet we want to send
    WaveShortMessage* pktToSend = NULL;

    simtime_t idleTime = simTime() - lastIdle;

    DBG_MAC << "Initiating transmit at " << simTime()
                   << ". I've been idle since " << idleTime << std::endl;

    for (std::map<t_access_category, EDCAQueue>::iterator iter =
            myQueues.begin(); iter != myQueues.end(); iter++) {
        //It's always the most uptodate CAM in the Queue

        if (iter->second.queue.size() != 0) {
            if (idleTime >= iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P
                    && iter->second.txOP == true) {

                DBG_MAC << "Queue " << iter->first << " is ready to send!"
                               << std::endl;

                iter->second.txOP = false;
                //this queue is ready to send
                if (pktToSend == NULL) {
                    pktToSend = iter->second.queue.front();
                } else {
                    //there was already another packet ready. we have to go increase cw and go into backoff. It's called internal contention and its wonderful

                    statsNumInternalContention++;
                    iter->second.cwCur = std::min(iter->second.cwMax,
                            iter->second.cwCur * 2);
                    iter->second.currentBackoff = /*OWNER*/owner->intuniform(0,
                            iter->second.cwCur);
                    DBG_MAC << "Internal contention for queue " << iter->first
                                   << " : " << iter->second.currentBackoff
                                   << ". Increase cwCur to "
                                   << iter->second.cwCur << std::endl;
                }
            }
        }
    }

    if (pktToSend == NULL) {
        throw cRuntimeError("No packet was ready");
    }
    return pktToSend;
}

simtime_t Mac1609_4::EDCA::startContent(simtime_t idleSince, bool guardActive) {

    ASSERT(!guardActive); // no SCH in use, is alway false

    DBG_MAC << "Restarting contention." << std::endl;

    simtime_t nextEvent = -1;

    simtime_t idleTime = SimTime().setRaw(
            std::max((int64_t) 0, (simTime() - idleSince).raw()));

    lastStart = idleSince;

    DBG_MAC << "Channel is already idle for:" << idleTime << " since "
                   << idleSince << std::endl;

//this returns the nearest possible event in this EDCA subsystem after a busy channel

    for (std::map<t_access_category, EDCAQueue>::iterator iter =
            myQueues.begin(); iter != myQueues.end(); iter++) {
        if (iter->second.queue.size() != 0) {

            /* 1609_4 says that when attempting to send (backoff == 0) when guard is active, a random backoff is invoked */

            //not used, guardActive always false, no SCH in use
            if (guardActive == true && iter->second.currentBackoff == 0) {

                ASSERT(false);
                //cw is not increased
                iter->second.currentBackoff = /*OWNER*/owner->intuniform(0,
                        iter->second.cwCur);
                statsNumBackoff++;
            }

            simtime_t DIFS = iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P;

            //the next possible time to send can be in the past if the channel was idle for a long time, meaning we COULD have sent earlier if we had a packet
            simtime_t possibleNextEvent = DIFS
                    + iter->second.currentBackoff * SLOTLENGTH_11P;

            DBG_MAC << "START CONTENT Waiting Time for Queue " << iter->first
                           << ":" << possibleNextEvent << "="
                           << iter->second.aifsn << " * " << SLOTLENGTH_11P
                           << " + " << SIFS_11P << "+"
                           << iter->second.currentBackoff << "*"
                           << SLOTLENGTH_11P << "; Idle time: " << idleTime
                           << std::endl;

            if (idleTime > possibleNextEvent) {
                DBG_MAC << "Could have already send if we had it earlier"
                               << std::endl;

                //we could have already sent. round up to next boundary
                simtime_t base = idleSince + DIFS;
                possibleNextEvent = simTime()
                        - simtime_t().setRaw(
                                (simTime() - base).raw() % SLOTLENGTH_11P.raw())
                        + SLOTLENGTH_11P;
            } else {
                //we are gonna send in the future
                DBG_MAC << "Sending in the future" << std::endl;

                possibleNextEvent = idleSince + possibleNextEvent;
            }
            nextEvent == -1 ? nextEvent = possibleNextEvent : nextEvent =
                                      std::min(nextEvent, possibleNextEvent);
        }
    }
    return nextEvent;
}

void Mac1609_4::EDCA::stopContent(bool allowBackoff, bool generateTxOp,
        omnetpp::cModule * source) {
//update all Queues

    DBG_MAC << "Stopping Contention at " << simTime().raw() << std::endl;

    simtime_t passedTime = simTime() - lastStart;

    DBG_MAC << "Channel was idle for " << passedTime << std::endl;

    lastStart = -1; //indicate that there was no last start

    for (std::map<t_access_category, EDCAQueue>::iterator iter =
            myQueues.begin(); iter != myQueues.end(); iter++) {
        if (iter->second.currentBackoff != 0
                || iter->second.queue.size() != 0) {
            //check how many slots we already waited until the chan became busy

            int oldBackoff = iter->second.currentBackoff;

            std::string info;
            if (passedTime < iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P) {
                //we didnt even make it one DIFS :(
                info.append(" No DIFS");
            } else {
                //decrease the backoff by one because we made it longer than one DIFS
                iter->second.currentBackoff--;

                //check how many slots we waited after the first DIFS
                int passedSlots = (int) ((passedTime
                        - SimTime(
                                iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P))
                        / SLOTLENGTH_11P);

                DBG_MAC << "Passed slots after DIFS: " << passedSlots
                               << std::endl;

                if (iter->second.queue.size() == 0) {
                    //this can be below 0 because of post transmit backoff -> backoff on empty queues will not generate macevents,
                    //we dont want to generate a txOP for empty queues
                    iter->second.currentBackoff -= std::min(
                            iter->second.currentBackoff, passedSlots);
                    info.append(" PostCommit Over");
                } else {
                    iter->second.currentBackoff -= passedSlots;
                    if (iter->second.currentBackoff <= -1) {
                        if (generateTxOp) {
                            iter->second.txOP = true;
                            info.append(" TXOP");
                        }
                        //else: this packet couldnt be sent because there was too little time. we could have generated a txop, but the channel switched
                        iter->second.currentBackoff = 0;
                    }

                }
            }
            DBG_MAC << "Updating backoff for Queue " << iter->first << ": "
                           << oldBackoff << " -> "
                           << iter->second.currentBackoff << info << std::endl;
        }
    }
}
void Mac1609_4::EDCA::backoff(t_access_category ac, cModule * source) {
    myQueues[ac].currentBackoff = /*OWNER*/owner->intuniform(0,
            myQueues[ac].cwCur);
    statsSlotsBackoff += myQueues[ac].currentBackoff;

    statsNumBackoff++;
    DBG_MAC
                   << "Going into Backoff because channel was busy when new packet arrived from upperLayer"
                   << std::endl;
    if (strcmp(source->getParentModule()->getParentModule()->getFullName(),
            "node[0]") == 0) {
        std::cerr << "BACKOFF at " << simTime()
                << " Going into Backoff because channel was busy when new packet arrived from upperLayer --> TimesIntoBackoff increases and SlotsBackoff added"
                << std::endl;
    }
}

void Mac1609_4::EDCA::postTransmit(t_access_category ac, cModule * source) {
    delete myQueues[ac].queue.front();
    myQueues[ac].queue.pop();
    myQueues[ac].cwCur = myQueues[ac].cwMin;
//post transmit backoff
    myQueues[ac].currentBackoff = /*OWNER*/owner->intuniform(0,
            myQueues[ac].cwCur);

    statsSlotsBackoff += myQueues[ac].currentBackoff;
    statsNumBackoff++;
    DBG_MAC << "Queue " << ac << " will go into post-transmit backoff for "
                   << myQueues[ac].currentBackoff << " slots" << std::endl;
    if (strcmp(source->getParentModule()->getParentModule()->getFullName(),
            "node[0]") == 0) {
        std::cerr << "POSTTRANSMIT at " << simTime() << " Queue " << ac
                << " will go into post-transmit backoff for "
                << myQueues[ac].currentBackoff
                << " slots; TimesIntoBackoff increases and SlotsBackoff added"
                << std::endl;
    }
}

void Mac1609_4::EDCA::cleanUp() {
    for (std::map<t_access_category, EDCAQueue>::iterator iter =
            myQueues.begin(); iter != myQueues.end(); iter++) {
        while (iter->second.queue.size() != 0) {
            delete iter->second.queue.front();
            iter->second.queue.pop();
        }
    }
    myQueues.clear();
}

void Mac1609_4::EDCA::revokeTxOPs() {
    for (std::map<t_access_category, EDCAQueue>::iterator iter =
            myQueues.begin(); iter != myQueues.end(); iter++) {
        if (iter->second.txOP == true) {
            iter->second.txOP = false;
            iter->second.currentBackoff = 0;
        }
    }
}

void Mac1609_4::channelBusySelf(bool generateTxOp) {

//the channel turned busy because we're sending. we don't want our queues to go into backoff
//internal contention is already handled in initiateTransmission

    if (!idleChannel)
        return;
    idleChannel = false;
    DBG_MAC << "Channel turned busy: Switch or Self-Send" << std::endl;

    lastBusy = simTime();

//channel turned busy
    if (nextMacEvent->isScheduled() == true) {
        cancelEvent(nextMacEvent);
    } else {
        //the edca subsystem was not doing anything anyway.
    }
    myEDCA[activeChannel]->stopContent(false, generateTxOp, this);

    emit(sigChannelBusy, true);
}

void Mac1609_4::channelBusy() {

    if (!idleChannel)
        return;

//the channel turned busy because someone else is sending
    idleChannel = false;
    DBG_MAC << "Channel turned busy: External sender" << std::endl;
    lastBusy = simTime();

//channel turned busy
    if (nextMacEvent->isScheduled() == true) {
        cancelEvent(nextMacEvent);
    } else {
        //the edca subsystem was not doing anything anyway.
    }
    myEDCA[activeChannel]->stopContent(true, false, this);

    emit(sigChannelBusy, true);
}

void Mac1609_4::channelIdle(bool afterSwitch) {

    DBG_MAC << "Channel turned idle: Switch: " << afterSwitch << std::endl;

    if (nextMacEvent->isScheduled() == true) {
        //this rare case can happen when another node's time has such a big offset that the node sent a packet although we already changed the channel
        //the workaround is not trivial and requires a lot of changes to the phy and decider
        return;
        //throw cRuntimeError("channel turned idle but contention timer was scheduled!");
    }

    idleChannel = true;

    simtime_t delay = 0;

//account for 1609.4 guards
    if (afterSwitch) {
        //	delay = GUARD_INTERVAL_11P;
    }
    if (useSCH) {
        delay += timeLeftTillGuardOver();
    }

//channel turned idle! lets start contention!
    lastIdle = delay + simTime();
    statsTotalBusyTime += simTime() - lastBusy;

//get next Event from current EDCA subsystem
    simtime_t nextEvent = myEDCA[activeChannel]->startContent(lastIdle,
            guardActive());
    nextMacEventTime.record(nextEvent);

    if (nextEvent != -1) {
        if ((!useSCH) || (nextEvent < nextChannelSwitch->getArrivalTime())) {

            scheduleAt(nextEvent, nextMacEvent);

            DBG_MAC << "next Event is at "
                           << nextMacEvent->getArrivalTime().raw() << std::endl;
        } else {
            DBG_MAC
                           << "Too little time in this interval. will not schedule macEvent"
                           << std::endl;
            statsNumTooLittleTime++;
            myEDCA[activeChannel]->revokeTxOPs();
        }
    } else {
        DBG_MAC << "I don't have any new events in this EDCA sub system"
                       << std::endl;
    }

    emit(sigChannelBusy, false);

}

double Mac1609_4::getTxPower() {
    return txPower;
}

uint64_t Mac1609_4::getBitrate() {
    return bitrate;
}

void Mac1609_4::setParametersForBitrate(uint64_t bitrate) {
    for (unsigned int i = 0; i < NUM_BITRATES_80211P; i++) {
        if (bitrate == BITRATES_80211P[i]) {
            n_dbps = N_DBPS_80211P[i];
            return;
        }
    }
    throw cRuntimeError(
            "Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
}

simtime_t Mac1609_4::getFrameDuration(int payloadLengthBits,
        enum PHY_MCS mcs) const {
    simtime_t duration;
    if (mcs == MCS_DEFAULT) {
// calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION
                + T_SYM_80211P * ceil((16 + payloadLengthBits + 6) / (n_dbps));
    } else {
        uint32_t ndbps = getNDBPS(mcs);
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION
                + T_SYM_80211P * ceil((16 + payloadLengthBits + 6) / (ndbps));
    }

    return duration;
}

