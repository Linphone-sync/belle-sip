// cain-sip-tester-native.cpp
#include "pch.h"
#include "cain-sip-tester-native.h"
#include "cain-sip/cain-sip.h"

using namespace cain_sip_tester_native;
using namespace Platform;

CainSipTesterNative::CainSipTesterNative()
{
}

void CainSipTesterNative::run()
{
	cain_sip_set_log_level(CAIN_SIP_LOG_DEBUG);
	cain_sip_tester_run_tests(0, 0);
}
