top_srcdir	?= ..
top_builddir	?= $(top_srcdir)
DEBIT		?= $(top_builddir)/debit
DUMPARG		?= --fakearg
DATADIR		?= $(top_srcdir)/data
DEBITDBG	?= -g 0x0
DEBIT_CMD	=$(VALGRIND) $(DEBIT) $(DEBITDBG) --datadir=$(DATADIR)

##################
### Debit work ###
##################

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
	$(DEBIT_CMD) --pipdump --input $< > $@ 2> $@.log

#extract some information from the xdl
%.pipxdl: %.xdl
	cat $< | grep pip | awk  '{print $$1 " " $$2 " " $$3 " -> " $$5}' | sort > $@

#compare
%.pipcompare: %.pipdebit %.pipxdl
	diff --speed-large-file $^ > $@

#use cautiously
# %.golden: %
# 	mv $< $@

######################################
### Debit/Xilinx timing comparison ###
######################################

TIME_FORMAT="%e %U"
TIME_CMD=`which time` -f $(TIME_FORMAT)

BITGEN_CMD=bitgen
BITGEN_SPEED_ARGS=-d -w

DEBIT_SPEED_ARGS=--lutdump --bramdump --pipdump

XDL2NCD=xdl -xdl2ncd

%.debitspeed: %.bit
	$(TIME_CMD) -o $@ $(DEBIT_CMD) $(DEBIT_SPEED_ARGS) --input $< > dummy.bit

%.xilspeed: %.ncd
	$(TIME_CMD) -o $@ $(BITGEN_CMD) $(BITGEN_SPEED_ARGS) $< dummy.bit

%.scale: %.debitspeed %.xilspeed
	cat $^ | awk 'BEGIN {RS=""} {print "scale=3; print " $$3 " / " $$1 ",\" \"," $$4 " / " $$2 ",\"\n\""  }' | bc > $@

#Do a bit of formatting. Count pips.
%.pipcount: %.pip
	echo -n "pips " > $@ && \
	cat $< | wc -l >> $@

%.allspeed: %.pipcount %.debitspeed %.xilspeed %.scale
	echo "[$*]" > $@ && \
	cat $^ | tr ' ' '\t' >> $@ && \
	echo "---" >> $@

%.ncd: %.xdl
	$(XDL2NCD) $< $@

clean:
	- rm -rf $(CLEANDIR)/*.dir
	- rm -rf $(CLEANDIR)/*.frames
	- rm -f $(CLEANDIR)/*.bram
	- rm -f $(CLEANDIR)/*.lut
	- rm -f $(CLEANDIR)/*.pip
	- rm -f $(CLEANDIR)/*.log
	- rm -f $(CLEANDIR)/*.allspeed
	- rm -f $(CLEANDIR)/*.ncd
	- rm -f $(CLEANDIR)/*.allspeed
	- rm -f $(CLEANDIR)/*.xilspeed
	- rm -f $(CLEANDIR)/*.debitspeed

.PHONY: examples clean
