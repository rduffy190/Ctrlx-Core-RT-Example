/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <map>

#include "common/scheduler/i_scheduler3.h"
#include "comm/datalayer/datalayer.h"
#include <common/scheduler/state2_generated.h>

namespace sdk_rt
{
  //! Enum of possible states of the final state machine to handle the none realtime actions of the callable.
  enum FinalStateMachineState
  {
    UNDEFINED = 0,
    IDLE,
    ERROR,
    UNKNOWN,
    SCHED_SETUP,
    SCHED_SETUP_PREPARE,
    SCHED_SERVICE,
    SCHED_OPERATING_PHASE_NONE,
    SCHED_OPERATING_PHASE_BEGIN,
    SCHED_OPERATION_PHASE_EXECUTE,
    SCHED_OPERATING_PREPARE,
    SCHED_OPERATING_PREPARE_READ_ECAT_IN,
    SCHED_OPERATING_PREPARE_READ_ECAT_OUT,
    SCHED_OPERATING_PREPARE_READ_PLC_IN,
    SCHED_OPERATING_PREPARE_READ_PLC_OUT, 
    SCHED_OPERATING,
    TASK_PROPERTIES_CHANGE,
    SCHED_EXIT
  };

  //! States as text
  static std::string s_states[] =
      {
          "UNDEFINED",
          "IDLE",
          "ERROR",
          "UNKNOWN",
          "SCHED_SETUP",
          "SCHED_SERVICE_PREPARE",
          "SCHED_SERVICE",
          "SCHED_OPERATING_PHASE_NONE",
          "SCHED_OPERATING_PHASE_BEGIN",
          "SCHED_OPERATION_PHASE_EXECUTE",
          "SCHED_OPERATING_PREPARE",
          "SCHED_OPERATING_PREPARE_READ_ECAT_IN",
          "SCHED_OPERATING_PREPARE_READ_ECAT_OUT",
          "SCHED_OPERATING_PREPARE_READ_PLC_IN",
          "SCHED_OPERATING_PREPARE_READ_PLC_OUT", 
          "SCHED_OPERATING",
          "TASK_PROPERTIES_CHANGE",
          "SCHED_EXIT"};


  //! The callable class, implementing the interface ICallable
  class RealTimeTemplateCallable : public common::scheduler::ICallable
  {
  private:
    int m_instanceID;
    std::thread m_thread;

    bool m_rtErrorTraced = false;

    comm::datalayer::IDataLayerFactory *m_datalayerFactoryPtr = nullptr;

    comm::datalayer::IClient *m_datalayerClientPtr = nullptr;

    std::shared_ptr<comm::datalayer::IMemoryUser> m_memoryUserIn;
    uint32_t m_revisionIn;
    std::map<std::string,uint32_t>m_ecatInMap;

    std::shared_ptr<comm::datalayer::IMemoryUser> m_memoryUserOut;
    uint32_t m_revisionOut;
    std::map<std::string,uint32_t>m_ecatOutMap; 

    std::shared_ptr<comm::datalayer::IMemoryUser> m_memoryUserPlcIn;
    uint32_t m_plcRevisionIn;
    std::map<std::string,uint32_t>m_plcInMap; 

    std::shared_ptr<comm::datalayer::IMemoryUser> m_memoryUserPlcOut;
    uint32_t m_plcRevisionOut;
    std::map<std::string,uint32_t>m_plcOutMap; 

    common::scheduler::SchedEventType m_schedEventType;

    sdk_rt::FinalStateMachineState m_finalStateMachineState = sdk_rt::FinalStateMachineState::IDLE;

    //! Current state of this callable.
    common::scheduler::fbs2::CurrentState m_fbsCurrentState;

    void finalStateMachine();

    bool readMemoryMap(
      const std::string &address,
      std::shared_ptr<comm::datalayer::IMemoryUser> &memoryUser,
      std::map<std::string,uint32_t> &map,
      uint32_t &revision);

    void readDiWriteDo();

    common::scheduler::SchedEventResponse execute(const common::scheduler::SchedEventType &eventType, const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param) final;
    common::scheduler::SchedEventResponse onEventTick(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onSwitchToExit(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onTaskPropertiesChange(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onGetCurrentState(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onSwitchToOperating(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onSwitchToSetup(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    common::scheduler::SchedEventResponse onSwitchToService(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param);
    
    uint16_t m_tick; 
    bool m_outValue;
  public:
    void setDataLayerFactory(comm::datalayer::IDataLayerFactory *datalayerFactoryPtr);
    
    void finalStateMachineStart();

    void finalStateMachineStop();
  };
}
