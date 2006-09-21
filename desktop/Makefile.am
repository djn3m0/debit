# This directory only contains data file
SUBDIRS=artwork

###############################
# fdo entries and mime things #
###############################

# inspired by GLOM

# fdo .desktop file
desktopdir = $(datadir)/applications
dist_desktop_DATA = debit.desktop

# Check target ?
# desktop-file-validate $(dist_desktop_DATA)

# MIME registry
MIMEdir = $(datadir)/mime/packages
dist_MIME_DATA = debit.xml

# Icons are in artwork

# How to run this without the package integrating this ?
# This should pertain to the package, not here, it clobbers make distcheck & packaging
install-data-hook:
	-echo "You should run update-mime-database $(datadir)/mime for file association to take effect"

