/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#include "trace/trace.h"
#include "common/scheduler/callable_configurations_generated.h"
#include "realtime_template_callable_factory.h"

//! Scans the value out of a string expression 'key=value'
std::string scanValue(const char *expression, const char *key)
{
  std::string value = "";

  if (std::strstr(expression, key) != NULL)
  {
    auto p = std::strstr(expression, "=");
    if (p != NULL)
    {
      value = p + 1;
    }
  }

  return value;
}

namespace sdk_rt
{

  //! The constructor of the class.
  RealTimeTemplateCallableFactory::RealTimeTemplateCallableFactory()
  {
    TRACE_INFO(" ");
  }

  void RealTimeTemplateCallableFactory::destroyAllCallables()
  {
    TRACE_INFO(" ");

    m_callable->finalStateMachineStop(); 
  }

  //! Called by the scheduler service, scans the parameters in 'param' and returns a new callable instance.
  std::shared_ptr<common::scheduler::ICallable> RealTimeTemplateCallableFactory::createCallable(const comm::datalayer::Variant &param)
  {
    TRACE_INFO(" ");
    m_callable->setDataLayerFactory(m_datalayerFactoryPtr); 
    m_callable->finalStateMachineStart();
    return m_callable;
  }

  //! Deletes a callable instance.
  common::scheduler::SchedStatus RealTimeTemplateCallableFactory::destroyCallable(const std::shared_ptr<common::scheduler::ICallable> &callable)
  {
    TRACE_INFO(" ");
    std::shared_ptr<RealTimeTemplateCallable> t_callable = std::static_pointer_cast<RealTimeTemplateCallable>(callable); 
    t_callable->finalStateMachineStop();
    return common::scheduler::SchedStatus::SCHED_S_OK;
  }

  //! Returns default parameters for auto instanceating of a callable after installation and for a new manually created instance.
  common::scheduler::SchedStatus RealTimeTemplateCallableFactory::getCallableArguments(std::vector<std::string> &arguments)
  {
    TRACE_INFO(" ");

    return common::scheduler::SchedStatus::SCHED_S_OK;
  }

  //! Returns configuration info for an auto generated task.
  void RealTimeTemplateCallableFactory::getCallableConfigurations(comm::datalayer::Variant &configurations) const
  {
   ;
  }

  //! Provides information about the scheduler callable.
  void RealTimeTemplateCallableFactory::getFactoryDescription(std::vector<std::string> &description) const
  {
    
  }

  void RealTimeTemplateCallableFactory::setDatalayerFactory(comm::datalayer::IDataLayerFactory *service)
  {
    TRACE_INFO(" ");
    m_datalayerFactoryPtr = service;
    m_callable->setDataLayerFactory(service); 
  }

}