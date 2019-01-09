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

#ifndef SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_PER_PERANDCHANNELQUALITYINDICATORCALCULATOR_H_
#define SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_PER_PERANDCHANNELQUALITYINDICATORCALCULATOR_H_

#include <string>
#include <omnetpp.h>
#include <map>
#include <math.h>
#include <assert.h>
#include "veins/base/utils/Coord.h"

struct carinfo {
    std::string carId;
    int lastmsgcount;
    int receivedMsgs;
    omnetpp::simtime_t lastarrivaltime;
    bool firstMessage;
    Coord position;
    bool useInCalc;
    int msgcountFromFirstRecMsg;
    double perCalculated;
};

class PERAndChannelQualityIndicatorCalculator {

private:
    double vPERRange; //in m, vPERRange --> SAE J2945, maximum Range between Cars
    double vPERMax; // SAE J2945, maxValue used for ChannelQualityIndicator
    omnetpp::cSimpleModule * source;
    void performTestCase(double testCaseNumber);

protected:
    double vPERInterval;
    double vPERSubInterval;
    std::map<std::string, carinfo> manageMap;

public:
    PERAndChannelQualityIndicatorCalculator(omnetpp::cSimpleModule * source,
            double vPERInterval, double vPERSubInterval, double vPERRange,
            double vPERMax);
    virtual ~PERAndChannelQualityIndicatorCalculator();
    void receivedBSM(std::string carId, int msgCount,
            omnetpp::simtime_t arrivaltime, Coord position);
    void calculatePER(Coord currentPosition, simtime_t currentTime,
            cOutVector & perVec, double testCaseNumber,simtime_t & startTime);
    double calculateChannelQualityIndicator();
    int calculateVehicleDensityInRange(Coord currentPosition,
            double testCaseNumber);

};

#endif /* SRC_VEINS_MASTERTHESIS_WAVE_CALCULATION_PER_PERANDCHANNELQUALITYINDICATORCALCULATOR_H_ */
