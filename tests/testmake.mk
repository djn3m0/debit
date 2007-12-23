top_srcdir	?= ..
top_builddir	?= $(top_srcdir)
DEBIT		?= $(top_builddir)/debit
XDL2BIT         ?= $(top_builddir)/xdl/xdl2bit
DUMPARG		?= --fakearg
DATADIR		?= $(top_srcdir)/data
DEBITDBG	?= -g 0x0
DEBIT_CMD	=$(VALGRIND_DEBIT_CMD) $(DEBIT) $(DEBITDBG) --datadir=$(DATADIR)
XDL2BIT_CMD	=$(VALGRIND_DEBIT_CMD) $(XDL2BIT) $(DEBITDBG) --datadir=$(DATADIR)

##################
### Debit work ###
##################

#only the stdout will reach the file, stderr will be piped to next process
DUMPME = 2>&1 1>$@
LOGME = 2>$@.log

%.frames: %.bit $(DEBIT)
	mkdir -p $*.dir && \
	$(DEBIT_CMD) $(DUMPARG) --outdir $*.dir --input $< $(DUMPME) $(LOGME) && \
	echo $*.dir/* | xargs md5sum | sed -e 's/_u//' $(DUMPME) && \
	rm -Rf $*.dir

%.rewrite: %.bit $(DEBIT)
	$(DEBIT_CMD) --input $< --outfile $@ $(LOGME)

%.bram: %.bit $(DEBIT)
	$(DEBIT_CMD) --bramdump --input $< $(DUMPME) $(LOGME)

%.lut: %.bit $(DEBIT)
	$(DEBIT_CMD) --lutdump --input $< $(DUMPME) $(LOGME)

%.pip: %.bit $(DEBIT)
	$(DEBIT_CMD) --pipdump --input $< $(DUMPME) $(LOGME)

%.nets: %.bit $(DEBIT)
	$(DEBIT_CMD) --netdump --input $< $(DUMPME) $(LOGME)

####################
### xdl2bit work ###
####################

%.xdl2bit: %.xdl $(XDL2BIT)
	$(XDL2BIT_CMD) --input $< --output $@ $(LOGME)

############################
### XDL/Debit comparison ###
############################

%.pipdebit: %.bit $(DEBIT)
	$(DEBIT_CMD) --pipdump --input $< $(LOGME) | sort $(DUMPME)

#extract some information from the xdl
%.pipxdl: %.xdl
	cat $< | grep pip | awk  '{print $$1 " " $$2 " " $$3 " -> " $$5}' | sort $(DUMPME)

#compare
%.pipcompare: %.pipdebit %.pipxdl
	diff --speed-large-file $^ $(DUMPME)

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
	cat $^ | awk 'BEGIN {RS=""} {print "scale=3; print " $$3 " / " $$1 ",\" \"," $$4 " / " $$2 ",\"\n\""  }' | bc $(DUMPME)

#Do a bit of formatting. Count pips.
%.pipcount: %.pip
	echo -n "pips " $(DUMPME) && \
	cat $< | wc -l >> $@

%.allspeed: %.pipcount %.debitspeed %.xilspeed %.scale
	echo "[$*]" $(DUMPME) && \
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
	- rm -f $(CLEANDIR)/*.xdl2bit
	- rm -f $(CLEANDIR)/*.log
	- rm -f $(CLEANDIR)/*.allspeed
	- rm -f $(CLEANDIR)/*.ncd
	- rm -f $(CLEANDIR)/*.allspeed
	- rm -f $(CLEANDIR)/*.xilspeed
	- rm -f $(CLEANDIR)/*.debitspeed

.PHONY: examples clean
