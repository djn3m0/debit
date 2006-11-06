DEBIT		?= debit
DATADIR		?=$(top_srcdir)/data
DEBITDBG	?=
DEBIT_CMD	=$(VALGRIND) $(DEBIT) $(DEBITDBG) --datadir=$(DATADIR)

%.frames: %.bit
	mkdir -p $@ && $(DEBIT_CMD) --framedump --outdir $@ --input $< 2> $@.log

%.bram: %.bit
	$(DEBIT_CMD) --bramdump --input $< > $@ 2> $@.log

%.lut: %.bit
	$(DEBIT_CMD) --lutdump --input $< > $@ 2> $@.log

%.pip: %.bit
	$(DEBIT_CMD) --pipdump --input $< > $@ 2> $@.log

#use cautiously
%.golden: %
	mv $< $@

clean:
	- rm -Rf $(CLEANDIR)*.frames
	- rm -f $(CLEANDIR)*.bram
	- rm -f $(CLEANDIR)*.lut
	- rm -f $(CLEANDIR)*.pip
	- rm -f $(CLEANDIR)*.log

.PHONY: examples clean
