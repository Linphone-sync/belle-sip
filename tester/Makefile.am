if BUILD_TESTS

noinst_PROGRAMS=cain_sip_tester  basic_test cain_sip_object_describe

cain_sip_tester_SOURCES=	cain_sip_tester.c \
				cain_sdp_tester.c \
				cain_sip_uri_tester.c \
				cain_sip_headers_tester.c \
				cain_sip_message_tester.c \
				auth_helper_tester.c \
				cast_test.c

cain_sip_tester_CFLAGS=$(CUNIT_CFLAGS)

cain_sip_tester_LDFLAGS=$(CUNIT_LIBS)


basic_test_SOURCES=basic_test.c

cain_sip_object_describe_SOURCES=describe.c

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src

LDADD=$(top_builddir)/src/libcainsip.la 

AM_LDFLAGS=-export-dynamic 

AM_CFLAGS=$(STRICT_OPTIONS)


test: cain_sip_tester
	./cain_sip_tester
	
endif
