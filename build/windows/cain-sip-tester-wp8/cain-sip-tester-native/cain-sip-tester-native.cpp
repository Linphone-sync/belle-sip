#include <string>

#include "cain-sip-tester-native.h"
#include "cain-sip/cain-sip.h"
#include "cunit/Util.h"

using namespace cain_sip_tester_native;
using namespace Platform;

#define MAX_TRACE_SIZE		512
#define MAX_SUITE_NAME_SIZE	128

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


CainSipTesterNative::CainSipTesterNative()
{
	cain_sip_tester_init();
}

CainSipTesterNative::~CainSipTesterNative()
{
	cain_sip_tester_uninit();
}

void CainSipTesterNative::setOutputTraceListener(OutputTraceListener^ traceListener)
{
	sTraceListener = traceListener;
}

void CainSipTesterNative::run(Platform::String^ suiteName, Platform::String^ caseName, Platform::Boolean verbose)
{
	std::wstring all(L"ALL");
	std::wstring wssuitename = suiteName->Data();
	std::wstring wscasename = caseName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	char ccasename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, wssuitename.c_str(), sizeof(csuitename));
	wcstombs(ccasename, wscasename.c_str(), sizeof(ccasename));

	if (verbose) {
		cain_sip_set_log_level(CAIN_SIP_LOG_DEBUG);
	} else {
		cain_sip_set_log_level(CAIN_SIP_LOG_ERROR);
	}
	cain_sip_set_log_handler(cainSipNativeOutputTraceHandler);
	CU_set_trace_handler(nativeOutputTraceHandler);

	cain_sip_tester_run_tests(wssuitename == all ? 0 : csuitename, wscasename == all ? 0 : ccasename);
}

unsigned int CainSipTesterNative::nbTestSuites()
{
	return cain_sip_tester_nb_test_suites();
}

unsigned int CainSipTesterNative::nbTests(Platform::String^ suiteName)
{
	std::wstring suitename = suiteName->Data();
	char cname[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(cname, suitename.c_str(), sizeof(cname));
	return cain_sip_tester_nb_tests(cname);
}

Platform::String^ CainSipTesterNative::testSuiteName(int index)
{
	const char *cname = cain_sip_tester_test_suite_name(index);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}

Platform::String^ CainSipTesterNative::testName(Platform::String^ suiteName, int testIndex)
{
	std::wstring suitename = suiteName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, suitename.c_str(), sizeof(csuitename));
	const char *cname = cain_sip_tester_test_name(csuitename, testIndex);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}
