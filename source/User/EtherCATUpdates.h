#include <map> 
#include "comm/datalayer/datalayer.h"

namespace EtherCATUpdate
            {
            void MDT(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap);
            void AT(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap);            
            }
class Drive 
    {
    public:
        uint16_t ControlWord = 0x0100;
        int32_t CMDVelocity = 0;
        int32_t CMDPosition = 0;
        uint16_t StatusWord;
        int32_t ActVelocity;
        int32_t ActPosition;
    };

//control word constants
const uint16_t CMD_DriveON = 0x8000; //Drive on
const uint16_t CMD_DriveEnable = 0x4000; //Drive enable
const uint16_t CMD_DriveHALT = 0x2000; //Drive halt
const uint16_t CMD_CommsToggle = 0x0400; //Needs to toggle every cycle. bitwise XOR with the control word
const uint16_t CMD_ClearOpMode = 0xF4FF; //Bitwise AND with the control word
const uint16_t CMD_PrimaryOpMode = 0x0000; //Bitwise OR with the control word after reset
const uint16_t CMD_SecondaryOpMode = 0x0100; //Bitwise OR with the control word after reset

//status word constants
const uint16_t ST_DriveInAF = 0xC000; //Drive is in the AF state
const uint16_t ST_DriveInAb = 0x8000; //Drive is in the Ab State
const uint16_t ST_DriveError = 0x2000; //Drive has error
const uint16_t ST_DriveWarning = 0x1000; //Drive has warning

