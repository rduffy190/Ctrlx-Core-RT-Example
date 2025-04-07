/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>

#include "celix_errno.h"
#include "celix/dm/DmActivator.h"

#include "common/scheduler/i_scheduler3.h"
#include "common/log/trace/i_registration_real_time3.h"
#include "comm/datalayer/datalayer.h"

#include "realtime_template_callable_factory.h"

#define DATA_LAYER_SHARED_MEMORY_SIZE (1000)
// Revision should be unique for this Layout, if you need a new memory layout define a new revision, or use checksum algorithms
#define DATA_LAYER_SHARED_MEMORY_REVISION (0)

namespace sdk_rt
{

  //! @class RealTimeTemplateComponent
  //! This is the bundle specific component class
  class RealTimeTemplate
  {
  private:

    // Store services 
    common::scheduler::IScheduler3 *m_schedulerServicePtr = nullptr;
    comm::datalayer::IDataLayerFactory *m_dataLayerFactoryPtr = nullptr;

    // Trace service
    bool m_traceServiceAvailable = false;

    // std::shared_ptr needed for common::scheduler::IScheduler3::registerCallableFactory()
    std::shared_ptr<sdk_rt::RealTimeTemplateCallableFactory> m_sdkCallableFactoryPtr = nullptr;

  public:

    RealTimeTemplate() = default;
    virtual ~RealTimeTemplate() = default;

    celix_status_t init();
    celix_status_t start();
    celix_status_t stop();
    celix_status_t deinit();

    void schedulerServiceStateChanged(common::scheduler::IScheduler3 *service);
    void schedulerServiceAdded(common::scheduler::IScheduler3 *service);
    void schedulerServiceRemoved(common::scheduler::IScheduler3 *service);

    void traceServiceStateChanged(common::log::trace::IRegistrationRealTime3 *service);
    void traceServiceAdded(common::log::trace::IRegistrationRealTime3 *service);
    void traceServiceRemoved(common::log::trace::IRegistrationRealTime3 *service);

    void datalayerServiceAdded(comm::datalayer::IDataLayerFactory *service);
    void datalayerServiceStateChanged(comm::datalayer::IDataLayerFactory *service);
    void datalayerServiceRemoved(comm::datalayer::IDataLayerFactory *service);
  };
}
