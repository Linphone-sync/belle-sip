ANTLR=antlr3

grammar_files= \
						$(srcdir)/cain_sip_message.g
						$(srcdir)/cain_sdp.g

generated_src= \
	cain_sip_uriParser.c cain_sip_uriParser.h \
	cain_sip_uriLexer.c cain_sip_uriLexer.h \
	cain_sip_messageParser.c cain_sip_messageParser.h \
	cain_sip_messageLexer.c cain_sip_messageLexer.h

$(builddir)/generated_src_stamp : $(grammar_files)
	$(ANTLR) -make -fo $(builddir) $(grammar_files)
	touch $(builddir)/generated_src_stamp

$(generated_src)	:	$(builddir)/generated_src_stamp
	

BUILT_SOURCES=$(generated_src)

noinst_LTLIBRARIES=libcainsip_generated.la

nodist_libcainsip_generated_la_SOURCES=$(generated_src)

libcainsip_generated_la_CFLAGS=$(LESS_STRICT_OPTIONS)

lib_LTLIBRARIES=libcainsip.la


libcainsip_la_SOURCES=clock_gettime.h \
									cain_sip_uri_impl.c \
									cain_sip_headers_impl.c \
									cain_sip_utils.c cain_sip_internal.h \
									cain_sip_object.c \
									cain_sip_loop.c \
									cain_sip_resolver.c cain_sip_resolver.h \
									cain_sip_parameters.c \
									transaction.c \
									listeningpoint.c \
									sipstack.c \
									provider.c \
									sender_task.c sender_task.h \
									message.c

libcainsip_la_CFLAGS=$(STRICT_OPTIONS)

libcainsip_la_LIBADD=libcainsip_generated.la -lantlr3c

INCLUDES=-I$(top_srcdir)/include

EXTRA_DIST=$(grammar_files)

CLEANFILES=$(generated_src) *.tokens generated_src_stamp
