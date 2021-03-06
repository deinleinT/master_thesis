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

#include "PERAndChannelQualityIndicatorCalculator.h"

PERAndChannelQualityIndicatorCalculator::PERAndChannelQualityIndicatorCalculator(
        omnetpp::cSimpleModule * source, double vPERInterval,
        double vPERSubInterval, double vPERRange, double vPERMax) {
    this->vPERInterval = vPERInterval;
    this->vPERSubInterval = vPERSubInterval;
    this->vPERRange = vPERRange;
    this->vPERMax = vPERMax;
    this->source = source;
}

PERAndChannelQualityIndicatorCalculator::~PERAndChannelQualityIndicatorCalculator() {
    manageMap.clear();
}

void PERAndChannelQualityIndicatorCalculator::receivedBSM(std::string carId,
        int msgCount, omnetpp::simtime_t arrivaltime, Coord position) {

    //TODO DONE
    ASSERT(msgCount >= 0);

    struct carinfo car;
    struct carinfo car2;

    //save the BSM into the Map
    int receivedmsgs = manageMap[carId].receivedMsgs;
    if (receivedmsgs == 0) {
        //first time BSM from this carId arrived
        car = {carId, msgCount, 1, arrivaltime, true, position, false, msgCount, -1};
        manageMap[carId] = car;
    } else if(receivedmsgs > 0) {
        int saveFirstMsgCount = manageMap[carId].msgcountFromFirstRecMsg;
        double perCalculated = manageMap[carId].perCalculated;
        car2 = {carId, msgCount, receivedmsgs + 1, arrivaltime, false, position, true, saveFirstMsgCount, perCalculated};
        manageMap[carId] = car2;
    } else {
        throw cRuntimeError("PERCalculator receivedBSM --> receivedmsgs < 0   not possible!");
    }
}

void PERAndChannelQualityIndicatorCalculator::calculatePER(
        Coord currentPosition, simtime_t currentTime, cOutVector & perVec,
        double testCaseNumber) {

    if (testCaseNumber >= 3 && testCaseNumber < 4) {
        Coord temp(1, 1, 0);
        currentPosition = temp;
        currentTime = SimTime(6);
        performTestCase(testCaseNumber);
    }

    //TODO first clean the map, all messages which are older than perCalculationIntervalWAVE --> delete   DONE
    std::map<std::string, carinfo>::iterator iter;
    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {
        if (currentTime - iter->second.lastarrivaltime > vPERInterval) {

            manageMap.erase(iter->second.carId);

        }
    }

    //TODO from cars from which only one message arrived, don't use in calc, but don't delete! --> DONE
    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {
        if (iter->second.firstMessage) {

            //only one message from this car received within PERCalculationInterval --> PER is undefined
            iter->second.useInCalc = false;
            iter->second.perCalculated = -1;

        }
    }

    /* taken from SAE J2945, Page 67
     * An RV is within vPERRange if the last BSM received from that RV during perCalculationSubIntervalWAVE contains a 2-D position within vPERRange (100m) of
     * the HV�s most recent 2-D Position Reference as of the time when PER is calculated. If a BSM is not received from that RV
     * during perCalculationSubIntervalWAVE the RV is not within vPERRange. */
    //TODO last BSM rec within perCalculationSubIntervalWAVE? if not, do not use it --> DONE
    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {
        // if BSM NOT received within perCalculationSubIntervalWAVE --> don't use it
        if (currentTime - iter->second.lastarrivaltime > vPERSubInterval) {

            iter->second.useInCalc = false;
        }

        if (iter->second.useInCalc) {

            //check, whether the distance between HV and RV is within PERRange
            Coord senderPosition = (*iter).second.position;
            double distance = currentPosition.distance(senderPosition);
            //if distance > PERRange, do not use it
            if (distance > vPERRange) {

                iter->second.useInCalc = false;
            }
        }
    }

    /* taken from SAE J2945, Page 67
     *
     * The number of expected BSMs for a given RV is 1 plus the difference between
     the values of DE_MsgCount in the last and first BSMs received from that RV within PERCalculationIntervaWAVE. The number of missed BSMs
     for the RV is the difference between the number of expected BSMs and the number of received BSMs from the RV
     within  PERCalculationIntervaWAVE.*/
    //TODO last step, calc PER for each RV with which useInCalcFlag is true --> DONE
    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {

        if (iter->second.useInCalc) {
            double numberOfExpectedBSM = 1
                    + (iter->second.lastmsgcount
                            - iter->second.msgcountFromFirstRecMsg);
            double numberOfMissedBSM = numberOfExpectedBSM
                    - iter->second.receivedMsgs;

            double per = numberOfMissedBSM / numberOfExpectedBSM;

            perVec.record(per);

            iter->second.perCalculated = per;

            if (testCaseNumber == 3.3 || testCaseNumber == 3.6) {
                ASSERT(per = 0.75);
            } else if (testCaseNumber == 3.4) {
                ASSERT(per = 0.81);
            }
        }
    }
}

/*taken from SAE J2945, Page 67
 *
 * Channel  Quality  Indicator  (is  calculated  at  the  end  of  PERCalculationSubInterval
 *  as  the  average  of  PER(k)  for  all  RVs  within vPERRange and for which PER(k) is calculated, with the following constraint:
 * if(channelQualityIndicator > vPERMax)
 *    channelQualitIndicator = vPERMax
 */
double PERAndChannelQualityIndicatorCalculator::calculateChannelQualityIndicator() {

    std::map<std::string, carinfo>::iterator iter;
    double counterMsgInUse = 0.0;
    double sumPERValues = 0.0;

    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {

        if (iter->second.useInCalc) {

            //count all cars
            counterMsgInUse++;
            //sum up all the PERs
            sumPERValues += iter->second.perCalculated;
        }
    }

    double channelQualityIndicator = sumPERValues / counterMsgInUse;

    if (counterMsgInUse == 0.0) {
        channelQualityIndicator = 0.0;
    } else if (channelQualityIndicator > vPERMax) {
        channelQualityIndicator = vPERMax;
    }

    return channelQualityIndicator;
}

/*taken from SAE J2945, page 67
 *
 * Vehicle Density in Range (N): The HV (System) calculates N(k) at the end of w k  as the number of unique RVs within
 * vPERRange (an RV is determined to be unique if it has a unique DE_TemporaryID included in its BSM).
 *
 */
int PERAndChannelQualityIndicatorCalculator::calculateVehicleDensityInRange(
        Coord currentPosition, double testCaseNumber) {

    std::map<std::string, carinfo>::iterator iter;
    simtime_t currentTime = simTime();
    int numberOfVehicles = 0;

    //tests
    if (testCaseNumber >= 3 && testCaseNumber < 4) {
        Coord temp(1, 1, 1);
        currentPosition = temp;
        currentTime = SimTime(6);
        performTestCase(testCaseNumber);
    }

    for (iter = manageMap.begin(); iter != manageMap.end(); iter++) {

        //first check--> arrival time within vPERSubInterval?

        if (currentTime - iter->second.lastarrivaltime <= vPERSubInterval) {

            // second check --> distance within vPERRange
            if (currentPosition.distance(iter->second.position) <= vPERRange) {
                numberOfVehicles++;
            }
        }
    }

    return numberOfVehicles;
}

//prepares the managemap for testcases
void PERAndChannelQualityIndicatorCalculator::performTestCase(
        double testCaseNumber) {

    //Period 1 - 6
    if (testCaseNumber == 3.1) {

        //Appendix A.8.3 in SAEJ2945 example 1--> arrivalTime is 0.5 --> is not within vPERInterval
        Coord positionOne(2, 1, 1);
        carinfo one = { "one", 8, 2, SimTime(0.5), false, positionOne, false, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;

    } else if (testCaseNumber == 3.2) {

        //Described in Appendix A.8.3 in SAEJ2945 example 1--> PER is 0.75, but not within vPERInterval --> PER is undefined
        Coord positionOne(2, 1, 1);
        carinfo one = { "one", 8, 2, SimTime(1.5), false, positionOne, true, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;

    } else if (testCaseNumber == 3.3) {

        //Described in Appendix A.8.3 in SAEJ2945 example 1--> PER is 0.75 and within vPERInterval
        Coord positionOne(2, 1, 1);
        carinfo one = { "one", 8, 2, SimTime(5.5), false, positionOne, true, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;

    } else if (testCaseNumber == 3.4) {

        //Described in Appendix A.8.3 in SAEJ2945 example 2a--> PER is 0.81 and within vPERInterval, arrivalTime is withing vPERSubInterval (this differs from the original example)
        Coord positionOne(2, 1, 1);
        carinfo one = { "one", 11, 2, SimTime(5.5), false, positionOne, true, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;

    } else if (testCaseNumber == 3.5) {

        //Car is out of Range, example 2b
        Coord positionOne(205, 1, 1);
        carinfo one = { "one", 11, 2, SimTime(5.5), false, positionOne, true, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;

    } else if (testCaseNumber == 3.6) {

        //two cars in map --> both 0.75 PER
        Coord positionOne(2, 1, 1);
        carinfo one = { "one", 8, 2, SimTime(5.5), false, positionOne, true, 1,
                -1 };
        Coord positionTwo(3, 1, 1);
        carinfo two = { "two", 8, 2, SimTime(5.5), false, positionOne, true, 1,
                -1 };

        manageMap.clear();
        manageMap[one.carId] = one;
        manageMap[two.carId] = two;

    }

}

