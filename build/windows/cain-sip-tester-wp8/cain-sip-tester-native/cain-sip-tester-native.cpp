#include <string>

#include "cain-sip-tester-native.h"
#include "cain-sip/cain-sip.h"

using namespace cain_sip_tester_native;
using namespace Platform;

CainSipTesterNative::CainSipTesterNative()
{
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

	cain_sip_tester_run_tests(suitename == all ? 0 : cname, 0);
}
