check_SCRIPTS=testmake.mk test.sh test_v2.sh test_v4.sh test_v5.sh
EXTRA_DIST=$(check_SCRIPTS)

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) \
		    top_builddir=$(top_builddir) \
		    $(SHELL)

TESTS=test_v5.sh test_v2.sh test_v4.sh

FULL_ENVIRONMENT = srcdir=$(srcdir) $(TESTS_ENVIRONMENT)

perfs.txt:
	$(FULL_ENVIRONMENT) speed.sh

mostlyclean-local:
	-for dir in `find $(builddir) -mindepth 3 -maxdepth 3`; \
		 do $(MAKE) -f $(srcdir)/testmake.mk CLEANDIR=$$dir clean; \
	 done
