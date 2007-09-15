dist_man1_MANS = altera.1 bit2pdf.1 bitisolation.1 debit.1 diff_bin_pos.1 xiledit.1 xdl2bit.1

EXTRA_DIST = altera.xml bit2pdf.xml bitisolation.xml debit.xml diff_bin_pos.xml xiledit.xml xdl2bit.xml \
	     seealso.xml entryinfo.xml
MAINTAINERCLEANFILES = $(dist_man1_MANS)

%.1: %.xml seealso.xml entryinfo.xml
	xmlto man $<