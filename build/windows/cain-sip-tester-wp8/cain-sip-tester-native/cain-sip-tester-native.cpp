// cain-sip-tester-native.cpp
#include "pch.h"
#include "cain-sip-tester-native.h"

using namespace cain_sip_tester_native;
using namespace Platform;

CainSipTesterNative::CainSipTesterNative()
{
}

void CainSipTesterNative::run()
{
	cain_sip_tester_run_tests("Resolver", 0);
}
