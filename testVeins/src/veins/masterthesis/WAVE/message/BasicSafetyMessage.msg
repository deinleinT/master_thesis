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
#include "veins\masterthesis\WAVE\message\headers\TRANSITION_STATE.h"
#include "veins\masterthesis\WAVE\message\headers\PositionalAccuracy.h"
#include "veins\masterthesis\WAVE\message\headers\AccelerationSet4Way.h"
#include "veins\masterthesis\WAVE\message\headers\BrakeSystemStatus.h"
#include "veins\masterthesis\WAVE\message\headers\VehicleSize.h"
}}

packet WaveShortMessage;
enum TRANSITION_STATE;
struct PositionalAccuracy;
struct AccelerationSet4Way;
struct BrakeSystemStatus;
struct VehicleSize;

//
// TODO generated message class
//
packet BasicSafetyMessage extends WaveShortMessage {
    int MSG_COUNT;
    string TEMP_ID;
    uint16_t SEC_MARK;
    int LAT;
    int LON;
    int ELEVATION;
    PositionalAccuracy ACCURACY;
    int SPEED;
    uint16_t HEADING;
    int ANGLE; 
    int TRANSITION_STATE @enum(TRANSITION_STATE);
    AccelerationSet4Way ACCEL_SET;
    BrakeSystemStatus BRAKESYSTEMSTATUS;
    VehicleSize SIZE;
    // added to find out from which node this BSM comes, not specified in SAE J2945
    string carId; // 
    simtime_t timeOfPositionUpdate = 0;
}
