#include <string>

#include "cain-sip-tester-native.h"
#include "cain-sip/cain-sip.h"
#include "cunit/Util.h"

using namespace cain_sip_tester_native;
using namespace Platform;

#define MAX_TRACE_SIZE	512

static OutputTraceListener^ sTraceListener;

static void nativeOutputTraceHandler(int lev, const char *fmt, va_list args)
{
	if (sTraceListener) {
		wchar_t wstr[MAX_TRACE_SIZE];
		std::string str;
		str.resize(MAX_TRACE_SIZE);
		vsnprintf((char *)str.c_str(), MAX_TRACE_SIZE, fmt, args);
		mbstowcs(wstr, str.c_str(), sizeof(wstr));
		String^ msg = ref new String(wstr);
		sTraceListener->outputTrace(msg);
	}
}

static void cainSipNativeOutputTraceHandler(cain_sip_log_level lev, const char *fmt, va_list args)
{
	nativeOutputTraceHandler((int)lev, fmt, args);
}


CainSipTesterNative::CainSipTesterNative(OutputTraceListener^ traceListener)
{
	sTraceListener = traceListener;
}

void CainSipTesterNative::run(Platform::String^ name, Platform::Boolean verbose)
{
	std::wstring all(L"ALL");
	std::wstring suitename = name->Data();
	char cname[128] = { 0 };
	wcstombs(cname, suitename.c_str(), sizeof(cname));

	if (verbose) {
		cain_sip_set_log_level(CAIN_SIP_LOG_DEBUG);
	} else {
		cain_sip_set_log_level(CAIN_SIP_LOG_ERROR);
	}
	cain_sip_set_log_handler(cainSipNativeOutputTraceHandler);
	CU_set_trace_handler(nativeOutputTraceHandler);

	cain_sip_tester_run_tests(suitename == all ? 0 : cname, 0);
}
