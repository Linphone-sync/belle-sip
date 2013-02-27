#pragma once

#include "cain-sip/cain-sip.h"
#include "cain_sip_tester.h"

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
        CainSipTesterNative();
		virtual ~CainSipTesterNative();
		void setOutputTraceListener(OutputTraceListener^ traceListener);
		unsigned int nbTestSuites();
		unsigned int nbTests(Platform::String^ suiteName);
		Platform::String^ testSuiteName(int index);
		Platform::String^ testName(Platform::String^ suiteName, int testIndex);
		void run(Platform::String^ suiteName, Platform::String^ caseName, Platform::Boolean verbose);
    };
}