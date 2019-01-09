/*
 * CamParameters.h
 *
 *  Created on: 24.02.2017
 *      Author: thoma
 */

#ifndef SRC_VEINS_MASTERTHESIS_ETSI_MESSAGE_HEADERS_CAMPARAMETERS_H_
#define SRC_VEINS_MASTERTHESIS_ETSI_MESSAGE_HEADERS_CAMPARAMETERS_H_

//see ETSI EN 302 637-2 V1.3.2, Page 23 / 24, Annex A

struct StationType {

};

struct BasicContainer{

    StationType stationType;
    //...

};

struct CamParameters{

    BasicContainer basicContainer;

};


#endif /* SRC_VEINS_MASTERTHESIS_ETSI_MESSAGE_HEADERS_CAMPARAMETERS_H_ */
