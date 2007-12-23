check_SCRIPTS=testmake.mk test.sh test_s3.sh test_v2.sh test_v4.sh test_v5.sh
EXTRA_DIST=$(check_SCRIPTS)

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) \
		    top_builddir=$(top_builddir)

TESTS=test_s3.sh test_v2.sh test_v4.sh test_v5.sh

FULL_ENVIRONMENT = srcdir=$(srcdir) $(TESTS_ENVIRONMENT)

# Copied on cairo's test suite
EXTRA_VALGRIND_FLAGS = $(DEBIT_EXTRA_VALGRIND_FLAGS)

VALGRIND_FLAGS = --error-exitcode=254 \
		 --tool=memcheck --leak-check=yes --show-reachable=yes \
		 --suppressions=$(top_srcdir)/tests/valgrind.exclude --gen-suppressions=all \
		$(EXTRA_VALGRIND_FLAGS)

check-valgrind: TESTS_ENVIRONMENT += VALGRIND_DEBIT_CMD="valgrind $(VALGRIND_FLAGS)"
check-valgrind:
	$(MAKE) $(AM_MAKEFLAGS) TESTS_ENVIRONMENT=$(TESTS_ENVIRONMENT) check 2>&1 \
	| tee valgrind-log


perfs.txt:
	$(FULL_ENVIRONMENT) speed.sh

mostlyclean-local:
	-for dir in `find $(builddir) -mindepth 3 -maxdepth 3`; \
		 do $(MAKE) -f $(srcdir)/testmake.mk CLEANDIR=$$dir clean; \
	 done

.PHONY: check-valgrind
