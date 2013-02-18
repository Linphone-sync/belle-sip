// cain-sip-tester-native.cpp
#include "pch.h"
#include "cain-sip-tester-native.h"
#include "cain-sip/cain-sip.h"

using namespace cain_sip_tester_native;
using namespace Platform;

CainSipTesterNative::CainSipTesterNative()
{
}

void CainSipTesterNative::run(Platform::String^ name)
{
	char suitename[128];
	const wchar_t *wcname;

	suitename[0] = '\0';
	wcname = name->Data();
	wcstombs(suitename, wcname, sizeof(suitename));
	if (strncmp(suitename, "ALL", sizeof(suitename)) == 0) suitename[0] = '\0';
	cain_sip_set_log_level(CAIN_SIP_LOG_DEBUG);
	cain_sip_tester_run_tests(suitename[0] == '\0' ? 0 : suitename, 0);
}
