/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

#include <common/log/trace/defs.h>
#include <common/log/trace/log_buffered3.h>

#include "trace/trace.h"

#include "realtime_template.h"

/*
  From: https://celix.apache.org/docs/2.3.0/celix/documents/components.html

  Components can provide services and depend on services. Components are configured
  declarative using the DM api.

  Service dependencies will influence the component’s lifecycle as a component will
  only be active when all required dependencies are available.
  The DM is responsible for managing the component’s service dependencies, the
  component’s lifecycle and when to register/unregister the component’s provided services.
*/

namespace sdk_rt
{

  //! Called at the beginning of the components lifecycle.
  //! Prerequisites: None
  //! Conditions after call: Your component is prepared to get injected all dependent services.
  celix_status_t RealTimeTemplate::init()
  {
    TRACE_INFO(" ");

    // Task: Prepare your component...
    m_sdkCallableFactoryPtr = std::make_shared<sdk_rt::RealTimeTemplateCallableFactory>();

    return CELIX_SUCCESS;
  }

  //! Called to register provided services.
  //! Prerequisites: All required dependencies are injected to your component
  //! Conditions after call:
  //! - Your component is up and running eg: all needed tasks are started
  //! - All services provided by your component can be injected to other components
  celix_status_t RealTimeTemplate::start()
  {
    TRACE_INFO(" ");

    m_sdkCallableFactoryPtr->setDatalayerFactory(m_dataLayerFactoryPtr);

    // m_dataLayerFactoryPtr cannot be == nullptr because the service is REQUIRED
    auto status = m_schedulerServicePtr->registerCallableFactory(m_sdkCallableFactoryPtr, "RealtimeTemplate");
    if (common::scheduler::SchedStatus::SCHED_S_OK != status)
    {
      TRACE_ERR("registerCallableFactory 'RealtimeTempalte' failed");
      return CELIX_SERVICE_EXCEPTION;
    }

    return CELIX_SUCCESS;
  }

  //! Called to stop provided services.
  //! Prerequisites: Your services provided are not used by other components
  //! Conditions after call:
  //! - Your component is shutdown
  //! - You will not use the dependent services
  celix_status_t RealTimeTemplate::stop()
  {
    TRACE_INFO(" ");

    m_schedulerServicePtr->unregisterCallableFactory(m_sdkCallableFactoryPtr, true);
    m_sdkCallableFactoryPtr->destroyAllCallables();

    return CELIX_SUCCESS;
  }

  //! Called to cleanup the instance.
  //! Prerequisites: Your bundle has no dependent services anymore
  //! Conditions after call: Your bundle is ready to be unloaded
  celix_status_t RealTimeTemplate::deinit()
  {
    TRACE_INFO(" ");

    // Task: Do everything so that your component can be unloaded...
    m_sdkCallableFactoryPtr = nullptr;
    m_schedulerServicePtr = nullptr;

    return CELIX_SUCCESS;
  }

  //! This method is called when the service was added.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::schedulerServiceAdded(common::scheduler::IScheduler3 *service)
  {
    // service is always != nullptr
    TRACE_INFO(" ");

    m_schedulerServicePtr = service;
  }

  //! This method is called when the state of the service changes.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::schedulerServiceStateChanged(common::scheduler::IScheduler3 *service)
  {
    // On stop service == nullptr
    TRACE_INFO(" ");
  }

  //! This method is called when the service was removed.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::schedulerServiceRemoved(common::scheduler::IScheduler3 *service)
  {
    // service is always != nullptr
    TRACE_INFO(" ");
  }

  //! This method is called when the service was added.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::traceServiceAdded(common::log::trace::IRegistrationRealTime3 *service)
  {
    // service is always != nullptr
    TRACE_INFO(" ");

    // Register here, because start method can already be called.
    Trace::getInstance()->attachTrace(service);

    m_traceServiceAvailable = true;
  }

  //! This method is called when the state of the service changes.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::traceServiceStateChanged(common::log::trace::IRegistrationRealTime3 *service)
  {
    // On stop service == nullptr
    TRACE_INFO(" ");
  }

  //! This method is called when the service was removed.
  //!
  //! @param[in]  service  A pointer to the service.
  void RealTimeTemplate::traceServiceRemoved(common::log::trace::IRegistrationRealTime3 *service)
  {
    // service is always != nullptr
    TRACE_INFO(" ");
    Trace::getInstance()->detachTrace(service);

    m_traceServiceAvailable = false;
  }

  void RealTimeTemplate::datalayerServiceAdded(comm::datalayer::IDataLayerFactory *service)
  {
    TRACE_INFO(" ");
    m_dataLayerFactoryPtr = service;
  }

  void RealTimeTemplate::datalayerServiceStateChanged(comm::datalayer::IDataLayerFactory *service)
  {
    // On stop service == nullptr
    TRACE_INFO(" ");
  }

  void RealTimeTemplate::datalayerServiceRemoved(comm::datalayer::IDataLayerFactory *service)
  {
    // service is always != nullptr
    TRACE_INFO(" ");
    // No need to inform RealTimeTemplateCallablefactory here, because we are suspended if DataLayer service is no longer active.
  }
}