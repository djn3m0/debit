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

#%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
#%define _unpackaged_files_terminate_build 0

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

%files
%defattr(-,root,root)
%dir "/"
%dir "/usr/"
%dir "/usr/bin/"
/usr/bin/*

%files data
%dir "/"
%dir "/usr/"
%dir "/usr/share/"
%dir "/usr/share/debit/"
/usr/share/debit/*

%changelog
* Fri Sep 29 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Merge spec files, so as to be able to build with rpmbuild -tX

* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build

