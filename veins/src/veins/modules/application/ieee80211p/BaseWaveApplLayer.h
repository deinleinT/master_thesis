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

#ifndef BASEWAVEAPPLLAYER_H_
#define BASEWAVEAPPLLAYER_H_

#include <map>
#include <math.h>
#include <omnetpp.h>
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"
#include "veins/masterthesis/WAVE/message/BasicSafetyMessage_m.h"
#include "veins/masterthesis/WAVE/calculation/PER/PERAndChannelQualityIndicatorCalculator.h"
#include "veins/masterthesis/WAVE/calculation/TrackingError/TrackingErrorCalculator.h"
#include "veins/base/utils/Move.h"
#include "veins/masterthesis/WAVE/message/BasicSafetyMessage_m.h"
#include "veins/masterthesis/ETSI/message/CooperativeAwarenessMessage_m.h"
#include "veins/masterthesis/WAVE/calculation/TransmissionProbability/TransmissionProbabilityCalculator.h"
#include "veins/masterthesis/WAVE/calculation/MaximumInterTransmitTime/MaximumInterTransmitTimeCalculator.h"
#include "veins/masterthesis/util/TestCaseHandler.h"
#include "veins/masterthesis/ETSI/util/CAMGenerationHelper.h"

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

/**
 * @brief
 * WAVE application layer base class.
 *
 * @author David Eckhoff
 * @author Thomas Deinlein: added code due to masterthesis
 *
 * @ingroup applLayer
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class BaseWaveApplLayer: public BaseApplLayer {

public:
    ~BaseWaveApplLayer();
    virtual void initialize(int stage);
    virtual void finish();
    void setPaketInterval(simtime_t paketInterval, bool activeState);

    virtual void receiveSignal(cComponent* source, simsignal_t signalID,
            cObject* obj, cObject* details);

    double getRxSens() const {
        return vRxSens;
    }

    enum WaveApplMessageKinds {
        SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
        SEND_BEACON_EVT,
        SEND_BEACON_BSM,
        SEND_BEACON_CAM,
        CHECK_CAM_PARAMS,
        CBP_MEASURE_INTERVAL_WAVE,
        TX_RATE_CNTRL_INTERVAL_WAVE,
        PER_CALCULATION_INTERVAL_WAVE,
        PER_CALCULATION_SUBINTERVAL_WAVE,
        RANDOM_CRITICAL_EVENT,
        SEND_DATA

    };

protected:

    static const simsignalwrap_t mobilityStateChangedSignal;

    /** @brief handle messages from below */
    virtual void handleLowerMsg(cMessage* msg);
    /** @brief handle self messages */
    virtual void handleSelfMsg(cMessage* msg);

    virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits,
            t_channel channel, int priority, int rcvId, int serial = 0);
    virtual WaveShortMessage* prepareData(std::string name, int lengthBits,
            t_channel channel, int priority, int rcvId, int serial);
    virtual void sendWSM(WaveShortMessage* wsm);
    virtual void onBeacon(WaveShortMessage* wsm) = 0;
    virtual void onData(WaveShortMessage* wsm) = 0;
    virtual void handlePositionUpdate(cObject* obj);

protected:
    int beaconLengthBits;
    int beaconPriority;
    bool sendData;
    bool sendBeacons;
    bool beaconETSI;

    //WAVE Specific
    cOutVector channelBusyPercentageVec;
    cOutVector channelQualityIndicatorVec;
    cOutVector vehicleDensityInRangeVec;
    cOutVector trackingErrorVec;
    cOutVector transmissionProbabilityVec;
    cOutVector maximumInterTransmitTimeCalculatorVec;
    cOutVector startBeaconVec;
    cOutVector nextBeaconScheduleTimeVec;
    cOutVector radiatedPowerInDbmVec;
    cOutVector radiatedPowerInMWVec;
    cOutVector busyTimeVec;
    cOutVector perVec;
    cOutVector smoothVehicleDensVec;
    cOutVector standardCBPVec;
    cOutVector criticalEventVec;
    cOutVector speedVec;
    cOutVector etsiCAMConditionVector;

    int txDecision_Dynamics_Count;
    int txDecision_Max_ITT_Count;
    int txDecision_Critical_Event_Count;
    bool beaconWAVE; // use WAVE DCC
    int msgCount; // used in BSM as sequence number, important for PER
    PERAndChannelQualityIndicatorCalculator * perCalculator;
    double vCBPMeasInt; // vCBPMeasInt in SAE J2945
    double vPERSubInterval; // vPERSubInterval in SAE J2945
    double vPERInterval; // vPERInterval in SAE J2945
    double channelBusyPercentageWAVE; // CBP in SAE J2945
    double vTxCntrlInt; // vTxRateCntrlInt in SAE J2945
    double vPERRange; // vPERRange in meter
    double vPERMax; // vPERMax
    double vCBPWeightFactor; // vCBPWeightFactor
    double vHVLocalPosEstIntMin; // used in TrackingErrorCalculator
    double vHVLocalPosEstIntMax; // used in TrackingErrorCalculator
    double vHVRemotePosEstIntMin; // used in TrackingErrorCalculator
    double vHVRemotePosEstIntMax; // used in TrackingErrorCalculator
    double vTrackingErrMin; // used in TransmissionProbabilityCalculator
    double vTrackingErrMax; // used in TransmissionProbabilityCalculator
    double vErrSensitivity; // used in TransmissionProbabilityCalculator
    double vMax_ITT; // used in MaximumInterTransmitTimeCalculator
    double vDensityWeightFactor; // used in MaximumInterTransmitTimeCalculator
    double vDensityCoefficient; // used in MaximumInterTransmitTimeCalculator
    double vRPMax; // used in Radiated Power Calculation
    double vRPMin; // used in Radiated Power Calculation
    double vSUPRAGain; // used in Radiated Power Calculation
    double vMinChanUtil; // used in Radiated Power Calculation
    double vMaxChanUtil; // used in Radiated Power Calculation
    double vTxRand; // used in Radiated Power Calculation
    double vRP; // used in Radiated Power Calculation
    double vRxSens; //sensitivity
    int counterLastHVStatusNotChanged;
    int counterNextScheduleTime;
    int counterUseBeaconInterval;
    int counterSentBeacon;
    int counterReceivedBeacon;
    double testCaseNumber;

    double channelQualityIndicator; //
    bool txDecision_Critical_Event; //used for TransmissionDecision
    bool txDecision_Dynamics; //used for TransmissionDecision
    bool txDecision_Max_ITT; //used for TransmissionDecision
    double vRescheduleTh; // used for TransmissionDecison
    double vTimeAccuracy; // used for TransmissionDecison
    simtime_t lastTxTime; //used for TransmissionDecision
    bool randomCriticalEvent; // used to simulate randomly critical event

    int vehicleDensityInRange; // N(k) in SAE J2945
    cMessage* timerMSG_CBPMeasureIntervalWAVE; // used as timer for vCBPMeasInt
    cMessage* timerMSG_PERCalculationSubIntervalWAVE; // used as timer for vPERSubInterval
    cMessage* timerMSG_PERCalculationIntervalWAVE; // used as timer for vPERInterval
    cMessage* timerMSG_TXRateCntrlIntervalWAVE; // used as timer for
    cMessage* timerMSG_RandomCriticalEvent;
    TrackingErrorCalculator * trackingErrorCalculator;
    double trackingError; // e(k) in SAE J2945
    simtime_t timePositionChangedHVLocalEstimate; // used for calulation of e(k)
    Move curMove; // current Move-Object, used for calculation of e(k)
    Move * lastMoveInHVStatus; // used for calculation of e(k)
    BasicSafetyMessage * lastHVStatus; // used for calculation of e(k)
    TransmissionProbabilityCalculator * transmissionProbabilityCalculator;
    double transmissionProbability;
    MaximumInterTransmitTimeCalculator * maximumInterTransmitTimeCalculator;
    double maximumInterTransmitTime; // calculated max_ITT
    double rp; // calculated radiated power
    double base_rp; // used in Radiated Power Calculation
    double previous_rp; // used in Radiated Power Calculation
    double fCBP; // used in Radiated Power Calculation
    double vMaxSuccessiveFail; // used in Assumption of Latest HV State Information at RVs (sendBeaconTimer)
    int txFailed;
    simtime_t totalBusyTime;


    //calc vehicles in range the first time after 1s
    bool calcVeh;
    simtime_t startTime;
    //

    simtime_t T_GenCam_DCC;
    simtime_t const T_GenCamMax = SimTime(1.0);
    simtime_t const T_GenCamMin = SimTime(0.1);
    simtime_t T_GenCam;
    simtime_t T_CheckCamGen;

    simtime_t defaultBeaconInterval;
    simtime_t lastTimeCBPWasCalculated;
    //
    CooperativeAwarenessMessage * lastSentCAM;
    Move * lastSentCAMMove;
    int const numGenCam = 3;
    int counterNumGenCam;

    simtime_t individualOffset;
    int dataLengthBits;
    bool dataOnSch;
    int dataPriority;
    Coord curPosition;
    int mySCH;
    int myId;

    cMessage* sendBeaconEvt; // --> send CAMS or BSM
    cMessage* checkCAMParams; // --> check all 100s CAM params

    double sendDataInterval;
    cMessage* sendDataEvt;
    int counterReceivedData;

    WaveAppToMac1609_4Interface* myMac;

private:
    void calculateCompletePERValuesWAVE();
    void sendBeaconBSM(bool previousBeaconFlag);
    void sendBeaconCAM();
    void performCBPMeasureInterval();
    void calculateTrackingError();
    void calculateTransmissionProbability();
    void calculateMaximumInterTransmitTime();
    void performTransmissionDecision(bool txCriticalEvent);
    void performScheduleTransmission();
    double convertDBMtoMW();
    void calculateRadiatedPower();
    void updateTxFailed();
    void assumptionOfLatestHVStatusAndSendBSM();
    void calculateNextScheduleTimeAndScheduleNextBeacon(cMessage* msg);
    void initAllWAVEParameters();
    void assertPERTestCases();
    void assertTrackingErrorTests();
    void prepareTransmissionProbabilityTests();
    void assertTransmissionProbabilityTest();
    void assertMaxInterTransmitTimeTest();
    void performCalculatingRadiatedPowerTest();
    void assertCalculatingRadiatedPower();
    void performCAMGenerating();

};

#endif /* BASEWAVEAPPLLAYER_H_ */
