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

cplusplus {{
#include "veins\modules\messages\WaveShortMessage_m.h"
#include "veins\masterthesis\ETSI\message\headers\CamParameters.h"
}}


packet WaveShortMessage;
struct CamParameters;

//
// see ETSI EN 302 637-2 V1.3.2, Page 23 / 24, Annex A
//
packet CooperativeAwarenessMessage extends WaveShortMessage {
    
    string itsPDUHeader;
    simtime_t generationTime;
    CamParameters camParameters;
    
    string carId; // 
    simtime_t timeOfPositionUpdate = 0;
    double CBR_R_0_Hop;
    double CBR_R_1_Hop; //Disseminated 1-hop channel busy ratio (CBR_L_1_Hop), i.e. CBR_L_1_Hop becomes  CBR_R_1_Hop when disseminated. At receiving ITS-S it becomes CBR_L_2_Hop.
    
}
