
// ReSharper disable CppDeprecatedEntity
// ReSharper disable CppClangTidyCertErr33C
// ReSharper disable CppClangTidyClangDiagnosticDeprecatedDeclarations
// ReSharper disable CppClangTidyClangDiagnosticFormatNonliteral
#include "trace.h"
#include <iostream>
#include <filesystem>


namespace sdk_rt
{
    void Trace::log(const TraceLevel level, const char *file, const char *functionName, std::uint32_t lineNumber, const char *format, ...) const
    {
        if (isDebugEnvironment())
        {
            va_list args;
            va_start(args, format);
            logConsole(level, file, functionName, lineNumber, format, args);
            va_end(args);
        }

        va_list args;
        va_start(args, format);
        logRealTime(level, file, functionName, lineNumber, format, args);
        va_end(args);
    }

    void Trace::logConsole(TraceLevel logLevel, const char *fileName, const char *functionName, const uint32_t lineNumber, const char *format, va_list args) const
    {
        char message[1024];
        vsprintf(message, format, args);

        switch (logLevel)
        {
        case TraceLevel::Info:
            std::cout << green << "INFO ";
            break;
        case TraceLevel::Warning:
            std::cout << yellow << "WARN ";
            break;
        case TraceLevel::Error:
            std::cout << red << "ERROR ";
            break;
        }

        std::cout << std::filesystem::relative(fileName, std::filesystem::current_path()).c_str()
                  << "(" << lineNumber << "): "
                  << functionName << "() "
                  << message
                  << reset
                  << '\n';
    }

    void Trace::logRealTime(TraceLevel logLevel, const char *fileName, const char *functionName, const uint32_t lineNumber, const char *format, va_list args) const
    {
        if (nullptr == m_rtTraceContext)
        {
            return;
        }

        if (logLevel == TraceLevel::Info && !m_traceEnablingState.messages)
        {
            return;
        }

        if (logLevel == TraceLevel::Warning && !m_traceEnablingState.warnings)
        {
            return;
        }

        if (logLevel == TraceLevel::Error && !m_traceEnablingState.errors)
        {
            return;
        }

        const LogParametersExtended logParameters{
            getEntity(),
            getSnapName(),
            getFileName(fileName),
            functionName,
            lineNumber,
            format};

        m_rtTraceContext->logRealTimeVaList(AUTOMATION_TRACE_MAIN_DIAGNOSIS_CODE_MESSAGE, &logParameters, args);
    }

    void Trace::attachTrace(IRegistrationRealTime3 *traceRegistration)
    {
        // Function internal callback to receive enablingState
        auto enablingStateCallback = [&](const EnablingState &enablingState)
        {
            m_traceEnablingState = enablingState;
        };

        ILogRealTime3 *rtTraceContext = nullptr;
        const TraceResult traceResult = traceRegistration->registerUnit(
            &rtTraceContext, getEntity(), m_traceEnablingState, enablingStateCallback);
        if (rtTraceContext && statusSucceeded(traceResult))
        {
            m_rtTraceContext = rtTraceContext;
            m_traceRegistration = traceRegistration;
            TRACE_INFO("Register rt trace unit '%s' succeeded", getTraceName())
        }
        else
        {
            TRACE_ERR("Register rt trace unit '%s' failed ('0x%08X')", getTraceName(), traceResult)
        }
    }

    void Trace::detachTrace(IRegistrationRealTime3 *traceRegistration)
    {
        ILogRealTime3 *traceContext = m_rtTraceContext;
        if (traceContext != &m_traceBuffered)
        {
            m_rtTraceContext = &m_traceBuffered;
            const TraceResult traceResult = traceRegistration->unregisterUnit(&traceContext);
            if (statusSucceeded(traceResult))
            {
                TRACE_INFO("Unregister rt trace unit '%s' succeeded", getTraceName())
            }
            else
            {
                TRACE_ERR("Unregister rt trace unit '%s' failed ('0x%08X')", getTraceName(), traceResult)
            }
            m_traceRegistration = nullptr;
        }
    }

    bool Trace::isDebugEnvironment()
    {
        const std::string snapEnv = std::getenv("SNAP_ENV"); // NOLINT(concurrency-mt-unsafe)
        return snapEnv == "DEBUG";
    }
}