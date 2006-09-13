# This directory only contains data file

DATAPREFIX=clb tterm bterm rterm lterm lioi rioi tioi bioi \
	   bram ttermbram btermbram bioibram tioibram

DATCONTROL=$(addsuffix /control.db,$(DATAPREFIX))
DATDATA=$(addsuffix /data.db,$(DATAPREFIX))
CHIPDB=xc2v2000/chip_control xc2v2000/chip_data

WIRES_DB=wires.db

ALLDATA=$(DATCONTROL) $(DATDATA) $(WIRES_DB) $(CHIPDB)

nobase_dist_pkgdata_DATA=$(ALLDATA)

