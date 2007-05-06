### -*- Mode: Makefile -*-
# Common build rules for databases
###

####
# Implicit pip database
####
IMPLDATA=$(addsuffix /implicit.db,$(IMPLPREFIX))

####
# Pips database
####

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))
DATGENERAL=$(addsuffix /all.db,$(DATAPREFIX))
PIPS_ALL_DBS=pipsdb.h

%control.db: ../piptoctrlini.m4 %all.db
	m4 $^ > $@

%data.db: ../piptodataini.m4 %all.db
	m4 $^ > $@

PIPS_H_DB=pips.h
PIPS_M4_SCRIPT=../pips_all_compiled.m4

$(PIPS_H_DB): $(PIPS_M4_SCRIPT) $(WIRES_H_DB) $(PIPS_ALL_DBS)
	m4 -I$(top_srcdir) -DSITEDB=$(SITE_INCL) -DPIPSDB=$(PIPS_ALL_DBS) -DWIREDB=$(WIRES_H_DB) $< > $@

####
# Logic database
####
LOGICONTROL=$(addsuffix /lcontrol.db,$(LOGIPREFIX))
LOGIDATA=$(addsuffix /ldata.db,$(LOGIPREFIX))
#DATGENERAL=$(addsuffix /all.db,$(LOGIPREFIX))

####
# Chip geometry database
####

CHIPDIRS=$(addprefix $(CHIP_PFX), $(CHIPS))
CHIPDB=$(addsuffix /chip_control, $(CHIPDIRS)) $(addsuffix /chip_data, $(CHIPDIRS))

$(CHIPDB): $(GENSCRIPT)
	$(srcdir)/$<

####
# Wiring database
####

WIRES_RAW_DB=wires.m4
WIRES_H_DB=wires.h
WIRES_DB=wires.db

DBGENSCRIPT ?= $(srcdir)/../wires_to_db.pl
FLIPCHIP    ?= 0
WIRES_M4_SCRIPT=$(srcdir)/../dbtoini.m4
WIRES_H_M4_SCRIPT=$(srcdir)/../dbtoh.m4

$(WIRES_RAW_DB): $(DATDATA) $(LOGIDATA) $(IMPLDATA) $(DBGENSCRIPT)
	cat $(DATDATA) $(LOGIDATA) $(IMPLDATA) \
	| sed -e 's/\[ *\([^ ]*\) *\]/\1/' \
	| sed -e 's/EP=\(.*\)$$/\1/' \
	| sed -e 's/\([^=]*\)=.*$$/\1/' \
	| grep -v '^$$' \
	| sort -n | uniq | $(DBGENSCRIPT) $(FLIPCHIP) > $@

$(WIRES_DB): $(WIRES_M4_SCRIPT) $(WIRES_RAW_DB)
	m4 $^ > $@

$(WIRES_H_DB): $(WIRES_H_M4_SCRIPT) $(WIRES_RAW_DB)
	m4 $^ > $@

####
# Autotools tracing of all this stuff
####

ALLDATA=$(DATCONTROL) $(DATDATA) $(LOGIDATA) $(LOGICONTROL) $(WIRES_DB) $(CHIPDB)
ALLDDATA=$(WIRES_RAW_DB) $(WIRES_H_DB) $(PIPS_H_DB) $(PIPS_ALL_DBS) $(DATGENERAL)

dist_noinst_SCRIPTS=$(GENSCRIPT)
dist_noinst_DATA=$(ALLDDATA)
virtex2_pkgdatadir=$(pkgdatadir)/virtex2
nobase_dist_virtex2_pkgdata_DATA=$(ALLDATA)

#Allow me to cleanup the mess
MAINTAINERCLEANFILES = $(CHIPDB) $(WIRES_RAW_DB) $(WIRES_H_DB) \
			$(DATCONTROL) $(DATDATA) $(PIPS_H_DB)
