DEBIT		?= $(top_builddir)/debit
DUMPARG		?= --fakearg
DATADIR		?= $(top_srcdir)/data
DEBITDBG	?= -g 0xffff
DEBIT_CMD	=$(VALGRIND) $(DEBIT) $(DEBITDBG) --datadir=$(DATADIR)

%.frames: %.bit
	mkdir -p $@ && $(DEBIT_CMD) $(DUMPARG) --outdir $@ --input $< 2> $@.log

%.bram: %.bit
	$(DEBIT_CMD) --bramdump --input $< > $@ 2> $@.log

%.lut: %.bit
	$(DEBIT_CMD) --lutdump --input $< > $@ 2> $@.log

%.pip: %.bit
	$(DEBIT_CMD) --pipdump --input $< > $@ 2> $@.log

############################
### XDL/Debit comparison ###
############################

%.pipdebit: %.bit
	$(DEBIT_CMD) --lutdump --input $< > $@ 2> $@.log

#extract some information from the xdl
%.pipxdl: %.xdl
	cat $< | grep pip | awk  '{print $$1 " " $$2 " " $$3 " -> " $$5}' | sort > $@

#compare
%.pipcompare: %.pipdebit %.pipxdl
	diff --speed-large-file $^ > $@

#use cautiously
# %.golden: %
# 	mv $< $@

clean:
	- rm -Rf $(CLEANDIR)*.frames
	- rm -f $(CLEANDIR)*.bram
	- rm -f $(CLEANDIR)*.lut
	- rm -f $(CLEANDIR)*.pip
	- rm -f $(CLEANDIR)*.log

.PHONY: examples clean
