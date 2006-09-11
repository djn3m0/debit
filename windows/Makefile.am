xildb.nsh:
	$(srcdir)/gendb.sh $(top_srcdir) $@

debit.nsi: debit.nsi.tmpl xildb.nsh
	cp $< $@

#For me makensis is just a bash stub calling makensis.exe under wine
MAKENSIS=makensis

win_top_srcdir=Z:\\$(shell echo $(top_srcdir) | sed -e 's:/:\\\\:g')
version=$(shell cat $(top_srcdir)/git-ref)

debit-$(version).exe: debit.nsi
	$(MAKENSIS) /DVERSION=$(version) /DSRCDIR=$(win_top_srcdir) $<

installer: debit-$(version).exe
