#pragma once

#include "cain-sip/cain-sip.h"

#ifdef __cplusplus
extern "C" {
#endif
	int cain_sip_tester_run_tests(char *suite_name, char *test_name);
#ifdef __cplusplus
}
#endif

namespace cain_sip_tester_native
{
	public interface class OutputTraceListener
	{
	public:
		void outputTrace(Platform::String^ msg);
	};

    public ref class CainSipTesterNative sealed
    {
    public:
        CainSipTesterNative(OutputTraceListener^ traceListener);
		void run(Platform::String^ name, Platform::Boolean verbose);
    };
}