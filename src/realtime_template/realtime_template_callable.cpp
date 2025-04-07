/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include <common/scheduler/task_generated.h>
#include "trace/trace.h"
#include "realtime_template_callable.h"

namespace sdk_rt
{
  void RealTimeTemplateCallable::setDataLayerFactory(comm::datalayer::IDataLayerFactory *datalayerFactoryPtr)
  {
    m_datalayerFactoryPtr = datalayerFactoryPtr; 
  }

  //! Starts the final state machine in a separate thread.
  void RealTimeTemplateCallable::finalStateMachineStart()
  {
    TRACE_INFO("#%d", m_instanceID);

    m_finalStateMachineState = FinalStateMachineState::IDLE;
    m_thread = std::thread(&RealTimeTemplateCallable::finalStateMachine, this);
  }

  // Stops the final state machine thread.
  void RealTimeTemplateCallable::finalStateMachineStop()
  {
    TRACE_INFO("#%d", m_instanceID);

    m_finalStateMachineState = FinalStateMachineState::SCHED_EXIT;
    if (m_thread.joinable())
    {
      m_thread.join();
    }
    TRACE_INFO("#%d stopped.", m_instanceID);
  }

  //! Called by the scheduler service, handles all reported events.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::execute(
      const common::scheduler::SchedEventType &eventType,
      const common::scheduler::SchedEventPhase &eventPhase,
      comm::datalayer::Variant &param)
  {

    // See /home/boschrexroth/git/sdk.control/public/include/common.scheduler/common/scheduler/i_scheduler.h

    m_schedEventType = eventType;

    if (eventType != common::scheduler::SchedEventType::SCHED_EVENT_TICK)
    {
      TRACE_INFO("#%d eventType=%s, eventPhase=%s ", m_instanceID, common::scheduler::getSchedEventTypeAsString(eventType).c_str(), common::scheduler::getSchedEventPhaseAsString(eventPhase).c_str());
    }

    switch (eventType)
    {
    case common::scheduler::SchedEventType::SCHED_EVENT_TICK:
      return onEventTick(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_EXIT:
      return onSwitchToExit(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_TASK_PROPERTIES_CHANGE:
      return onTaskPropertiesChange(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_GET_CURRENT_STATE:
      return onGetCurrentState(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_OPERATING:
      return onSwitchToOperating(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SETUP:
      return onSwitchToSetup(eventPhase, param);

    case common::scheduler::SchedEventType::SCHED_EVENT_SWITCH_TO_SERVICE:
      return onSwitchToService(eventPhase, param);

    default:
      TRACE_WARN("#%d UNHANDLED eventType=%s", m_instanceID, common::scheduler::getSchedEventTypeAsString(m_schedEventType).c_str());
    }

    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! The cyclic task executed each cycle.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onEventTick(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    if (m_finalStateMachineState == FinalStateMachineState::SCHED_OPERATING)
    {
      readDiWriteDo();
      
    }

    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling on switch to EXIT state.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onSwitchToExit(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {

    if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
    {
      m_finalStateMachineState = FinalStateMachineState::SCHED_EXIT;
      m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_EXIT;
    }

    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling if task properties were changed.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onTaskPropertiesChange(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    if (comm::datalayer::DlResult::DL_OK != param.verifyFlatbuffers(common::scheduler::fbs::VerifyTaskBuffer))
    {
      TRACE_ERR("#%d Invalid param flatbuffers", m_instanceID);
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_ERROR;
    }

    const common::scheduler::fbs::Task *poTask = common::scheduler::fbs::GetTask(param.getData());
    if (nullptr == poTask)
    {
      TRACE_ERR("#%d GetTask failed", m_instanceID);
      return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_ERROR;
    }

    TRACE_INFO("#%d Affinity=%d Priority=%d Cycle time=%d Stacksize=%d", m_instanceID, (uint32_t)poTask->affinity(), (uint32_t)poTask->priority(), (uint32_t)poTask->cycletime(), (uint32_t)poTask->stacksize());
    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling if current state were changed.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onGetCurrentState(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    flatbuffers::FlatBufferBuilder builder;
    auto finished = common::scheduler::fbs2::CreateState(builder, m_fbsCurrentState);
    builder.Finish(finished);
    param.copyFlatbuffers(builder);
    return ::common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling on switch to OPERATING state.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onSwitchToOperating(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    switch (eventPhase)
    {
    case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_NONE:
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PHASE_NONE;
      break;
    case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_BEGIN:
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PHASE_BEGIN;
      break;

    case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_EXECUTE:
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATION_PHASE_EXECUTE;
      break;

    case common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END:
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
      m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_OPERATING;
      break;

    default:
      TRACE_WARN("#%d UNHANDLED eventPhase=%s ", m_instanceID, common::scheduler::getSchedEventPhaseAsString(eventPhase).c_str());
      break;
    }

    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling on switch to SERVICE state.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onSwitchToService(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
    {
      m_finalStateMachineState = FinalStateMachineState::SCHED_SERVICE;
      m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_SERVICE;
    }
    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Special event handling on switch to SETUP state.
  common::scheduler::SchedEventResponse RealTimeTemplateCallable::onSwitchToSetup(const common::scheduler::SchedEventPhase &eventPhase, comm::datalayer::Variant &param)
  {
    if (eventPhase == common::scheduler::SchedEventPhase::SCHED_EVENT_PHASE_END)
    {
      m_fbsCurrentState = common::scheduler::fbs2::CurrentState::CurrentState_SETUP;
      m_finalStateMachineState = FinalStateMachineState::SCHED_SETUP_PREPARE;
    }

    return common::scheduler::SchedEventResponse::SCHED_EVENT_RESP_OKAY;
  }

  //! Implementation of the final state machine, handles none realtime actions (outside the scheduler context).
  void RealTimeTemplateCallable::finalStateMachine()
  {
    char counter = 0;
    while (true)
    {
      counter = (++counter) & 7;
      TRACE_INFO("%d #%d state=%s schedEventType=%s", counter, m_instanceID, s_states[m_finalStateMachineState].c_str(), common::scheduler::getSchedEventTypeAsString(m_schedEventType).c_str());

      switch (m_finalStateMachineState)
      {
        // Known states, but nothing to do
      case FinalStateMachineState::IDLE:
      case FinalStateMachineState::ERROR:
      case FinalStateMachineState::UNKNOWN:
      case FinalStateMachineState::SCHED_SERVICE:
      case FinalStateMachineState::SCHED_OPERATING_PHASE_NONE:
      case FinalStateMachineState::SCHED_OPERATING_PHASE_BEGIN:
      case FinalStateMachineState::SCHED_OPERATION_PHASE_EXECUTE:
      case FinalStateMachineState::TASK_PROPERTIES_CHANGE:
        break;

      case FinalStateMachineState::SCHED_EXIT:
        TRACE_INFO("Terminating thread #%d", m_instanceID);
        return; // Terminate thread

      case FinalStateMachineState::SCHED_SETUP_PREPARE:
      case FinalStateMachineState::SCHED_OPERATING_PREPARE:
        TRACE_INFO("Createing Data Layer Client");
        if (m_datalayerFactoryPtr != nullptr)
        {
          m_datalayerClientPtr = m_datalayerFactoryPtr->createClient();
        }

        if (m_datalayerClientPtr == nullptr)
        {
          TRACE_ERR("Creating Data Layer client failed.");
          break;
        }else{
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_ECAT_IN;
          break;
        }
      case FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_ECAT_IN:
        TRACE_INFO("Creating Ecat In");
        if (readMemoryMap("fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/input",m_memoryUserIn,m_ecatInMap,m_revisionIn))
        {
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_ECAT_OUT;
          break; 
        }else
        {
          TRACE_ERR("Ecat In Memory Open Failed"); 
          break; 
        }
      case FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_ECAT_OUT: 
        TRACE_INFO("Creating Ecat OUT");
        if(readMemoryMap("fieldbuses/ethercat/master/instances/ethercatmaster/realtime_data/output",m_memoryUserOut,m_ecatOutMap,m_revisionOut))
        {
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_PLC_IN;
          break;
        }else{
          TRACE_ERR("Ecat Out Memory Open Failed")
          break; 
        }
      case FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_PLC_IN:  
        TRACE_INFO("Creating PLC In");
        if(readMemoryMap("plc/app/Application/realtime_data/PRG_RTDatalayer_RTDataIn",m_memoryUserPlcIn,m_plcInMap,m_plcRevisionIn))
        {
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_PLC_OUT; 
          break; 
        }else{
          TRACE_ERR("PLC Memory In Failure"); 
          break; 
        }
      case FinalStateMachineState::SCHED_OPERATING_PREPARE_READ_PLC_OUT:
        TRACE_INFO("Creating PLC Out");
        if(readMemoryMap("plc/app/Application/realtime_data/PRG_RTDatalayer_RTDataOut",m_memoryUserPlcOut,m_plcOutMap,m_plcRevisionOut))
        {
          // Enable Reading DI, writing DO
          m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING;
          break; 
        }else 
        {
          TRACE_ERR("PLC Memory Out Failure"); 
          break; 
        }
      case FinalStateMachineState::SCHED_SETUP:
      case FinalStateMachineState::SCHED_OPERATING:
        // execute() will call executeSchedulerEventTickReadDiWriteDo()
        break;

      default:
        TRACE_WARN("Unhandled FinalStateMachineState %d", m_finalStateMachineState);
        break;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  bool RealTimeTemplateCallable::readMemoryMap(
      const std::string &address,
      std::shared_ptr<comm::datalayer::IMemoryUser> &memoryUser,
      std::map<std::string,uint32_t> &map,
      uint32_t &revision
      )
  {
    comm::datalayer::Variant mapVariant;
    auto result = m_datalayerClientPtr->readSync(address + "/map", &mapVariant);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_ERR("readSync(%s) failed: %s", address.c_str(), result.toString());
      return false;
    }

    result = m_datalayerFactoryPtr->openMemory(memoryUser, address);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_ERR("openMemory(%s) failed: %s", address.c_str(), result.toString());
      return false;
    }

    int tries = 10;
    do
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      result = memoryUser->getMemoryMap(mapVariant);
      if (comm::datalayer::STATUS_FAILED(result))
      {
        tries--;
        if (tries <= 0)
          break;
      }
    } while (comm::datalayer::STATUS_FAILED(result));
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_ERR("getMemoryMap() %s failed: %s", address.c_str(), result.toString());
      m_datalayerFactoryPtr->closeMemory(memoryUser);
      return false;
    }

    result = mapVariant.verifyFlatbuffers(comm::datalayer::VerifyMemoryMapBuffer);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_ERR("Verifing map %s failed: %s", address.c_str(), result.toString());
      m_datalayerFactoryPtr->closeMemory(memoryUser);
      return false;
    }

    auto memoryMap = comm::datalayer::GetMemoryMap(mapVariant.getData());
    revision = memoryMap->revision();
    bool found = false;
    for (auto variable = memoryMap->variables()->begin(); variable != memoryMap->variables()->end(); variable++)
    { 
      map.emplace(variable->name()->str(),variable->bitoffset()); 
    }
    return true;
  }

  void RealTimeTemplateCallable::readDiWriteDo()
  {
    // Take time stamp here to measure scheduler deviation
    // https://stackoverflow.com/questions/18022927/convert-high-resolution-clock-time-into-an-integer-chrono
    // auto timePoint = std::chrono::high_resolution_clock::now();

    // Read Input ----------------------------------------------

    uint8_t *dataIn;
    uint8_t *dataOut; 
    uint8_t *PlcIn; 
    uint8_t *PlcOut; 
    uint16_t plcReply; 
    m_tick = (m_tick + 1)%500; 
    if (m_tick == 0)
    {
      m_outValue = !m_outValue;
    }
    auto result = m_memoryUserIn->beginAccess(dataIn, m_revisionIn);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_INFO("Failed Open ECAT Mem In"); 
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
    }
    m_memoryUserIn->endAccess(); 
    result = m_memoryUserOut->beginAccess(dataOut, m_revisionOut);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_INFO("FAile Open ECAT Mem Out ")
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
    }else{
      if (m_outValue)
      {
        dataOut[m_ecatOutMap["XI211208/Channel_1.Value"]/8] = 255; 
      }else{
        dataOut[m_ecatOutMap["XI211208/Channel_1.Value"]/8] = 0; 
      }
    }
    m_memoryUserOut->endAccess(); 
    result = m_memoryUserPlcOut->beginAccess(PlcOut, m_plcRevisionOut);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_INFO("Failed Open Plc Out"); 
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
    }else{
      plcReply = *(uint16_t*)&PlcOut[m_plcOutMap["TestOut"]/8]; //lol// 
    }
    m_memoryUserPlcOut->endAccess(); 
    result = m_memoryUserPlcIn->beginAccess(PlcIn, m_plcRevisionIn);
    if (comm::datalayer::STATUS_FAILED(result))
    {
      TRACE_INFO("Failed Open Plc In"); 
      m_finalStateMachineState = FinalStateMachineState::SCHED_OPERATING_PREPARE;
    }else{
      *(uint16_t*)&PlcIn[m_plcInMap["TestIn"]/8] = plcReply; 
    }
    m_memoryUserPlcIn->endAccess(); 
  }
}