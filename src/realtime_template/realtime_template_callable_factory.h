/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <iostream>
#include <vector>

#include "common/scheduler/i_scheduler3.h"
#include "comm/datalayer/datalayer.h"

#include "realtime_template_callable.h"

namespace sdk_rt
{

#define _SDK_CLASS_NAME_(x) (#x)
#define SDK_CLASS_NAME(x) (_SDK_CLASS_NAME_(x))

  class RealTimeTemplateCallableFactory : public common::scheduler::ICallableFactory2
  {
  private:
    comm::datalayer::IDataLayerFactory *m_datalayerFactoryPtr = nullptr;
    std::string m_datalayerShmBaseAddress;

    std::shared_ptr<RealTimeTemplateCallable> m_callable = std::make_shared<RealTimeTemplateCallable>();



  public:
    RealTimeTemplateCallableFactory();
    virtual ~RealTimeTemplateCallableFactory() = default;

    //! This method returns the class name.
    //!
    //! @return                      Return class name.
    static const char *getClassName(void) { return (SDK_CLASS_NAME(RealTimeTemplateCallableFactory)); }

    void destroyAllCallables();

    // ICallableFactory2
    std::shared_ptr<common::scheduler::ICallable> createCallable(const comm::datalayer::Variant &param) final;
    common::scheduler::SchedStatus destroyCallable(const std::shared_ptr<common::scheduler::ICallable> &callable) final;
    common::scheduler::SchedStatus getCallableArguments(std::vector<std::string> &arguments) final;
    virtual void getFactoryDescription(std::vector<std::string> &description) const override;
    virtual void getCallableConfigurations(comm::datalayer::Variant &configurations) const override;

    void setDatalayerFactory(comm::datalayer::IDataLayerFactory *service);
    void setDatalayerShmBaseAddress(const std::string &datalayerShmBaseAddress);
  };
}
