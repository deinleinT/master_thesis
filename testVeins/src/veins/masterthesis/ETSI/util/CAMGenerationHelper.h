/*
 * CAMGenerationHelper.h
 *
 *  Created on: 01.03.2017
 *      Author: thoma
 */

#ifndef SRC_VEINS_MASTERTHESIS_ETSI_UTIL_CAMGENERATIONHELPER_H_
#define SRC_VEINS_MASTERTHESIS_ETSI_UTIL_CAMGENERATIONHELPER_H_

#include <assert.h>
#include <math.h>
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Move.h"

class CAMGenerationHelper {
public:
    static void performTests() {

        Coord one(1, 3, -2);
        Coord two(-1, 4, 3);
        double value = getDirectionDifAngle(one, two);
        ASSERT(value >= 74.0 && value <= 75.0);

        Coord three(1, 5, 0);
        Coord four(3, 7, 0);
        double value2 = getDirectionDifAngle(three, four);
        ASSERT(value2 >= 11.8 && value2 <= 11.9);

        Coord five(2, 9, 5);
        Coord six(6, -2, 4);
        double value3 = getDirectionDifAngle(five, six);
        ASSERT(value3 >= 79.7 && value3 <= 79.8);

        Move oneM;
        Move twoM;
        oneM.setSpeed(25);
        twoM.setSpeed(20);
        double dif = getSpeedDifference(oneM, &twoM);
        ASSERT(dif == 5);

        Coord a(1, 0, 0);
        Coord b(0, 0, 0);
        double difPos = getPositionDif(a, b);
        ASSERT(difPos == 1);
    }
    ;

    static double getSpeedDifference(Move & curMove, Move * lastSentCAMMove) {

        return fabs(curMove.getSpeed() - lastSentCAMMove->getSpeed());
    }
    ;

    static double getDirectionDifAngle(const Coord & posOne,
            const Coord & posTwo) {

        double dot = posOne.x * posTwo.x + posOne.y * posTwo.y
                + posOne.z * posTwo.z;
        double lengthOne = posOne.length();
        double lengthTwo = posTwo.length();
        double cosAngle = dot / (lengthOne * lengthTwo);
        double value = acos(cosAngle) * 180 / M_PI;

        return value;

    }
    ;

    static double getPositionDif(Coord & one, Coord & two) {

        return one.distance(two);

    }

};

#endif /* SRC_VEINS_MASTERTHESIS_ETSI_UTIL_CAMGENERATIONHELPER_H_ */
