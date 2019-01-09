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

package org.car2x.veins.modules.mac.ieee80211p;

import org.car2x.veins.base.modules.BaseMacLayer;

//
// Manages timeslots for CCH and SCH listening and sending.
//
// @author Christopher Saloman
// @author David Eckhoff : rewrote complete model
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see PhyLayer80211p
//
simple Mac1609_4 extends BaseMacLayer
{
    parameters:
        @class(Mac1609_4);

        // cycle between an SCH and the CCH (or stay on CCH all the time)
        bool useServiceChannel = default(false);

        // the service channel this maclayer listens on
        int serviceChannel = default(1);

        // debug switch
        bool debug = default(false);

        // length of MAC header, header is already added at 1609_4
        headerLength @unit(bit) = default(0 bit);

        // bit rate 
        double bitrate @unit(bps) = default(18 Mbps);
        double bitrateWAVE @unit(bps);

        // little asynchronization between cars
        double syncOffset @unit(s) = default(0.0003s);

        //tx power [mW]
        double txPower @unit(mW);
        double txPowerWAVE @unit(dBm);

        //ETSI
        double NDL_defTxPower @unit(dBm);
        double NDL_minTxPower @unit(dBm);
        double NDL_maxTxPower @unit(dBm);
        double NDL_activeTxPower @unit(dBm);
        bool useDCCProfileCAM = default(false);
        bool useGateKeeping = default(false);
        double NDL_maxPacketIntervalDCCProfileRELAXED @unit(s) = default(0.095s);
        double NDL_maxPacketIntervalDCCProfileACTIVE @unit(s) = default(0.190s);
        double NDL_maxPacketIntervalDCCProfileRESTRICTIVE @unit(s) = default(0.250s);

        double NDL_defDatarate @unit(bps);
        double NDL_minDatarate @unit(bps);
        double NDL_maxDatarate @unit(bps);

        double NDL_defPacketInterval @unit(s);
        double NDL_minPacketInterval @unit(s);
        double NDL_maxPacketInterval @unit(s);
        double NDL_maxPacketDuration @unit(s);

        double NDL_timeUp @unit(s) = default(1s);
        double NDL_timeDown @unit(s) = default(5s);
        double channelLoadTimer @unit(s);
        double NDL_defCarrierSense @unit(dBm);
        double NDL_minCarrierSense @unit(dBm);
        double NDL_maxCarrierSense @unit(dBm);
        
        double NDL_minChannelLoad;
        double NDL_maxChannelLoad;
        
        double NDL_queueLength = default(2);//NDL_queueLength, see TS 102 687 V1.1.2, Table A.9


        int queueSize = default(0); 
        // signal informing interested application about channel busy state
        @signal[sigChannelBusy](type=bool);
        // signal informing interested application about a collision
        @signal[sigCollision](type=bool);

        

}
