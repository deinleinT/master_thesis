//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "veins/modules/application/traci/TraCIDemo11p.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

const simsignalwrap_t TraCIDemo11p::parkingStateChangedSignal = simsignalwrap_t(
TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(TraCIDemo11p);

void TraCIDemo11p::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();
        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        sentMessage = false;
        lastDroveAt = simTime();
        findHost()->subscribe(parkingStateChangedSignal, this);
        isParking = false;
        sendWhileParking = par("sendWhileParking").boolValue();
    }
}

void TraCIDemo11p::onBeacon(WaveShortMessage* wsm) {

    //TODO: RECEIVE BEACON FROM REMOTE VEHICLE

    switch (wsm->getKind()) {
    case SEND_BEACON_EVT: {
        break;
    }
    case SEND_BEACON_CAM: {

        Mac1609_4* macLayer = FindModule<Mac1609_4*>::findSubModule(
                getParentModule());

        CooperativeAwarenessMessage * cam =
                dynamic_cast<CooperativeAwarenessMessage*>(wsm);

        //TODO
                //CBR_L_1_Hop is the maximum CBR_R_0_Hop value received from a neighbouring ITS-S in a given T_cbr interval, i.e. it is the 1-hop channel busy ratio. It is subsequently disseminated to  neighbours as CBR_R_1_Hop
                macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_1_Hop =
                        fmax(cam->getCBR_R_0_Hop(),
                                macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_1_Hop);

                // CBR_L_2_Hop is the maximum CBR_R_1_Hop value received from a neighbouring ITS-S in a given T_cbr interval, i.e. it is the 2-hop channel busy ratio. It is never disseminated by ego ITS-S
                macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_2_Hop =
                        fmax(cam->getCBR_R_1_Hop(),
                                macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_2_Hop);
                //

//        if (strcmp(getParentModule()->getFullName(), "node[0]") == 0) {
//            std::cerr << "Receive Beacon at " << simTime() << " CamCBR_R_0_Hop "
//                    << cam->getCBR_R_0_Hop() << " and in listener "
//                    << macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_1_Hop
//                    << " CamCBR_R_1_hop " << cam->getCBR_R_1_Hop()
//                    << " and in listener "
//                    << macLayer->getChannelLoadListenerETSI()->setCbrValues().CBR_L_2_Hop
//                    << std::endl;
//        }

        if (testCaseNumber == 8)
            ASSERT(strcmp(cam->getItsPDUHeader(), "test") == 0);

        counterReceivedBeacon++;

        break;
    }
    case SEND_BEACON_BSM: {
        //TODO
        //receive Beacon as BSM from a car

        BasicSafetyMessage * bsm = dynamic_cast<BasicSafetyMessage*>(wsm);

        if (testCaseNumber == 8)
            ASSERT(bsm->getELEVATION() == 123456);

        //PER
        perCalculator->receivedBSM(bsm->getCarId(), bsm->getMSG_COUNT(),
                simTime(), bsm->getSenderPos());
        counterReceivedBeacon++;
        //

        break;
    }

    default: {
        //TODO:
        break;
    }
    }
}

void TraCIDemo11p::onData(WaveShortMessage* wsm) {

    counterReceivedData++;

//    findHost()->getDisplayString().updateWith("r=16,green");
//    annotations->scheduleErase(1,
//            annotations->drawLine(wsm->getSenderPos(),
//                    mobility->getPositionAt(simTime()), "blue"));

//    if (mobility->getRoadId()[0] != ':')
//        traciVehicle->changeRoute(wsm->getWsmData(), 9999);
//    if (!sentMessage)
//        sendMessage(wsm->getWsmData());
}

void TraCIDemo11p::sendMessage(std::string blockedRoadId) {
    sentMessage = true;

    t_channel channel = dataOnSch ? type_SCH : type_CCH;
    WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel,
            dataPriority, -1, 2);
    wsm->setWsmData(blockedRoadId.c_str());
    sendWSM(wsm);
}

void TraCIDemo11p::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}
void TraCIDemo11p::handleParkingUpdate(cObject* obj) {
    isParking = mobility->getParkingState();
    if (sendWhileParking == false) {
        if (isParking == true) {
            (FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(
                    this->getParentModule()->getSubmodule("nic"));
        } else {
            Coord pos = mobility->getCurrentPosition();
            (FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(
                    this->getParentModule()->getSubmodule("nic"),
                    (ChannelAccess*) this->getParentModule()->getSubmodule(
                            "nic")->getSubmodule("phy80211p"), &pos);
        }
    }
}
void TraCIDemo11p::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10) {
            findHost()->getDisplayString().updateWith("r=16,red");
            if (!sentMessage)
                sendMessage(mobility->getRoadId());
        }
    } else {
        lastDroveAt = simTime();
    }
}
void TraCIDemo11p::sendWSM(WaveShortMessage* wsm) {

    if (std::string(wsm->getName()).find("beacon") != std::string::npos) {
        counterSentBeacon++;
    }

    if (isParking && !sendWhileParking)
        return;

    sendDelayedDown(wsm, individualOffset);
}
