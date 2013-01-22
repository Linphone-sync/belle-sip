
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
									port.c port.h \
									cain_sip_uri_impl.c \
									cain_sip_headers_impl.c \
									cain_sip_utils.c cain_sip_internal.h \
									cain_sip_object.c \
									cain_sip_loop.c \
									cain_sip_resolver.c cain_sip_resolver.h \
									cain_sip_parameters.c \
									cain_sdp_impl.c \
									transaction.c \
									listeningpoint.c listeningpoint_internal.h \
									sipstack.c \
									provider.c \
									channel.c channel.h \
									message.c \
									md5.c md5.h \
									auth_helper.c \
									siplistener.c \
									ict.c \
									ist.c \
									nict.c \
									nist.c \
									dialog.c \
									auth_event.c \
									transports/udp_listeningpoint.c \
									transports/udp_channel.c \
									transports/stream_channel.c \
									transports/stream_channel.h \
									transports/stream_listeningpoint.c \
									transports/tls_listeningpoint.c \
									refresher.c refresher-helper.h
if BUILD_TLS
libcainsip_la_SOURCES+=transports/tls_channel.c 
endif

libcainsip_la_CFLAGS=$(STRICT_OPTIONS) $(ANTLR_CFLAGS) $(TLS_CFLAGS)

libcainsip_la_LIBADD=libcainsip_generated.la $(ANTLR_LIBS) $(TLS_LIBS)

libcainsip_la_LDFLAGS=-no-undefined


INCLUDES=-I$(top_srcdir)/include

EXTRA_DIST=$(grammar_files)

CLEANFILES=$(generated_src) *.tokens generated_src_stamp

discovery:
	touch specs.cpp
	$(CC) $(CFLAGS) -include $(top_builddir)/config.h $(ANTLR_CFLAGS) $(CUNIT_CFLAGS) $(TLS_CFLAGS) -E -P -v -dD specs.cpp

