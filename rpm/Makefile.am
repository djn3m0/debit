SPECFILES=debit-data-0.1-1.spec debit-0.1-1.spec
RPMFILES=$(shell ls *.rpm)
dist_noinst_DATA=$(SPECFILES)
CLEANFILES=$(RPMFILES)

.PHONY: rpms

rpms:
	mkdir -p SOURCES BUILD && mv ../$(PACKAGE)-$(VERSION).tar.gz SOURCES \
	rpmbuild --sign -bb --target noarch debit-data-0.1-1.spec && \
	rpmbuild --sign -bb debit-0.1-1.spec

clean-local:
	- rm -Rf SOURCES BUILD

