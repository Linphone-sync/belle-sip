if BUILD_TESTS

noinst_PROGRAMS=cain_sip_tester cast_test

cain_sip_tester_SOURCES=cain_sip_tester.c cain_sip_uri_tester.c cain_sip_headers_tester.c
cast_test_SOURCES=cast_test.c

INCLUDES=-I$(top_srcdir)/include

LDADD=$(top_builddir)/src/libcainsip.la -lcunit

AM_CFLAGS=$(STRICT_OPTIONS)

endif
