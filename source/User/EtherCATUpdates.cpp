#include "EtherCATUpdates.h"
#include "../impl/Logger.h"

long int m_ticks = 0;    
int16_t DigitalOutputs = 0xFF;
Drive axis1;

namespace EtherCATUpdate{
    void MDT(u_int8_t* outData, std::map<std::string,uint32_t> m_outMap)
    {
        m_ticks++; 

        //turn on/off some outputs just to show how it is done
        if(0 == m_ticks%500)
        {
            DigitalOutputs = DigitalOutputs ^ 0xFFFF;
        }
        //Enable the only drive  
        //Check to see if the drive is in Ab and Error free
        LOG_INFO("Ab State: %i", axis1.StatusWord & ST_DriveInAb);             
        LOG_INFO("Erorr State: %i", axis1.StatusWord & ST_DriveError);             
        if (((axis1.StatusWord & ST_DriveInAb) != 0) && ((axis1.StatusWord & ST_DriveError) == 0)) 
        {
            axis1.ControlWord = axis1.ControlWord | (CMD_DriveEnable | CMD_DriveHALT | CMD_DriveON); //Enable bits 15,14,13 to enable (0xE000). 
            axis1.ControlWord = (axis1.ControlWord & CMD_ClearOpMode); //Reset the operation mode
            axis1.ControlWord = (axis1.ControlWord | CMD_SecondaryOpMode); //Set the secondary operation mode            
        }
        else
        {   //if there is an error or if the drive is not ready then clear the control bits
            axis1.ControlWord = axis1.ControlWord & 0x1FFF; 
        }
        axis1.ControlWord = axis1.ControlWord ^ CMD_CommsToggle; //toggle the control bit, the 10th bit in the control word
        //copy over the IO
        std::memcpy(&outData[m_outMap["DO_16_1/Channel_1.Value"]/8], &DigitalOutputs, 2); 
        //copy over the control word
        LOG_INFO("Control Word: %i", axis1.ControlWord);  
        std::memcpy(&outData[m_outMap["Axis1/MDT.Master_control_word"]/8], &axis1.ControlWord, 2); 
        //copy over the velocity commands
        //int32_t Velocity = 300000;
        axis1.CMDVelocity = 300000;
        std::memcpy(&outData[m_outMap["Axis1/MDT.VelocityCommand"]/8], &axis1.CMDVelocity, 4);
    }

    void AT(u_int8_t* inData, std::map<std::string,uint32_t> m_inMap)
    {
        LOG_INFO("Writing"); 
        //Read in the status word
        axis1.StatusWord = *(int16_t*)(&inData[m_inMap["Axis1/AT.Drive_status_word"]/8]);         
        axis1.ActPosition = *(int32_t*)(&inData[m_inMap["Axis1/AT.Position_feedback_value_1"]/8]);         
        LOG_INFO("Status Word: %i, Actual Position: %i", axis1.StatusWord, axis1.ActPosition); 
    }
}
