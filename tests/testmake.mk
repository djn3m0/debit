DEBIT		?= $(top_builddir)/debit
DUMPARG		?= --fakearg
DATADIR		?= $(top_srcdir)/data
DEBITDBG	?= -g 0xffff
DEBIT_CMD	=$(VALGRIND) $(DEBIT) $(DEBITDBG) --datadir=$(DATADIR)

############################
### XDL/Debit comparison ###
############################

%.frames: %.bit $(DEBIT)
	mkdir -p $*.dir && \
	$(DEBIT_CMD) $(DUMPARG) --outdir $*.dir --input $< 2> $@.log && \
	echo $*.dir/* | xargs md5sum | sed -e 's/_u//' > $@ && \
	rm -Rf $*.dir

%.bram: %.bit $(DEBIT)
	$(DEBIT_CMD) --bramdump --input $< > $@ 2> $@.log

%.lut: %.bit $(DEBIT)
	$(DEBIT_CMD) --lutdump --input $< > $@ 2> $@.log

%.pip: %.bit $(DEBIT)
	$(DEBIT_CMD) --pipdump --input $< > $@ 2> $@.log

############################
### XDL/Debit comparison ###
############################

%.pipdebit: %.bit $(DEBIT)
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
	- rm -rf $(CLEANDIR)/*.dir
	- rm -rf $(CLEANDIR)/*.frames
	- rm -f $(CLEANDIR)/*.bram
	- rm -f $(CLEANDIR)/*.lut
	- rm -f $(CLEANDIR)/*.pip
	- rm -f $(CLEANDIR)/*.log

.PHONY: examples clean
