Summary: Reverse-engeneering tools for xilinx bitstreams
Name: debit
Version: 0.1
Release: 1
Vendor: Jean-Baptiste Note <jean-baptiste.note@m4x.org>
License: GPL
Group: Applications/Engineering
Source: %{name}-%{version}.tar.gz
Provides: %{name} = %{version}
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
# We're not including here utils needed to rebuild the whole bunch of files,
# only what's required to build from the dist tarball
BuildRequires: gcc
BuildRequires: pkgconfig
BuildRequires: glib-devel
BuildRequires: cairo-devel
BuildRequires: gtk2-devel

%description
This tool allows takes as input virtex-2 bitstreams and can output
pseudo-xdl. For now this is proof-of-concept code, you can contact me
if you want more detailed reverse-engeneering or other services.

%package data
Summary: Data files for debit
Group: Applications/Engineering

%description data
The data files for debitting. These are architecture-independent. How
to generate them is kept secret for now.

%prep
%setup -q -n debit-%{version}

%build
%configure
make RPM_OPT_FLAGS="%{optflags}"

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
make install DESTDIR=%{buildroot} prefix=/usr

%clean
rm -rf $RPM_BUILD_ROOT

%post
update-desktop-database %{_datadir}/applications || :
update-mime-database %{_datadir}/mime || :

%postun
update-desktop-database %{_datadir}/applications || :
update-mime-database %{_datadir}/mime || :

%files
%defattr(-,root,root)
%dir "/"
%dir "/usr/"
%dir "/usr/bin/"
/usr/bin/*

%dir %{_datadir}/applications/
%{_datadir}/applications/%{name}.desktop

%dir %{_datadir}/mime
%dir %{_datadir}/mime/packages
%{_datadir}/mime/packages/%{name}.xml

%dir %{_datadir}
%dir %{_datadir}/icons/
%dir %{_datadir}/icons/hicolor/
%dir %{_datadir}/icons/hicolor/48x48/
%dir %{_datadir}/icons/hicolor/48x48/apps/
%{_datadir}/icons/hicolor/48x48/apps/*

%dir %{_datadir}/icons/hicolor/32x32/
%dir %{_datadir}/icons/hicolor/32x32/apps/
%{_datadir}/icons/hicolor/32x32/apps/*

%dir %{_datadir}/icons/hicolor/48x48/mimetypes
%{_datadir}/icons/hicolor/48x48/mimetypes/*

%files data
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/*

%changelog
* Fri Sep 29 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Include desktop files

* Fri Sep 29 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Merge spec files, so as to be able to build with rpmbuild -tX

* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build

