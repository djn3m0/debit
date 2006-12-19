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

WIRES_DB=wires.db
V2DBGENSCRIPT=wires_to_db.pl

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB) $(V2CHIPDB)

$(V2CHIPDB): $(V2GENSCRIPT)
	$(srcdir)/$<

$(WIRES_DB): $(DATDATA) $(V2DBGENSCRIPT)
	cat $(DATDATA) \
	| sed -e 's/\[\(.*\)\]/\1/' \
	| sed -e 's/\([^=]*\)=.*$$/\1/' \
	| sort -n | uniq | $(srcdir)/$(V2DBGENSCRIPT) > $@

dist_noinst_SCRIPTS=$(V2GENSCRIPT) $(V2DBGENSCRIPT)
nobase_dist_pkgdata_DATA=$(ALLDATA)

#Allow me to cleanup the mess
MAINTAINERCLEANFILES = $(V2CHIPDB)
