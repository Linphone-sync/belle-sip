#pragma once

extern "C" {
	int cain_sip_tester_run_tests(char *suite_name, char *test_name);
}

namespace cain_sip_tester_native
{
    public ref class CainSipTesterNative sealed
    {
    public:
        CainSipTesterNative();
		void run();
    };
}