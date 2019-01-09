//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

const simsignalwrap_t BaseWaveApplLayer::mobilityStateChangedSignal =
        simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void BaseWaveApplLayer::initialize(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(
                getParentModule());
        assert(myMac);
        startTime = simTime();

        myId = getParentModule()->getIndex();

        headerLength = par("headerLength").longValue();
        double maxOffset = par("maxOffset").doubleValue();
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        beaconETSI = getParentModule()->par("beaconETSI").boolValue();
        beaconWAVE = getParentModule()->par("beaconWAVE").boolValue();
        testCaseNumber = getParentModule()->par("testCaseNumber").doubleValue();
        defaultBeaconInterval = par("beaconInterval").doubleValue();
        speedVec.setName("Average Speed in kmh");

        if (beaconETSI && beaconWAVE) {

            throw cRuntimeError(
                    "Not possible to use WAVE AND ETSI-Mode. Set one flag to false in omnetpp.ini (beaconETSI or beaconWAVE).");
        }

        //WAVE Specific parameters, only necessery in WAVE Mode
        if (beaconWAVE) {
            //params taken from SAE J2945
            initAllWAVEParameters();
            //

        } else if (beaconETSI) {

            //necessary for CAM
            beaconPriority = par("beaconPriorityAC_BE").doubleValue();
            std::cout << "ETSI mode priority " << beaconPriority << std::endl;

            counterSentBeacon = 0;
            counterReceivedBeacon = 0;
            //T_GenCam is the current BeaconInterval
            T_GenCam = T_GenCamMax;
            //T_GenCam_DCC = defaultBeaconInterval;
            T_CheckCamGen = T_GenCamMin;
            lastSentCAM = nullptr;
            lastSentCAMMove = nullptr;
            counterNumGenCam = 1;
            lastTxTime = 0;

            startBeaconVec.setName("startBeaconingTime");
            nextBeaconScheduleTimeVec.setName("currentBeaconIntervalVec");
            etsiCAMConditionVector.setName("CAMConditions, 1 or 2");
        }

        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();
        sendDataInterval = par("sendDataInterval").doubleValue();
        counterReceivedData = 0;

        if (beaconETSI) {
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_CAM);
            checkCAMParams = new cMessage("checkCAMParams", CHECK_CAM_PARAMS);
        } else if (beaconWAVE) {
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_BSM);
        } else {
            sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        }

        //used in WAVE and ETSI mode
        individualOffset = dblrand() * maxOffset;

        findHost()->subscribe(mobilityStateChangedSignal, this);

        if (sendBeacons) {

            if (beaconWAVE) {

                simtime_t startBeaconingTime = simTime()
                        + uniform(maxOffset, defaultBeaconInterval - maxOffset);

                startBeaconVec.record(startBeaconingTime);

                scheduleAt(startBeaconingTime, sendBeaconEvt);

                scheduleAt(startBeaconingTime + vCBPMeasInt,
                        timerMSG_CBPMeasureIntervalWAVE);

                //Calc vehicles first time after 1s or 5s
                if (calcVeh) {

                    scheduleAt(startBeaconingTime + vPERSubInterval,
                            timerMSG_PERCalculationSubIntervalWAVE);
                } else {
                    scheduleAt(startBeaconingTime + vPERInterval,
                            timerMSG_PERCalculationIntervalWAVE);
                }
                //

                scheduleAt(startBeaconingTime + vTxCntrlInt,
                        timerMSG_TXRateCntrlIntervalWAVE);
                if (randomCriticalEvent) {
                    scheduleAt(startBeaconingTime + 6.01,
                            timerMSG_RandomCriticalEvent);
                }
            } else if (beaconETSI) {

                simtime_t startBeaconingTime = simTime()
                        + uniform(maxOffset, defaultBeaconInterval - maxOffset);

                startBeaconVec.record(startBeaconingTime);

                //necessary to send at least one cam
                scheduleAt(startBeaconingTime, sendBeaconEvt);
                //the cam trigger
                scheduleAt(startBeaconingTime + T_CheckCamGen, checkCAMParams);
            }
        }

        if (strcmp(getParentModule()->getFullName(), "node[0]") == 0) {
            std::cerr << "BeaconLengthBits " << beaconLengthBits << " Priority "
                    << beaconPriority << std::endl;
        }

        if (sendData) {

            sendDataEvt = new cMessage("data", SEND_DATA);

            if (strcmp(getParentModule()->getFullName(), "node[0]") == 0) {
                std::cerr << "Send Data ON " << std::endl;
            }

            //broadcast data first time after at least one second
            scheduleAt(simTime() + 1, sendDataEvt);
        } else {

            sendDataEvt = nullptr;

            if (strcmp(getParentModule()->getFullName(), "node[0]") == 0) {
                std::cerr << "Send Data OFF " << std::endl;
            }
        }
    }

}

void BaseWaveApplLayer::performCAMGenerating() {

    simtime_t timeElapsed = simTime() - lastTxTime;
    ASSERT(T_GenCam_DCC >= T_GenCamMin && T_GenCam_DCC <= T_GenCamMax);

    //at least one cam was generated
    if (lastSentCAM != nullptr && lastSentCAMMove != nullptr) {

        if (timeElapsed >= T_GenCam_DCC) {

            //only called in TestMode
            if (testCaseNumber > 0) {
                CAMGenerationHelper::performTests();
            }

            double difSpeed = CAMGenerationHelper::getSpeedDifference(curMove,
                    lastSentCAMMove);
            double difDirection = CAMGenerationHelper::getDirectionDifAngle(
                    lastSentCAMMove->getDirection(), curMove.getDirection());
            double difPosition = CAMGenerationHelper::getPositionDif(
                    curPosition, lastSentCAM->getSenderPos());

            if (difSpeed > 0.5 || difPosition > 4 || difDirection > 4) {

                T_GenCam = timeElapsed;
                counterNumGenCam = 1;

                if (sendBeaconEvt->isScheduled()) {
                    cancelEvent(sendBeaconEvt);
                }
                scheduleAt(simTime(), sendBeaconEvt);

                if (simTime() >= startTime + 6)
                    etsiCAMConditionVector.record(1);

                //Condition Two
            } else if (timeElapsed >= T_GenCam_DCC && timeElapsed >= T_GenCam) {

                counterNumGenCam++;

                if (counterNumGenCam == numGenCam) {

                    T_GenCam = T_GenCamMax;
                    counterNumGenCam = 0;

                }
                //condition two appears, send CAM immediately
                if (sendBeaconEvt->isScheduled()) {
                    cancelEvent(sendBeaconEvt);
                }
                scheduleAt(simTime(), sendBeaconEvt);

                if (simTime() >= startTime + 6)
                    etsiCAMConditionVector.record(2);
            }
        }
    }
}

void BaseWaveApplLayer::handleSelfMsg(cMessage* msg) {

    switch (msg->getKind()) {
    case CHECK_CAM_PARAMS: {

        performCAMGenerating();

        scheduleAt(simTime() + T_CheckCamGen, msg);

        break;
    }

    case SEND_BEACON_CAM: {

        if (simTime() >= startTime + 6)
            nextBeaconScheduleTimeVec.record(simTime() - lastTxTime);

        sendBeaconCAM();

//        if (simTime() >= startTime + 6)
//            nextBeaconScheduleTimeVec.record(T_GenCam);

//        scheduleAt(simTime() + T_GenCam, msg);

        break;
    }
        //used in WAVE Mode only --> only used when in omnetpp.ini randomCriticalFlag is set to true
        // simulates a criticalEvent every 6 seconds
    case RANDOM_CRITICAL_EVENT: {

        performTransmissionDecision(true);

        performScheduleTransmission();

        if (simTime() >= startTime + 6)
            criticalEventVec.record(simTime());

        scheduleAt(simTime() + 6.01, msg);

        break;
    }

        // send a BSM Beacon
    case SEND_BEACON_BSM: {

        //CURRENTTIME == NextScheduledTime!!!

        //RADIATED POWER 6.3.8.7, SAE J2945, Eq. 8 - 11
        calculateRadiatedPower();

        //generate new BSM, save the sent BSM local as lastHVStatus, set lastTxTime and lastMove
        //see A.8.1
        assumptionOfLatestHVStatusAndSendBSM();
        //

        // calculate currentBeaconInterval
        calculateNextScheduleTimeAndScheduleNextBeacon(msg);

        break;
    }
        //calculate the CBP with the interval CBP_MEASURE_INTERVAL_WAVE
    case CBP_MEASURE_INTERVAL_WAVE: {

        performCBPMeasureInterval();

        //reschedule this event
        scheduleAt(simTime() + vCBPMeasInt, msg);

        break;
    }
    case PER_CALCULATION_INTERVAL_WAVE: {

        //calculates PER, ChannelQualityIndicator and VehicleDensityRange
        calculateCompletePERValuesWAVE();

        //calculate PER every second
        //schedule PER Calclation at every following SubInterval
        scheduleAt(simTime() + vPERSubInterval,
                timerMSG_PERCalculationSubIntervalWAVE);

        break;
    }
    case PER_CALCULATION_SUBINTERVAL_WAVE: {

        //calculates PER, ChannelQualityIndicator and VehicleDensityRange
        calculateCompletePERValuesWAVE();

        //reschedule this event
        scheduleAt(simTime() + vPERSubInterval, msg);

        break;
    }
    case TX_RATE_CNTRL_INTERVAL_WAVE: {

        //TrackingError e(k)
        calculateTrackingError();

        //calculate transmission probability p(k)
        calculateTransmissionProbability();

        //calculate MaxITT
        calculateMaximumInterTransmitTime();

        //calculate Transmission decision and schedule decision
        performTransmissionDecision(false);

        performScheduleTransmission();
        //

        scheduleAt(simTime() + vTxCntrlInt, msg);

        break;
    }
        // only in use, when neither ETSI-Mode nor WAVE-Mode is chosen
    case SEND_BEACON_EVT: {

        WaveShortMessage * temp = prepareWSM("beacon", beaconLengthBits,
                type_CCH, beaconPriority, 0, -1);
        sendWSM(temp);

        double offSet = dblrand() * (defaultBeaconInterval.dbl() / 2);
        offSet = offSet + floor(offSet / 0.050) * 0.050;
        scheduleAt(simTime() + offSet, msg);

        break;
    }
        // used when in omnetpp.ini sendData Flag is true, sendDataInterval is also set in omnetpp.ini
    case SEND_DATA: {

        WaveShortMessage * temp = prepareData("data", beaconLengthBits,
                type_CCH, dataPriority, 0, -1);
        sendWSM(temp);

        scheduleAt(simTime() + sendDataInterval, msg);

        break;
    }
    default: {
        if (msg)
            DBG << "APP: Error: Got Self Message of unknown kind! Name: "
                       << msg->getName() << endl;
        break;
    }
    }
}

WaveShortMessage* BaseWaveApplLayer::prepareData(std::string name,
        int lengthBits, t_channel channel, int priority, int rcvId,
        int serial) {

    WaveShortMessage* wsm;
    std::string beaconName;

    beaconName = "Data from " + std::string(getParentModule()->getFullName());
    wsm = new WaveShortMessage(beaconName.c_str());
    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);

    switch (channel) {
    case type_SCH:
        wsm->setChannelNumber(Channels::SCH1);
        break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
    case type_CCH:
        if (beaconWAVE) {
            wsm->setChannelNumber(Channels::CCH);
        } else if (beaconETSI) {
            wsm->setChannelNumber(Channels::CCH_ETSI);
        } else {
            wsm->setChannelNumber(Channels::CCH);
        }
        break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);
    wsm->setKind(SEND_DATA);

    return wsm;
}

WaveShortMessage* BaseWaveApplLayer::prepareWSM(std::string name,
        int lengthBits, t_channel channel, int priority, int rcvId,
        int serial) {

    WaveShortMessage* wsm;
    std::string beaconName;

    if (beaconWAVE) {
        beaconName = "BSM beacon from "
                + std::string(getParentModule()->getFullName());
        wsm = new BasicSafetyMessage(beaconName.c_str());
        BasicSafetyMessage *temp = dynamic_cast<BasicSafetyMessage*>(wsm);
        msgCount++;
        temp->setMSG_COUNT(msgCount);
        temp->setCarId(getParentModule()->getFullName());
        temp->setTimeOfPositionUpdate(timePositionChangedHVLocalEstimate);
        if (testCaseNumber == 8)
            temp->setELEVATION(123456);
        wsm = dynamic_cast<WaveShortMessage*>(temp);
        wsm->setTxPower(FWMath::dBm2mW(rp));

    } else if (beaconETSI) {

        Mac1609_4* macLayer = FindModule<Mac1609_4*>::findSubModule(
                getParentModule());
        beaconName = "CAM beacon from "
                + std::string(getParentModule()->getFullName());
        wsm = new CooperativeAwarenessMessage(beaconName.c_str());
        CooperativeAwarenessMessage * temp =
                dynamic_cast<CooperativeAwarenessMessage*>(wsm);
        temp->setCarId(getParentModule()->getFullName());
        temp->setTimeOfPositionUpdate(timePositionChangedHVLocalEstimate);

        //see 102 687, V1.2
        temp->setCBR_R_0_Hop(
                macLayer->getChannelLoadListenerETSI()->getLastCbrValues().CBR_L_0_Hop);

        //Disseminated 1-hop channel busy ratio (CBR_L_1_Hop), i.e. CBR_L_1_Hop becomes CBR_R_1_Hop when disseminated. At receiving ITS-S it becomes CBR_L_2_Hop.
        temp->setCBR_R_1_Hop(
                macLayer->getChannelLoadListenerETSI()->getLastCbrValues().CBR_L_1_Hop);

        //

        if (testCaseNumber == 8)
            temp->setItsPDUHeader("test");

        wsm = dynamic_cast<WaveShortMessage*>(temp);
//        wsm->setTxPower(macLayer->getTxPower());
//        wsm->setBitrate(macLayer->getBitrate());

    } else {

        beaconName = "beacon from "
                + std::string(getParentModule()->getFullName());
        wsm = new WaveShortMessage(beaconName.c_str());

    }

    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);

    switch (channel) {
    case type_SCH:
        wsm->setChannelNumber(Channels::SCH1);
        break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
    case type_CCH:
        if (beaconWAVE) {
            wsm->setChannelNumber(Channels::CCH);
        } else if (beaconETSI) {
            wsm->setChannelNumber(Channels::CCH_ETSI);
        } else {
            wsm->setChannelNumber(Channels::CCH);
        }
        break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (beaconETSI) {

        wsm->setKind(SEND_BEACON_CAM);

    } else if (beaconWAVE) {

        wsm->setKind(SEND_BEACON_BSM);
    }

    return wsm;
}

void BaseWaveApplLayer::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

void BaseWaveApplLayer::handlePositionUpdate(cObject* obj) {
    ChannelMobilityPtrType const mobility = check_and_cast<
            ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
    timePositionChangedHVLocalEstimate = simTime();
    //last move
    curMove = mobility->getMove();
    if (simTime() >= startTime + 6)
        speedVec.record((curMove.getSpeed() * 60 * 60) / 1000);
    //
}

void BaseWaveApplLayer::handleLowerMsg(cMessage* msg) {

    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()).find("beacon") != std::string::npos) {
        onBeacon(wsm);
    } else if (std::string(wsm->getName()).find("Data") != std::string::npos) {
        onData(wsm);
    } else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete (msg);
}

void BaseWaveApplLayer::calculateNextScheduleTimeAndScheduleNextBeacon(
        cMessage* msg) {
    //
    //calculate currentBeaconInterval
    double rand = uniform(-vTxRand, vTxRand);

    simtime_t nextBeaconScheduleTime = lastTxTime + maximumInterTransmitTime
            + rand;

    if (simTime() >= startTime + 6)
        nextBeaconScheduleTimeVec.record(nextBeaconScheduleTime - simTime());

    //
    // reschedule this event
    if (nextBeaconScheduleTime > (simTime())) {
        counterNextScheduleTime++;
        scheduleAt(nextBeaconScheduleTime, msg);
    } else {
        counterUseBeaconInterval++;
        scheduleAt(simTime() + defaultBeaconInterval, msg);
    }
}

void BaseWaveApplLayer::calculateCompletePERValuesWAVE() {

    if (calcVeh) {
        if (simTime() - startTime >= vPERInterval) {
            perCalculator->calculatePER(curPosition, simTime(), perVec,
                    testCaseNumber, startTime);
            channelQualityIndicator =
                    perCalculator->calculateChannelQualityIndicator();
            if (simTime() >= startTime + 6)
                channelQualityIndicatorVec.record(channelQualityIndicator);
        }
    } else {
        //first time to calculate PER
        perCalculator->calculatePER(curPosition, simTime(), perVec,
                testCaseNumber, startTime);

        channelQualityIndicator =
                perCalculator->calculateChannelQualityIndicator();
        if (simTime() >= startTime + 6)
            channelQualityIndicatorVec.record(channelQualityIndicator);
    }

    vehicleDensityInRange = perCalculator->calculateVehicleDensityInRange(
            curPosition, testCaseNumber);
    if (simTime() >= startTime + 6)
        vehicleDensityInRangeVec.record(vehicleDensityInRange);

}

void BaseWaveApplLayer::updateTxFailed() {
    //transmission is done, update txFailed --> see in SAE J2945, A.8.2

    double rand = bernoulli(0.5);

    if (rand < channelQualityIndicator) {

        txFailed += 1;

    } else {

        txFailed = 0;

    }
}

void BaseWaveApplLayer::assumptionOfLatestHVStatusAndSendBSM() {

    //generate new BSM, save the sent BSM local as lastHVStatus, set lastTxTime and lastMove
    //see A.8.1

    if (lastHVStatus == nullptr) {

        //generate and send first beacon, also save the sent beacon as lastHVState --> flag true
        sendBeaconBSM(true);

    } else {

        //pre-tx --> lastHVStatus Beacon
        //latest --> the currently generated beacon that was sent

        if (txFailed > 0 && txFailed <= vMaxSuccessiveFail) {

            // --> in this case we assume, that the straight away generated beacon not perceive with RVs, i.e.,
            // we don't save the beacon as lastHVStatus!
            sendBeaconBSM(false);

            //counter for statistics
            counterLastHVStatusNotChanged += 1;

        } else {

            //set flag zero
            txFailed = 0;

            //--> latestStatus is currently generated beacon --> flag true

            sendBeaconBSM(true);

        }
    }
}

void BaseWaveApplLayer::sendBeaconBSM(bool previousBeaconFlag) {

    WaveShortMessage* temp = prepareWSM("beacon", beaconLengthBits, type_CCH,
            beaconPriority, 0, -1);

    // Save the last sent beacon for calculation of e(k)
    // Save the last HVStatusInformation which was sent
    if (previousBeaconFlag) {

        if (lastHVStatus != nullptr) {
            cancelAndDelete(lastHVStatus);
        }
        lastHVStatus = new BasicSafetyMessage(temp->getName());
        lastHVStatus->setPsid(temp->getPsid());
        lastHVStatus->setPriority(temp->getPriority());
        lastHVStatus->setWsmVersion(temp->getWsmVersion());
        lastHVStatus->setChannelNumber(temp->getChannelNumber());
        lastHVStatus->setTimestamp(temp->getTimestamp());
        lastHVStatus->setSenderAddress(temp->getSenderAddress());
        lastHVStatus->setRecipientAddress(temp->getRecipientAddress());
        lastHVStatus->setSenderPos(temp->getSenderPos());
        lastHVStatus->setSerial(temp->getSerial());
        lastHVStatus->setMSG_COUNT(
                dynamic_cast<BasicSafetyMessage*>(temp)->getMSG_COUNT());
        lastHVStatus->setCarId(getParentModule()->getFullName());
        lastHVStatus->setTimeOfPositionUpdate(
                dynamic_cast<BasicSafetyMessage*>(temp)->getTimeOfPositionUpdate());

        //save last move-Object
        if (lastMoveInHVStatus != nullptr) {
            delete lastMoveInHVStatus;
        }
        lastMoveInHVStatus = new Move(curMove);
    }

    // send the beacon
    sendWSM(temp);

    //set lastTxTime to currentTime
    lastTxTime = simTime();

    //transmission is done, update txFailed --> see in SAE J2945, A.8.2
    updateTxFailed();

}

void BaseWaveApplLayer::sendBeaconCAM() {

    WaveShortMessage* temp = prepareWSM("beacon", beaconLengthBits, type_CCH,
            beaconPriority, 0, -1);

    // Save the last sent beacon for calculation of e(k)

    if (lastSentCAM != nullptr) {
        cancelAndDelete(lastSentCAM);
    }
    lastSentCAM = new CooperativeAwarenessMessage(temp->getName());
    lastSentCAM->setPsid(temp->getPsid());
    lastSentCAM->setPriority(temp->getPriority());
    lastSentCAM->setWsmVersion(temp->getWsmVersion());
    lastSentCAM->setChannelNumber(temp->getChannelNumber());
    lastSentCAM->setTimestamp(temp->getTimestamp());
    lastSentCAM->setSenderAddress(temp->getSenderAddress());
    lastSentCAM->setRecipientAddress(temp->getRecipientAddress());
    lastSentCAM->setSenderPos(temp->getSenderPos());
    lastSentCAM->setSerial(temp->getSerial());
    lastSentCAM->setCarId(getParentModule()->getFullName());
    lastSentCAM->setTimeOfPositionUpdate(
            dynamic_cast<CooperativeAwarenessMessage*>(temp)->getTimeOfPositionUpdate());
    lastSentCAM->setCBR_R_0_Hop(
            dynamic_cast<CooperativeAwarenessMessage*>(temp)->getCBR_R_0_Hop());
    lastSentCAM->setCBR_R_1_Hop(
            dynamic_cast<CooperativeAwarenessMessage*>(temp)->getCBR_R_1_Hop());

    //save last move-Object
    if (lastSentCAMMove != nullptr) {
        delete lastSentCAMMove;
    }
    lastSentCAMMove = new Move(curMove);

    // send the beacon
    sendWSM(temp);
    lastTxTime = simTime();

}

void BaseWaveApplLayer::performCBPMeasureInterval() {

    // Calculate CBR --> DONE
    Mac1609_4* macLayer = FindModule<Mac1609_4*>::findSubModule(
            getParentModule());
    //CBP
    lastTimeCBPWasCalculated = simTime();

    channelBusyPercentageWAVE =
            macLayer->getChannelBusyWaveListener()->calculateChannelBusyPercentage(
                    vCBPMeasInt, vCBPWeightFactor, busyTimeVec, standardCBPVec,
                    totalBusyTime, testCaseNumber, startTime);
    if (simTime() >= startTime + 6)
        channelBusyPercentageVec.record(channelBusyPercentageWAVE);
//
}

void BaseWaveApplLayer::calculateRadiatedPower() {

// RADIATED POWER 6.3.8.7, SAE J2945, Eq. 8 - 11
    if (txDecision_Critical_Event || txDecision_Dynamics) {

        rp = vRPMax;

    } else {

        //calculate fCBP

        if (channelBusyPercentageWAVE <= vMinChanUtil) {

            fCBP = vRPMax;

        } else if ((vMinChanUtil < channelBusyPercentageWAVE)
                && (channelBusyPercentageWAVE < vMaxChanUtil)) {

            fCBP = vRPMax
                    - ((vRPMax - vRPMin) / (vMaxChanUtil - vMinChanUtil))
                            * (channelBusyPercentageWAVE - vMinChanUtil);

        } else if (vMaxChanUtil <= channelBusyPercentageWAVE) {

            fCBP = vRPMin;

        }

        base_rp = previous_rp + vSUPRAGain * (fCBP - previous_rp);
        rp = base_rp;
        previous_rp = base_rp;
    }
    if (simTime() >= startTime + 6)
        radiatedPowerInDbmVec.record(rp);

    double convertedRadiatedPowerIntoMw = convertDBMtoMW();
    if (simTime() >= startTime + 6)
        radiatedPowerInMWVec.record(convertedRadiatedPowerIntoMw);

}

void BaseWaveApplLayer::calculateTrackingError() {
    //calculate tracking error
    //timPositionChange saves the time, at which the position changed the last time
    trackingError = trackingErrorCalculator->calculateTrackingError(
            timePositionChangedHVLocalEstimate, curPosition, curMove,
            lastHVStatus, lastMoveInHVStatus, testCaseNumber);
    if (simTime() >= startTime + 6)
        trackingErrorVec.record(trackingError);
}

void BaseWaveApplLayer::calculateTransmissionProbability() {
//calculate transmission probability

    transmissionProbability =
            transmissionProbabilityCalculator->calculateTransmissionProbability(
                    trackingError);
    if (simTime() >= startTime + 6)
        transmissionProbabilityVec.record(transmissionProbability);
}

void BaseWaveApplLayer::calculateMaximumInterTransmitTime() {

    //calculate MaxITT
    maximumInterTransmitTime =
            maximumInterTransmitTimeCalculator->calculateMaximumInterTransmission(
                    (double) (vehicleDensityInRange), smoothVehicleDensVec,
                    testCaseNumber, startTime);
    if (simTime() >= startTime + 6)
        maximumInterTransmitTimeCalculatorVec.record(maximumInterTransmitTime);
}

void BaseWaveApplLayer::performTransmissionDecision(bool txCriticalEvent) {
    //calculate Transmission decision and schedule decision
    txDecision_Critical_Event = false;
    txDecision_Dynamics = false;
    txDecision_Max_ITT = false;

    // use bernoulli trial
    int rand = bernoulli(0.5);

    //If (rand()<=p(k) && (NextScheduledMsgTime – CurrentTime)>=vRescheduleTh), then  Set TxDecision_Dynamics = 1
    //NextScheduledTime = sendBeaconEvt->getArrivalTime()
    if (rand <= transmissionProbability
            && (sendBeaconEvt->getArrivalTime() - simTime()) >= vRescheduleTh) {

        txDecision_Dynamics = true;
        if (simTime() - startTime > 6)
            txDecision_Dynamics_Count += 1;
    }

    if ((sendBeaconEvt->getArrivalTime()
            - (lastTxTime + maximumInterTransmitTime)) >= vRescheduleTh) {
        txDecision_Max_ITT = true;
        txDecision_Max_ITT_Count += 1;
    }

    //simulating critical event
    if (txCriticalEvent) {
        txDecision_Critical_Event = txCriticalEvent;
        txDecision_Critical_Event_Count += 1;
    }
}

void BaseWaveApplLayer::performScheduleTransmission() {

    //SAE j2945, 6.3.8.6,
    //nextScheduledTime = sendBeaconEvt->getArrivalTime
    //lastTXTime --> was set when sendWMS is called
    // currentTime = simTime()

    if (txDecision_Critical_Event || txDecision_Dynamics) {

        if (sendBeaconEvt->isScheduled()) {

            //cancel scheduled transmission of next beacon
            cancelEvent(sendBeaconEvt);

        }
        //schedule transmission now --> nextScheduledTime == currentTime
        scheduleAt(simTime(), sendBeaconEvt);

    } else if (txDecision_Max_ITT) {

        //cancel scheduledTransmission
        //nextScheduledTime = max(currentTime, lastTxTime + maxITT(k))

        if (sendBeaconEvt->isScheduled()) {

            cancelEvent(sendBeaconEvt);

        }
        simtime_t nextScheduledTime = SimTime(0);

        if (simTime() > (lastTxTime + maximumInterTransmitTime)) {

            nextScheduledTime = simTime();

        } else {

            nextScheduledTime = lastTxTime + maximumInterTransmitTime;
        }
        //reschedule at nextScheduleTime
        scheduleAt(nextScheduledTime, sendBeaconEvt);

    } else {

        //do nothing

    }
}

double BaseWaveApplLayer::convertDBMtoMW() {
//same calculation as in FWMath
    double txPowerInMW = 1 * pow(10.0, rp / 10.0);
    return txPowerInMW;

}

// this method is overridden in TraciDemo!
void BaseWaveApplLayer::sendWSM(WaveShortMessage* wsm) {

    sendDelayedDown(wsm, individualOffset);

}

void BaseWaveApplLayer::initAllWAVEParameters() {

//params taken from SAE J2945
    vCBPMeasInt = par("vCBPMeasInt").doubleValue();    // vCBPMeasInt, for timer
    vPERInterval = par("vPERInterval").doubleValue(); // vPERInterval, for timer
    vPERSubInterval = par("vPERSubInterval").doubleValue(); // vPERSubInterval, for timer
    vTxCntrlInt = par("vTxCntrlInt").doubleValue(); // vTxRateCntrlInt, for timer
    vPERRange = par("vPERRange").doubleValue(); // vPERRange in meter, used in PERCalculationCalculator
    vPERMax = par("vPERMax").doubleValue();  // used in PERCalculationCalculator
    vHVLocalPosEstIntMin = par("vHVLocalPosEstIntMin").doubleValue(); // used in TrackingErrorCalculator
    vHVLocalPosEstIntMax = par("vHVLocalPosEstIntMax").doubleValue(); // used in TrackingErrorCalculator
    vHVRemotePosEstIntMin = par("vHVRemotePosEstIntMin").doubleValue(); // used in TrackingErrorCalculator
    vHVRemotePosEstIntMax = par("vHVRemotePosEstIntMax").doubleValue(); // used in TrackingErrorCalculator
    vTrackingErrMin = par("vTrackingErrMin").doubleValue(); // used in TransmissionProbabilityCalculator
    vTrackingErrMax = par("vTrackingErrMax").doubleValue(); // used in TransmissionProbabilityCalculator
    vErrSensitivity = par("vErrSensitivity").doubleValue(); // used in TransmissionProbabilityCalculator
    vMax_ITT = par("vMax_ITT").doubleValue(); // used in MaximumInterTransmitTimeCalculator
    vDensityWeightFactor = par("vDensityWeightFactor").doubleValue(); // used in MaximumInterTransmitTimeCalculator
    vDensityCoefficient = par("vDensityCoefficient").doubleValue(); // used in MaximumInterTransmitTimeCalculator
    vRescheduleTh = par("vRescheduleTh").doubleValue(); //used for TransmissionDecision
    vTimeAccuracy = par("vTimeAccuracy").doubleValue(); // used for TransmissionDecison
    vCBPWeightFactor = par("vCBPWeightFactor").doubleValue(); // used for calculating CBP
    vRPMax = par("vRPMax").doubleValue();  // used in Radiated Power Calculation
    vRPMin = par("vRPMin").doubleValue();  // used in Radiated Power Calculation
    vSUPRAGain = par("vSUPRAGain").doubleValue(); // used in Radiated Power Calculation
    vMinChanUtil = par("vMinChanUtil").doubleValue(); // used in Radiated Power Calculation
    vMaxChanUtil = par("vMaxChanUtil").doubleValue(); // used in Radiated Power Calculation
    vTxRand = par("vTxRand").doubleValue(); // used in Radiated Power Calculation
    vRP = par("vRP").doubleValue();        // used in Radiated Power Calculation
    vMaxSuccessiveFail = par("vMaxSuccessiveFail").doubleValue();
    vRxSens = par("vRxSens").doubleValue();

//
//statistics
    channelBusyPercentageVec.setName("ChannelBusyPercentage");
    channelQualityIndicatorVec.setName("ChannelQualityIndicator");
    vehicleDensityInRangeVec.setName("VehicleDensityInRange");
    transmissionProbabilityVec.setName("TransmissionProbability");
    maximumInterTransmitTimeCalculatorVec.setName("MaximumInterTransmitTime");
    trackingErrorVec.setName("TrackingError");
    startBeaconVec.setName("startBeaconingTime");
    nextBeaconScheduleTimeVec.setName("currentBeaconIntervalVec");
    radiatedPowerInDbmVec.setName("Radiated Power in dBm");
    radiatedPowerInMWVec.setName("Radiated Power in mW");
    busyTimeVec.setName("busyTime");
    perVec.setName("PER");
    smoothVehicleDensVec.setName("smoothVehicleDens");
    standardCBPVec.setName("standardCBP");
    criticalEventVec.setName("CriticalEvents");
//
//calculated Values
    channelQualityIndicator = 0.0;            // calculated
    vehicleDensityInRange = 0;            // calculated
    channelBusyPercentageWAVE = 0.0;
    trackingError = 0.0;
    transmissionProbability = 0.0;
    maximumInterTransmitTime = 0.0;
//
//all timer objects
    timerMSG_CBPMeasureIntervalWAVE = new cMessage("channel busy period",
            CBP_MEASURE_INTERVAL_WAVE);
    timerMSG_PERCalculationIntervalWAVE = new cMessage("PERCalculationInterval",
            PER_CALCULATION_INTERVAL_WAVE);
    timerMSG_PERCalculationSubIntervalWAVE = new cMessage(
            "PERCalculationSubInterval", PER_CALCULATION_SUBINTERVAL_WAVE);
    timerMSG_TXRateCntrlIntervalWAVE = new cMessage("TXRateCntrlInterval",
            TX_RATE_CNTRL_INTERVAL_WAVE);
    timerMSG_RandomCriticalEvent = new cMessage("RandomCriticalEvent",
            RANDOM_CRITICAL_EVENT);
//
//local parameters and objects, helping calculation
    msgCount = 1;            // necessary for BSM
    timePositionChangedHVLocalEstimate = SimTime(0);
    lastHVStatus = nullptr;
    lastMoveInHVStatus = nullptr;
    perCalculator = new PERAndChannelQualityIndicatorCalculator(this,
            vPERInterval, vPERSubInterval, vPERRange, vPERMax);
    trackingErrorCalculator = new TrackingErrorCalculator(this,
            vHVLocalPosEstIntMin, vHVLocalPosEstIntMax, vHVRemotePosEstIntMin,
            vHVRemotePosEstIntMax);
    transmissionProbabilityCalculator = new TransmissionProbabilityCalculator(
            this, vTrackingErrMin, vTrackingErrMax, vErrSensitivity);
    maximumInterTransmitTimeCalculator = new MaximumInterTransmitTimeCalculator(
            this, vMax_ITT, vDensityWeightFactor, vDensityCoefficient);
    txDecision_Critical_Event = false; //used for TransmissionDecision --> true, when a critical event occurs
    txDecision_Dynamics = false;            //used for TransmissionDecision
    txDecision_Max_ITT = false;            //used for TransmissionDecision
    lastTxTime = SimTime(0);            // used for TransmissionDecision
    randomCriticalEvent = par("randomCriticalEvent").boolValue(); //used in Transmission Decision, simulates randomly critical events
    rp = vRP;            // calculated Radiated power, initial Value is vRP
    base_rp = vRP;            // used in calculating Radiated power
    previous_rp = vRP;            // used in calculating Radiated power
    fCBP = 0.0;            // used in calculating Radiated power
    txFailed = 0;
    counterLastHVStatusNotChanged = 0;
    txDecision_Critical_Event_Count = 0;         // set in transmission decision
    txDecision_Dynamics_Count = 0;            // set in transmission decision
    txDecision_Max_ITT_Count = 0;            // set in transmission decision
    counterNextScheduleTime = 0;
    counterUseBeaconInterval = 0;
    counterSentBeacon = 0;            // cout the sent beacons
    counterReceivedBeacon = 0;
    totalBusyTime = 0;
    lastTimeCBPWasCalculated = 0;
    calcVeh = par("calcVeh").boolValue();
    if (calcVeh) {
        startTime = simTime();
    }

    if (strcmp(getParentModule()->getFullName(), "node[0]") == 0) {

        if (randomCriticalEvent) {
            std::cerr << "Random Critical Events ON" << std::endl;
        } else {
            std::cerr << "Random Critical Events OFF" << std::endl;
        }
        if (testCaseNumber > 0) {
            std::cerr << " TestCase with number " << testCaseNumber
                    << std::endl;
        }

    }
}

void BaseWaveApplLayer::finish() {

    if (sendBeaconEvt->isScheduled()) {
        cancelAndDelete(sendBeaconEvt);
    } else {
        delete sendBeaconEvt;
    }

    if (sendDataEvt != nullptr) {
        if (sendDataEvt->isScheduled()) {
            cancelAndDelete(sendDataEvt);
        } else {
            delete sendDataEvt;
        }
    }

//WAVE
    if (beaconWAVE) {

        if (timerMSG_CBPMeasureIntervalWAVE->isScheduled()) {
            cancelAndDelete(timerMSG_CBPMeasureIntervalWAVE);
        } else {
            delete timerMSG_CBPMeasureIntervalWAVE;
        }

        if (timerMSG_PERCalculationIntervalWAVE->isScheduled()) {
            cancelAndDelete(timerMSG_PERCalculationIntervalWAVE);
        } else {
            delete timerMSG_PERCalculationIntervalWAVE;
        }

        if (timerMSG_PERCalculationSubIntervalWAVE->isScheduled()) {
            cancelAndDelete(timerMSG_PERCalculationSubIntervalWAVE);
        } else {
            delete timerMSG_PERCalculationSubIntervalWAVE;
        }

        if (timerMSG_TXRateCntrlIntervalWAVE->isScheduled()) {
            cancelAndDelete(timerMSG_TXRateCntrlIntervalWAVE);
        } else {
            delete timerMSG_TXRateCntrlIntervalWAVE;
        }

        if (lastHVStatus->isScheduled()) {
            cancelAndDelete(lastHVStatus);
        } else {
            delete lastHVStatus;
        }

        if (timerMSG_RandomCriticalEvent->isScheduled()) {
            cancelAndDelete(timerMSG_RandomCriticalEvent);
        } else {
            delete timerMSG_RandomCriticalEvent;
        }

        if (perCalculator != nullptr) {
            delete perCalculator;
        }

        if (trackingErrorCalculator != nullptr) {
            delete trackingErrorCalculator;
        }

        if (lastMoveInHVStatus != nullptr) {
            delete lastMoveInHVStatus;
        }

        if (transmissionProbabilityCalculator != nullptr) {
            delete transmissionProbabilityCalculator;
        }

        if (maximumInterTransmitTimeCalculator != nullptr) {
            delete maximumInterTransmitTimeCalculator;
        }

// only necessary to calculate the correct sum of busyTimes
        Mac1609_4* macLayer = FindModule<Mac1609_4*>::findSubModule(
                getParentModule());
//CBP
        channelBusyPercentageWAVE =
                macLayer->getChannelBusyWaveListener()->calculateChannelBusyPercentage(
                        simTime() - lastTimeCBPWasCalculated, vCBPWeightFactor,
                        busyTimeVec, standardCBPVec, totalBusyTime,
                        testCaseNumber, startTime);
        lastTimeCBPWasCalculated = simTime();
        if (simTime() >= startTime + 6)
            channelBusyPercentageVec.record(channelBusyPercentageWAVE);

        if (simTime() - startTime > 6) {
            recordScalar("Counter LastHVStatus NOT changed",
                    counterLastHVStatusNotChanged);
            recordScalar("txDynamics", txDecision_Dynamics_Count);
            recordScalar("txCriticalEvent", txDecision_Critical_Event_Count);
            recordScalar("txMaxItt", txDecision_Max_ITT_Count);
            recordScalar("CounterNextScheduleTimeCalculated",
                    counterNextScheduleTime);
            recordScalar("CounterUseBeaconInterval", counterUseBeaconInterval);
            recordScalar("CounterSentBeacon", counterSentBeacon);
            recordScalar("TotalBusyTime", totalBusyTime.dbl());
            recordScalar("Received Beacons", counterReceivedBeacon);
        }
    } //WAVE
    else if (beaconETSI) {
        if (simTime() - startTime > 6) {
            recordScalar("CounterSentBeacon", counterSentBeacon);
            recordScalar("Received Beacons", counterReceivedBeacon);
        }
    }

//
    findHost()->unsubscribe(mobilityStateChangedSignal, this);

}

//--> packetInterval DCC is necessary for generating CAM
//T_GenCam_DCC
// see TS 102 687, TS 102 637-2 Page 18, TS 102 724 Page 10 AND C.2.2, EN 302 637-2 6.1.3
void BaseWaveApplLayer::setPaketInterval(simtime_t paketInterval,
        bool activeState) {

    if (paketInterval >= T_GenCamMin && paketInterval <= T_GenCamMax) {
        T_GenCam_DCC = paketInterval;
    } else if (paketInterval > T_GenCamMax) {
        T_GenCam_DCC = T_GenCamMax;
    } else if (paketInterval < T_GenCamMin) {
        T_GenCam_DCC = T_GenCamMin;
    } else {
        throw cRuntimeError(
                "Error - BaseWaveApplLayer::setPaketInterval --> invalid paketInterval");
    }

}

BaseWaveApplLayer::~BaseWaveApplLayer() {

}
