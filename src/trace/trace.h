/*
 * SPDX-FileCopyrightText: Bosch Rexroth AG
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdarg>

#include "common/log/trace/defs.h"
#include "common/log/trace/i_log_real_time3.h"
#include "common/log/trace/i_registration_real_time3.h"
#include "common/log/trace/log_buffered3.h"

#include <fstream>
#include <memory>

namespace sdk_rt
{
	using namespace common::log::trace;

#define TRACE_INFO(text, ...)                                                                                               \
	{                                                                                                                       \
		Trace::getInstance()->log(TraceLevel::Info, __FILE__, __FUNCTION__, __LINE__, text, ##__VA_ARGS__); \
	}

#define TRACE_WARN(text, ...)                                                                                                  \
	{                                                                                                                          \
		Trace::getInstance()->log(TraceLevel::Warning, __FILE__, __FUNCTION__, __LINE__, text, ##__VA_ARGS__); \
	}

#define TRACE_ERR(text, ...)                                                                                                 \
	{                                                                                                                        \
		Trace::getInstance()->log(TraceLevel::Error, __FILE__, __FUNCTION__, __LINE__, text, ##__VA_ARGS__); \
	}

	enum class TraceLevel
	{
		Error = 1,
		Warning = 2,
		Info = 3
	};

	class Trace
	{
	public:
		static Trace *getInstance()
		{
			static std::unique_ptr<Trace> instance{std::make_unique<Trace>()};
			return instance.get();
		}

		ILogRealTime3 &operator()() const { return *m_rtTraceContext; }

		void attachTrace(IRegistrationRealTime3 *traceRegistration);
		void detachTrace(IRegistrationRealTime3 *traceRegistration);

		static bool isDebugEnvironment();

		void setTraceName(const std::string &traceName) { m_traceName = traceName; }
		const char *getTraceName() const { return m_traceName.c_str(); }

		void setSnapName(const std::string &snapName) { m_snapName = snapName; }
		const char *getSnapName() const { return m_snapName.c_str(); }

		void setEntity(const std::string &entity) { m_entity = entity; }
		const char *getEntity() const { return m_entity.c_str(); }

		void log(TraceLevel level, const char *file, const char *functionName, std::uint32_t lineNumber, const char *format, ...) const;
		void logRealTime(TraceLevel logLevel, const char *fileName, const char *functionName, uint32_t lineNumber, const char *format, va_list args) const;
		void logConsole(TraceLevel logLevel, const char *fileName, const char *functionName, uint32_t lineNumber, const char *format, va_list args) const;

	private:
		// Reset console color
		static constexpr const char *reset = "\033[0m";

		// Green console color
		static constexpr const char *green = "\033[32m";

		// Yellow console color
		static constexpr const char *yellow = "\033[33m";

		// Red console color
		static constexpr const char *red = "\033[31m";

		// Main diagnosis code for trace message of component common.scheduler defined in diagnosis database.
		static constexpr std::uint32_t AUTOMATION_TRACE_MAIN_DIAGNOSIS_CODE_MESSAGE = 0x0E0A0ABC;

		// Main diagnosis code for trace warning of component common.scheduler defined in diagnosis database.
		static constexpr std::uint32_t AUTOMATION_TRACE_MAIN_DIAGNOSIS_CODE_WARNING = 0x0E0E0ABC;

		// Main diagnosis code for trace error of component common.scheduler defined in diagnosis database.
		static constexpr std::uint32_t AUTOMATION_TRACE_MAIN_DIAGNOSIS_CODE_ERROR = 0x0E0F0ABC;

		static bool m_isDebugEnvironment;

		std::string m_entity;
		std::string m_traceName;
		std::string m_snapName;

		ILogRealTime3 *m_rtTraceContext = &m_traceBuffered;
		LogBuffered3 m_traceBuffered{getTraceName(), true};
		IRegistrationRealTime3 *m_traceRegistration = nullptr;
		EnablingState m_traceEnablingState = {true, true, true}; // info, warning, error
	};
}