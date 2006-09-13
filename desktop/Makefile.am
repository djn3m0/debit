# This directory only contains data file
SUBDIRS=artwork

###############################
# fdo entries and mime things #
###############################

# inspired by GLOM

# fdo .desktop file
desktopdir = $(datadir)/applications
dist_desktop_DATA = debit.desktop

# MIME registry
MIMEdir = $(datadir)/mime/packages
dist_MIME_DATA = debit.xml

# Icons are in artwork

install-data-hook:
	-update-mime-database $(datadir)/mime

