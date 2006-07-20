# This directory only contains data file

DATAPREFIX=clb tterm bterm rterm lterm lioi rioi tioi bioi \
	   bram ttermbram btermbram bioibram tioibram

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))

WIRES_DB=wires.db

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB)

nobase_dist_pkgdata_DATA=$(ALLDATA)
