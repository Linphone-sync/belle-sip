
grammar_files= \
						$(srcdir)/cain_sip_message.g \
						$(srcdir)/cain_sdp.g

generated_src= \
	cain_sip_messageParser.c cain_sip_messageParser.h \
	cain_sip_messageLexer.c cain_sip_messageLexer.h \
	cain_sdpParser.c cain_sdpParser.h \
	cain_sdpLexer.c cain_sdpLexer.h

$(builddir)/generated_src_stamp : $(grammar_files)
	$(ANTLR) -make -fo $(builddir) $(grammar_files)
	touch $(builddir)/generated_src_stamp

$(generated_src)	:	$(builddir)/generated_src_stamp
	

BUILT_SOURCES=$(generated_src)

noinst_LTLIBRARIES=libcainsip_generated.la

nodist_libcainsip_generated_la_SOURCES=$(generated_src)

libcainsip_generated_la_CFLAGS=$(LESS_STRICT_OPTIONS) $(ANTLR_CFLAGS)

lib_LTLIBRARIES=libcainsip.la


libcainsip_la_SOURCES=				\
									clock_gettime.c clock_gettime.h \
									cain_sip_uri_impl.c \
									cain_sip_headers_impl.c \
									cain_sip_utils.c cain_sip_internal.h \
									cain_sip_object.c \
									cain_sip_loop.c \
									cain_sip_resolver.c cain_sip_resolver.h \
									cain_sip_parameters.c \
									cain_sdp_impl.c \
									transaction.c \
									listeningpoint.c \
									sipstack.c \
									provider.c \
									channel.c channel.h \
									message.c \
									md5.c \
									auth_helper.c \
									ict.c \
									transports/udp_listeningpoint.c \
									transports/udp_channel.c \
									transports/stream_channel.c \
									transports/stream_listeningpoint.c

libcainsip_la_CFLAGS=$(STRICT_OPTIONS) $(ANTLR_CFLAGS)

libcainsip_la_LIBADD=libcainsip_generated.la $(ANTLR_LIBS)


INCLUDES=-I$(top_srcdir)/include

EXTRA_DIST=$(grammar_files)

CLEANFILES=$(generated_src) *.tokens generated_src_stamp

discovery:
	touch specs.cpp
	$(CC) $(CFLAGS) $(ANTLR_CFLAGS) $(CUNIT_CFLAGS) -E -P -v -dD specs.cpp
