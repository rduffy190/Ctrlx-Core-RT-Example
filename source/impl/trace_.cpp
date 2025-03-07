/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "trace.h"
#include "common/log/trace/log_buffered.h"

AUTOMATIONSCHEDULER_API automation::scheduler::RtTrace automation::scheduler::g_rt_trace;

automation::scheduler::BaseTrace::BaseTrace()
{
  std::string envSnapName = "SNAP_NAME";

  auto envVar = getenv(envSnapName.c_str());
  if (envVar)
  {
    m_snapName = envVar;
  }

  return;
}



void automation::scheduler::RtTrace::attachTrace(common::log::trace::IRegistrationRealTime2* traceRegistration)
{
  if (nullptr != traceRegistration)
  {
    if (m_rtTraceContext != &m_traceBuffered)
    {
      detachTrace(traceRegistration);
    }
    
    auto enablingStateCallback = [&m_traceEnablingState = this->m_traceEnablingState](const common::log::trace::EnablingState &enablingState)
    {
      m_traceEnablingState = enablingState;
    };

    common::log::trace::ILogRealTime* rtTraceContext = nullptr;
    common::log::trace::TraceResult traceResult = traceRegistration->registerUnit(&rtTraceContext, getTraceName(), m_traceEnablingState, enablingStateCallback);
    if (rtTraceContext && common::log::trace::statusSucceeded(traceResult))
    {
      m_rtTraceContext = rtTraceContext;
      TRACE_MSG("Register rt trace unit '%s' succeeded", getTraceName());
    }
    else
    {
      TRACE_ERR("Register rt trace unit '%s' failed ('0x%08X')", getTraceName(), traceResult);
    }
  }
}

void automation::scheduler::RtTrace::detachTrace(common::log::trace::IRegistrationRealTime2* traceRegistration)
{
  if (nullptr != traceRegistration)
  {
    common::log::trace::ILogRealTime* traceContext = m_rtTraceContext;
    if (traceContext != &m_traceBuffered)
    {
      m_rtTraceContext = &m_traceBuffered;
      common::log::trace::TraceResult traceResult = traceRegistration->unregisterUnit(&traceContext);
      if (common::log::trace::statusSucceeded(traceResult))
      {
        TRACE_MSG("Unregister rt trace unit '%s' succeeded", getTraceName());
      }
      else
      {
        TRACE_ERR("Unregister rt trace unit '%s' failed ('0x%08X')", getTraceName(), traceResult);
      }
    }
  }
}
