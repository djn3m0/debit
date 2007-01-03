# Files which will be distributed
svg_artwork = ulogic_banner.svg ulogic_logo.svg ulogic_withname.svg ulogic_icon.svg ulogic_splash.svg

# Files which can be rebuilt
# We also distribute them, as some special fonts are needed to generate them properly
# We provide entries for various resolutions.
icons48dir   = $(datadir)/icons/hicolor/48x48/apps
dist_icons48_DATA = ulogic_icon_48.png
icons32dir   = $(datadir)/icons/hicolor/32x32/apps
dist_icons32_DATA = ulogic_icon_32.png

mimicons48dir = $(datadir)/icons/hicolor/48x48/mimetypes
dist_mimicons48_DATA = application-x-xilinx-bitstream.png application-x-altera-bitstream.png

# How to rebuild them
%_48.png: %.svg
	inkscape -z -f $< -e $@ -D -w48 -h48

%_32.png: %.svg
	inkscape -z -f $< -e $@ -D -w32 -h32

# Build Windows & NSIS - specific artwork
win_artwork = ulogic_icon.ico \
              application-x-xilinx-bitstream.ico application-x-altera-bitstream.ico \
	      ulogic_banner.bmp ulogic_splash.bmp \
              ulogic_installer.ico ulogic_uninstaller.ico

website_trac_banner.png: website_trac_banner.svg
	inkscape -z -f $< -e $@ -D -w319 -h73 -bwhite -y0.0

# These are the dimentions that are expected by NSIS
%_banner.png: %_banner.svg
	inkscape -z -f $< -e $@ -D -w150 -h57

%_splash.png: %_splash.svg
	inkscape -z -f $< -e $@ -D -w300

%.ppm: %.png
	convert $< $@

%.bmp: %.ppm
	ppmtobmp -windows $< > $@

# for windows
%.ico: %_48.png %_32.png
	icotool -c $^ -o $@

# Default icon size
%.png: %.svg
	inkscape -z -f $< -e $@ -D -w48 -h48

dist_noinst_DATA=$(svg_artwork) $(win_artwork)

#And allow me to cleanup the mess
MAINTAINERCLEANFILES = $(win_artwork) $(dist_icons48_DATA) $(dist_icons32_DATA) $(dist_mimicons48_DATA)
