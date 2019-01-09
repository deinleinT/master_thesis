

#ifndef SRC_VEINS_MASTERTHESIS_ETSI_DCC_STATEMACHINE_STATES_H_
#define SRC_VEINS_MASTERTHESIS_ETSI_DCC_STATEMACHINE_STATES_H_

enum StateType {

    RELAXED, ACTIVE, RESTRICTIVE

};

struct State {

    State() {
    }
    State(StateType stateType, double txPower, uint64_t datarate,
            double packetInterval, double carrierSense) {
        this->stateType = stateType;
        this->txPower = txPower;
        this->datarate = datarate;
        this->packetInterval = packetInterval;
        this->carrierSense = carrierSense;
    }

    StateType stateType;
    double txPower;
    uint64_t datarate;
    double packetInterval;
    double carrierSense;

};

#endif /* SRC_VEINS_MASTERTHESIS_ETSI_DCC_STATEMACHINE_STATES_H_ */
