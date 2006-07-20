SUBDIRS = data

AM_CPPFLAGS	= -Wall -Werror -Wunused -O3 -g -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS	= debit
debit_SOURCES	= bitarray.c bitheader.c bitstream_parser.c filedump.c \
		localpips.c bitstream.c wiring.c debit.c \
		bitarray.h bitstream.h config.h design.h localpips.h \
		virtex2_config.h bitheader.h bitstream_parser.h \
		debitlog.h filedump.h sites.h wiring.h debit.h
debit_CFLAGS	= @GLIB_CFLAGS@ #-DG_DISABLE_ASSERT -DDEBIT_DEBUG=0
debit_LDFLAGS	= @GLIB_LIBS@

#Specific testing and profiling targets. To be reorganized

DEBIT=debit

TMPDIR=trash
BITFILE_REF=$(TMPDIR)/reference.bit
REF_DIR=$(TMPDIR)/reference_dir
TEST_DIR=$(TMPDIR)/test_dir
PROFILE_DIR=$(TMPDIR)/profile_dir
DEBIT_ARGS=--pipdump

checkpoint: $(DEBIT)
	mkdir -p $(REF_DIR) && pushd $(REF_DIR) && \
	$$OLDPWD/$< --input $$OLDPWD/$(BITFILE_REF) $(DEBIT_ARGS) && \
	popd

checkcheckpoint: $(DEBIT)
	mkdir -p $(TEST_DIR) && pushd $(TEST_DIR) && \
	$$OLDPWD/$< --input $$OLDPWD/$(BITFILE_REF) $(DEBIT_ARGS) && \
	popd && \
	diff -q $(REF_DIR) $(TEST_DIR) && echo "Test OK"

all-profiles: profile-mem profile-exec profile-std

# gather some profiling data. Please use !
MASSIF_ARGS=--tool=massif --depth=5 --alloc-fn=g_malloc0 --alloc-fn=g_malloc --alloc-fn=__xcalloc --alloc-fn=__xmalloc
DEBIT_MASSIF_ARGS=--pipdump
profile-mem: $(DEBIT)
	mkdir -p $(PROFILE_DIR) && pushd $(PROFILE_DIR) && rm -f *.ps && \
	valgrind $(MASSIF_ARGS) $$OLDPWD/$< $(DEBIT_MASSIF_ARGS) --input $$OLDPWD/$(BITFILE_REF) &> /dev/null && \
	popd

DEBIT_EXEC_ARGS=--pipdump
profile-exec: $(DEBIT)
	mkdir -p $(PROFILE_DIR) && pushd $(PROFILE_DIR) && rm -f callgrind.out.* && \
	callgrind $$OLDPWD/$< $(DEBIT_EXEC_ARGS) --input $$OLDPWD/$(BITFILE_REF) &> /dev/null && \
	callgrind_annotate callgrind.out.* --auto=yes --include=$$OLDPWD > callgrind.out && \
	popd

MEMCHECK_ARGS=--tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=yes --log-file=valgrind
profile-std: $(DEBIT)
	mkdir -p $(PROFILE_DIR) && pushd $(PROFILE_DIR) && rm -f valgrind.* && \
	valgrind $(MEMCHECK_ARGS) $$OLDPWD/$< $(DEBIT_ARGS) --input $$OLDPWD/$(BITFILE_REF) &> /dev/null && \
	popd

# doxygen documentation generation
DOCSOURCES=$(shell find ./ -name '*.c' -or -name '*.h')
DOXYCONFIG=doc/doxygen.cfg
DOXYDIR=doc

doc: $(DOCSOURCES) $(DOXYCONFIG)
	doxygen $(DOXYCONFIG) #&& make -C $(DOXYDIR)/latex/

deb: dist
#could also be $(DIST_ARCHIVES)
	mv $(PACKAGE)-$(VERSION).tar.gz ../$(PACKAGE)_$(VERSION).orig.tar.gz && dpkg-buildpackage -rfakeroot -i'(trash|.git)' -Itrash -I.git

.PHONY: debian
