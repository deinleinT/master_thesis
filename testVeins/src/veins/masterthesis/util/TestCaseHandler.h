/*
 * TestCaseDescription.h
 *
 *  Created on: 28.02.2017
 *      Author: thoma
 */

#ifndef SRC_VEINS_MASTERTHESIS_UTIL_TESTCASEHANDLER_H_
#define SRC_VEINS_MASTERTHESIS_UTIL_TESTCASEHANDLER_H_

#include <string>
#include <omnetpp.h>

class TestCaseHandler {

public:
    static std::string getDescription(double testCasenumber) {

        if (testCasenumber == 0.0) {

            return "Regular Mode - No TestCase chosen";

        } else if (testCasenumber == 1.1) {

            return " TestCase ETSI 1.1, set channelLoad 0.149 ";

        } else if (testCasenumber == 1.2) {

            return " TestCase ETSI 1.2, set channelLoad 0.15 ";

        } else if (testCasenumber == 1.3) {

            return " TestCase ETSI 1.3, set channelLoad 0.399 % ";

        } else if (testCasenumber == 1.4) {

            return " TestCase ETSI 1.4, set channelLoad 0.4 % ";

        } else if (testCasenumber == 1.5) {

            return " TestCase ETSI 1.5, set channelLoad during simtime >= 1 and simtime <= 1.1 to 0.20 and during simtime >= 6 and simtime <= 7.1 to 0.8 ";

        } else if (testCasenumber == 1.6) {

            return " TestCase ETSI 1.6, *** run Test within ChannelLoadListenerEtsi and confirm Tests with ASSERT \n(--> Intervalstart is set to 0.7, intervalend is set to 1.5, in vector are two busy times, 0.5-1.0 --> busyTime 0.3; 1.1-1.2 --> busyTime 0.1; and flag isBusy is set to true, lastBusytime is set to 1.4 --> 1.5-1.4=0.1 ==> totalBusyTime has to be 0.5) ***";

        } else if (testCasenumber == 2.1) {

            return " TestCase WAVE 2.1, set rawCBP 49.9% ";

        } else if (testCasenumber == 2.2) {

            return " TestCase WAVE 2.2, set rawCBP 50 ";

        } else if (testCasenumber == 2.3) {

            return " TestCase WAVE 2.3, set rawCBP 79.9 % ";

        } else if (testCasenumber == 2.4) {

            return " TestCase WAVE 2.4, set rawCBP 80 % ";

        } else if (testCasenumber == 2.5) {

            return " TestCase WAVE 2.5, *** run Test within ChannelBusyListener and confirm Tests with ASSERT (--> Intervalstart is set to 0.7, intervalend is set to 1.5, in vector are two busy times, 0.5-1.0 --> busyTime 0.3; 1.1-1.2 --> busyTime 0.1; and flag isBusy is set to true, lastBusytime is set to 1.4 --> 1.5-1.4=0.1 ==> totalBusyTime has to be 0.5) ***";

        } else if (testCasenumber == 3.1) {

            return " TestCase WAVE 3.1, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (no BSM received) ";

        } else if (testCasenumber == 3.2) {

            return " TestCase WAVE 3.2, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (BSM arrived, but not within vPERSubInterval) ";

        } else if (testCasenumber == 3.3) {

            return " TestCase WAVE 3.3, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (BSM arrived, but not within vPERSubInter and Range) ";

        } else if (testCasenumber == 3.4) {

            return " TestCase WAVE 3.4, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (Appendix A.8.3 example 2a) ";

        } else if (testCasenumber == 3.5) {

            return " TestCase WAVE 3.5, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (Car is out of Range) ";

        } else if (testCasenumber == 3.6) {

            return " TestCase WAVE 3.6, run Test within PERAndChannelQualityIndicatorCalculator and confirm Tests with ASSERT (two cars in range, both with PER 0.75) ";

        } else if (testCasenumber == 4.1) {

            return " TestCase WAVE 4.1, run Test within TrackingErrorCalculator and confirm Tests with ASSERT (deltaTimeHVLocalEstimate = 0.16 --> trackingError is 0) ";

        } else if (testCasenumber == 4.2) {

            return " TestCase WAVE 4.2, run Test within TrackingErrorCalculator and confirm Tests with ASSERT (deltaTimeHVRemoteEstimate = 3.1 --> trackingError is 0) ";

        } else if (testCasenumber == 4.3) {

            return " TestCase WAVE 4.3, run Test within TrackingErrorCalculator and confirm Tests with ASSERT (deltaTimeHVRemoteEstimate = 0.04 and deltaTimeHVLocalEstimate = 0.04 --> trackingError is calculated with dummy coord --> trackingError is 1) ";

        } else if (testCasenumber == 4.4) {

            return " TestCase WAVE 4.4, run Test within TrackingErrorCalculator and confirm Tests with ASSERT (deltaTimeHVRemoteEstimate = 0.06 and deltaTimeHVLocalEstimate = 0.06 --> trackingError is calculated with dummy move objects--> trackingError is between 0.95and 1.01) ";

        } else if (testCasenumber == 5.1) {

            return " TestCase WAVE 5.1, run Test within TransmissionProbability and confirm Tests with ASSERT (trackingError == 0.19 --> transmissionPro is 0) ";

        } else if (testCasenumber == 5.2) {

            return " TestCase WAVE 5.2, run Test within TransmissionProbability and confirm Tests with ASSERT (trackingError == 0.20 --> transmissionPro is 0) ";

        } else if (testCasenumber == 5.3) {

            return " TestCase WAVE 5.3, run Test within TransmissionProbability and confirm Tests with ASSERT (trackingError == 0.49 --> transmissionPro is ...) ";

        } else if (testCasenumber == 5.4) {

            return " TestCase WAVE 5.4, run Test within TransmissionProbability and confirm Tests with ASSERT (trackingError == 0.5 --> transmissionPro is 1) ";

        } else if (testCasenumber == 5.5) {

            return " TestCase WAVE 5.5, run Test within TransmissionProbability and confirm Tests with ASSERT (trackingError == 0.8 --> transmissionPro is 1) ";

        } else if (testCasenumber == 6.1) {

            return " TestCase WAVE 6.1, run Test within MaximumInterTransmistTimeCalculator and confirm Tests with ASSERT (vehicleDensityInRange = 0, --> smoothVehicleDensityInRange = 0) ";

        } else if (testCasenumber == 6.2) {

            return " TestCase WAVE 6.2, run Test within MaximumInterTransmistTimeCalculator and confirm Tests with ASSERT (smoothVehicleDensityInRange = 25 --> Max_ITT = 0.1) ";

        } else if (testCasenumber == 6.3) {

            return " TestCase WAVE 6.3, run Test within MaximumInterTransmistTimeCalculator and confirm Tests with ASSERT (smoothVehicleDensityInRange = 26 --> Max_ITT = 0.104) ";

        } else if (testCasenumber == 6.4) {

            return " TestCase WAVE 6.4, run Test within MaximumInterTransmistTimeCalculator and confirm Tests with ASSERT (smoothVehicleDensityInRange = 149 --> Max_ITT = 0.596) ";

        } else if (testCasenumber == 6.5) {

            return " TestCase WAVE 6.5, run Test within MaximumInterTransmistTimeCalculator and confirm Tests with ASSERT (smoothVehicleDensityInRange = 150 --> Max_ITT = 0.6) ";

        } else if (testCasenumber == 7.1) {

            return " TestCase WAVE 7.1, run Test Calculating Radiated Power (set TX_DecisionDynamics to true --> rp == vRPMax) ";

        } else if (testCasenumber == 7.2) {

            return " TestCase WAVE 7.2, run Test Calculating Radiated Power (set TX_DecisionDynamics to false and CBP to vMinChanUtil --> fCBP == vRPMax) ";

        } else if (testCasenumber == 7.3) {

            return " TestCase WAVE 7.3, run Test Calculating Radiated Power (set TX_DecisionDynamics to false and CBP to vMinChanUtil+1 --> fCBP == 19.6667) ";

        } else if (testCasenumber == 7.4) {

            return " TestCase WAVE 7.4, run Test Calculating Radiated Power (set TX_DecisionDynamics to false and CBP to vMaxChanUtil --> fCBP == vRPMin) ";

        } else if (testCasenumber == 7.5) {

            return " TestCase WAVE 7.5, run Test Calculating Radiated Power (set TX_DecisionDynamics to false and CBP to vMaxChanUtil+1 --> fCBP == vRPMin) ";

        } else if (testCasenumber == 7.6) {

            return " TestCase WAVE 7.6, run Test Calculating Radiated Power (set TX_DecisionDynamics to false and CBP to 100 --> fCBP == vRPMin) ";

        } else if (testCasenumber == 8) {

            return " TestCase WAVE and ETSI, sending a BSM or CAM with specific value and test the value at the receiver ";

        }else if (testCasenumber == 9.1) {

            return " TestCase ETSI, DccFlowControl --> one Message is in the Queue, Timestamp is older than 1s --> drop the Message ";

        }else if (testCasenumber == 9.2) {

            return " TestCase ETSI, DCC QUEUE is has two packets, not more packets are allowed, drop the msg ";

        } else {
            std::string text = "Unknown TestCaseNumber "
                    + omnetpp::double_to_str(testCasenumber);
            throw omnetpp::cRuntimeError(text.c_str());
        }
    }
    ;

    static double getTestValueETSI(double testCaseNumber) {

        if (testCaseNumber == 1.1) {
            return 0.149;
        } else if (testCaseNumber == 1.2) {
            return 0.15;
        } else if (testCaseNumber == 1.3) {
            return 0.399;
        } else if (testCaseNumber == 1.4) {
            return 0.4;
        } else if (testCaseNumber == 1.5) {
            if (omnetpp::simTime() >= 1 && omnetpp::simTime() <= 1.1) {
                return 0.20;
            } else if (omnetpp::simTime() >= 6 && omnetpp::simTime() <= 7.1) {
                return 0.8;
            } else {
                return 0.1;
            }

        } else {
            std::string text = "Unknown TestCaseNumber "
                    + omnetpp::double_to_str(testCaseNumber) + " in ETSI Mode";
            throw omnetpp::cRuntimeError(text.c_str());
        }
    }
    ;

    static double getTestValueWAVE(double testCaseNumber) {
        // all numbers with 2 tests cbp
        if (testCaseNumber == 2.1) {
            return 49.9;
        } else if (testCaseNumber == 2.2) {
            return 50.0;
        } else if (testCaseNumber == 2.3) {
            return 79.9;
        } else if (testCaseNumber == 2.4) {
            return 80;
        } else {
            std::string text = "Unknown TestCaseNumber "
                    + omnetpp::double_to_str(testCaseNumber) + " in WAVE Mode";
            throw omnetpp::cRuntimeError(text.c_str());
        }
    }
    ;

}
;

#endif /* SRC_VEINS_MASTERTHESIS_UTIL_TESTCASEHANDLER_H_ */
