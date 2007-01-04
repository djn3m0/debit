# This directory only contains data file

DATAPREFIX=clb tterm bterm rterm lterm lioi rioi tioi bioi \
	   bram ttermbram btermbram bioibram tioibram

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))

# Virtex II chipd database
V2CHIPS=40 80 250 500 1000 1500 2000 3000 4000 6000 8000
V2CHIPDIRS=$(addprefix xc2v, $(V2CHIPS))
V2CHIPDB=$(addsuffix /chip_control, $(V2CHIPDIRS)) $(addsuffix /chip_data, $(V2CHIPDIRS))
V2GENSCRIPT=generate_vII_chip.pl

WIRES_RAW_DB=wires.m4
WIRES_H_DB=wires.h
WIRES_DB=wires.db

DBGENSCRIPT=$(srcdir)/../wires_to_db.pl
WIRES_M4_SCRIPT=$(srcdir)/../dbtoini.m4
WIRES_H_M4_SCRIPT=$(srcdir)/../dbtoh.m4

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB) $(V2CHIPDB)

$(V2CHIPDB): $(V2GENSCRIPT)
	$(srcdir)/$<

$(WIRES_RAW_DB): $(DATDATA) $(DBGENSCRIPT)
	cat $(DATDATA) \
	| sed -e 's/\[\(.*\)\]/\1/' \
	| sed -e 's/\([^=]*\)=.*$$/\1/' \
	| sort -n | uniq | $(DBGENSCRIPT) > $@

$(WIRES_DB): $(WIRES_M4_SCRIPT) $(WIRES_RAW_DB)
	m4 $^ > $@

$(WIRES_H_DB): $(WIRES_H_M4_SCRIPT) $(WIRES_RAW_DB)
	m4 $^ > $@

dist_noinst_SCRIPTS=$(V2GENSCRIPT)
dist_noinst_DATA=$(WIRES_RAW_DB) $(WIRES_DB) $(WIRES_H_DB)
virtex2_pkgdatadir=$(pkgdatadir)/virtex2
nobase_dist_virtex2_pkgdata_DATA=$(ALLDATA)

#Allow me to cleanup the mess
MAINTAINERCLEANFILES = $(V2CHIPDB)
