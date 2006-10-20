Summary: Reverse-engeneering tools for xilinx bitstreams
Name: debit
Version: 0.1
Release: 1
Vendor: Jean-Baptiste Note <jean-baptiste.note@m4x.org>
License: GPL
Group: Applications/Engineering
URL: http://www.ulogic.org/trac
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
# what's required to build from the dist tarball
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

%doc AUTHORS COPYING README
%{_bindir}/*
%{_datadir}/applications/%{name}.desktop
%{_datadir}/mime/packages/%{name}.xml
%{_datadir}/icons/*/*/*/*

%files data

%doc AUTHORS COPYING README
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/*
%{_mandir}/*/*

%changelog
* Fri Oct 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org> - 0.1-1
- Fix some rpmlint errors and warnings
W: debit no-version-in-last-changelog
W: debit no-url-tag
E: debit useless-explicit-provides debit
W: debit no-documentation
E: debit standard-dir-owned-by-package /usr/bin
E: debit standard-dir-owned-by-package /usr/share/icons
E: debit standard-dir-owned-by-package /usr
E: debit standard-dir-owned-by-package /
E: debit standard-dir-owned-by-package /usr/share
W: debit-data no-version-in-last-changelog
W: debit-data no-url-tag
W: debit-data no-documentation
W: debit-debuginfo no-version-in-last-changelog
W: debit-debuginfo no-url-tag

* Fri Sep 29 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Include desktop files

* Fri Sep 29 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Merge spec files, so as to be able to build with rpmbuild -tX

* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build

