DATAPREFIX=int

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))

# Virtex 5 chip database
V5CHIPS=30 85 110 220 330
V5CHIPDIRS=$(addprefix xc5vlx, $(V5CHIPS))
V5CHIPDB=$(addsuffix /chip_control, $(V5CHIPDIRS)) $(addsuffix /chip_data, $(V5CHIPDIRS))
V5GENSCRIPT=generate_v5_chip.pl

WIRES_RAW_DB=wires.m4
WIRES_DB=wires.db

DBGENSCRIPT=$(srcdir)/../wires_to_db.pl
WIRES_M4_SCRIPT=$(srcdir)/../dbtoini.m4

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB) $(V5CHIPDB)

$(V5CHIPDB): $(V5GENSCRIPT)
	$(srcdir)/$<

$(WIRES_RAW_DB): $(DATDATA) $(DBGENSCRIPT)
	cat $(DATDATA) \
	| sed -e 's/\[\(.*\)\]/\1/' \
	| sed -e 's/\([^=]*\)=.*$$/\1/' \
	| sort -n | uniq | $(DBGENSCRIPT) > $@

$(WIRES_DB): $(WIRES_M4_SCRIPT) $(WIRES_RAW_DB)
	m4 $^ > $@

dist_noinst_SCRIPTS=$(V5GENSCRIPT) $(WIRES_RAW_DB)
virtex5_pkgdatadir=$(pkgdatadir)/virtex5
nobase_dist_virtex5_pkgdata_DATA=$(ALLDATA)

#Allow me to cleanup the mess
MAINTAINERCLEANFILES = $(V5CHIPDB)
