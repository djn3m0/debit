SUBDIRS = scripts

scriptutils=convert.sh xcompile.sh gendb.sh
products=debit-installer.exe debit.nsi xildb.nsh
EXTRA_DIST=debit.nsi.tmpl cross.env $(scriptutils)
CLEANFILES=$(products)

xildb.nsh: $(srcdir)/gendb.sh
	$(srcdir)/gendb.sh $(top_srcdir) $@

debit.nsi: debit.nsi.tmpl xildb.nsh
	cp $< $@

#For me makensis is just a bash stub calling makensis.exe under wine
MAKENSIS=makensis
#This should be pretty standard for wine installations
win_top_srcdir=Z:\\$(shell echo $(top_srcdir) | sed -e 's:/:\\\\:g')

debit-installer.exe: debit.nsi
	$(MAKENSIS) /DVERSION=$(version) /DSRCDIR=$(win_top_srcdir) $<
