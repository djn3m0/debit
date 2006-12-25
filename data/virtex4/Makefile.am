# This directory only contains data file

DATAPREFIX=int

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))

# Virtex 4 chipd database
V4CHIPS=15 25 40 60 80 100 160 200
V4CHIPDIRS=$(addprefix xc4vlx, $(V4CHIPS))
V4CHIPDB=$(addsuffix /chip_control, $(V4CHIPDIRS)) $(addsuffix /chip_data, $(V4CHIPDIRS))
V4GENSCRIPT=generate_v4_chip.pl

WIRES_DB=wires.db
DBGENSCRIPT=$(srcdir)/../wires_to_db.pl

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB) $(V4CHIPDB)

$(V4CHIPDB): $(V4GENSCRIPT)
	$(srcdir)/$<

$(WIRES_DB): $(DATDATA) $(DBGENSCRIPT)
	cat $(DATDATA) \
	| sed -e 's/\[\(.*\)\]/\1/' \
	| sed -e 's/\([^=]*\)=.*$$/\1/' \
	| sort -n | uniq | $(DBGENSCRIPT) > $@

dist_noinst_SCRIPTS=$(V4GENSCRIPT)
virtex4_pkgdatadir=$(pkgdatadir)/virtex4
nobase_dist_virtex4_pkgdata_DATA=$(ALLDATA)

#Allow me to cleanup the mess
MAINTAINERCLEANFILES = $(V4CHIPDB)
