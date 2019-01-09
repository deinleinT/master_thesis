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

package org.car2x.veins.modules.application.ieee80211p;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// WAVE application layer base class.
//
// @author David Eckhoff
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple BaseWaveApplLayer like IBaseApplLayer
{
    parameters:
        bool debug = default(false); //whether to print debug output
        int headerLength @unit(bit) = default(88bit); //header length of the application
        int beaconLengthBits @unit(bit) = default(4664bit); //the length of a beacon packet

        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        int beaconPriority = default(3); //the access category of the beacon messages
        volatile double beaconInterval @unit(s) = default(1s); //the intervall between 2 beacon messages
        // added WAVE
        
        volatile double vCBPMeasInt @unit(s) = default(0.1s);
        volatile double vPERSubInterval @unit(s) = default(1s);
        volatile double vPERInterval @unit(s) = default(5s);
        volatile double vTxCntrlInt @unit(s) = default(0.1s);
        double vPERRange = default(100.0);
        double vPERMax = default(0.3);
        double vCBPWeightFactor = default(0.5);
        volatile double vHVLocalPosEstIntMin @unit(s) = default(0.05s);// used in TrackingErrorCalculator
        volatile double vHVLocalPosEstIntMax @unit(s) = default(0.15s); // used in TrackingErrorCalculator
        volatile double vHVRemotePosEstIntMin @unit(s) = default(0.05s); // used in TrackingErrorCalculator
        volatile double vHVRemotePosEstIntMax @unit(s) = default(3s);// used in TrackingErrorCalculator
        double vTrackingErrMin = default(0.2); //in meters, used in TransmissionProbabilityCalculator
        double vTrackingErrMax = default(0.5); //in meters, used in TransmissionProbabilityCalculator
        double vErrSensitivity = default(75.0); // used in TransmissionProbabilityCalculator
        double vMax_ITT = default(600.0); // milliseconds, DONT use values like 0.6!!!
        double vDensityWeightFactor = default(0.05); //
        double vDensityCoefficient = default(25.0); //
        volatile double vRescheduleTh @unit(s) = default(0.025s); // used for transmission decission
        volatile double vTimeAccuracy @unit(s) = default(0.001s); // used for TransmissionDecison
        bool randomCriticalEvent = default(false);
        double vRPMax = default(20.0); // used in Radiated Power
        double vRPMin = default(10.0); // used in Radiated Power
        double vSUPRAGain = default(0.5); // used in Radiated Power
        double vMinChanUtil = default(0.5); // used in Radiated Power
        double vMaxChanUtil = default(0.5); // used in Radiated Power
        volatile double vTxRand @unit(s) = default(0.005s);
        double vRP = default(15.0); // used in Radiated Power
        double vMaxSuccessiveFail = default(3.0);
        volatile double sendDataInterval @unit(s);
        double vRxSens;
        //

        //
        bool calcVeh = default(true);
        //

		bool beaconETSI = default(false);
        bool beaconWAVE = default(false);

		//CAM is AC_BE, not AC_VO
        double beaconPriorityAC_BE = default(1);

        double maxOffset @unit(s) = default(0s); //the maximum delay of messages sent down to avoid synchronous channel access
        bool sendData = default(false); //whether this applayer should send data
        int dataLengthBits @unit(bit) = default(1024bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
