/*
 * CBRValues.h
 *
 *  Created on: 14.03.2017
 *      Author: thoma
 */

#ifndef SRC_VEINS_MASTERTHESIS_ETSI_DCC_CHANNELLOAD_CBRVALUES_H_
#define SRC_VEINS_MASTERTHESIS_ETSI_DCC_CHANNELLOAD_CBRVALUES_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;

//See TS 102 635-4-2, Table 1
struct CBRValues {

    //TODO in listener!
    double CBR_L_0_Hop; //Local (measured) channel busy ratio, disseminated to neighbouring ITS-S as CBR_R_0_Hop

    //TODO when sending CAM!!!
    double CBR_R_0_Hop; //Disseminated local (measured) channel busy ratio (CBR_L_0_Hop), i.e. CBR_L_0_Hop becomes CBR_R_0_Hop when disseminated. At receiving ITS-S it becomes CBR_L_1_Hop.


    //TODO max CBR_R_0_HOP --> in Traci, read CBR_R_0_Hop from CAM and Save the max value
    double CBR_L_1_Hop; //CBR_L_1_Hop is the maximum CBR_R_0_Hop value received from a neighbouring ITS-S in a given T_cbr interval, i.e. it is the 1-hop channel busy ratio. It is subsequently disseminated to  neighbours as CBR_R_1_Hop

    double CBR_R_1_Hop; //Disseminated 1-hop channel busy ratio (CBR_L_1_Hop), i.e. CBR_L_1_Hop becomes CBR_R_1_Hop when disseminated. At receiving ITS-S it becomes CBR_L_2_Hop.

    double CBR_L_2_Hop; // CBR_L_2_Hop is the maximum CBR_R_1_Hop value received from a neighbouring ITS-S in a given T_cbr interval, i.e. it is the 2-hop channel busy ratio. It is never disseminated by ego ITS-S

    double CBR_G; //Global channel busy ratio at ego ITS-S, used in the DCC algorithm (maximum over CBR_L_0_Hop, CBR_L_1_Hop and CBR_L_2_Hop).

};

#endif /* SRC_VEINS_MASTERTHESIS_ETSI_DCC_CHANNELLOAD_CBRVALUES_H_ */
