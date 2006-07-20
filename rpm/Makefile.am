SPECFILES=debit-data-0.1-1.spec debit-0.1-1.spec
dist_noinst_DATA=$(SPECFILES)

rpm:
	mkdir -p SOURCES BUILD && mv ../$(PACKAGE)-$(VERSION).tar.gz SOURCES \
	rpmbuild --sign -bb --target noarch debit-data-0.1-1.spec && \
	rpmbuild --sign -bb debit-0.1-1.spec
