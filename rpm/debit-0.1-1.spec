Summary: Reverse-engeneering tools for xilinx bitstreams
Name: debit
Version: 0.1
Release: 1
Vendor: Jean-Baptiste Note <jean-baptiste.note@m4x.org>
License: GPL
Group: Applications/Engineering
#BuildRequires:glib-devel
Source: ../%{name}-%{version}.tar.gz
Provides: %{name} = %{version}
BuildRoot: %{_tmppath}/%{name}-buildroot
#Requires: debit-data

#all this stuff allows in-tree, user building of the rpm
%define _topdir %(echo "$PWD")
%define _rpmtopdir %{_topdir}
%define _rpmdir %{_topdir}
%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
%define _unpackaged_files_terminate_build 0
%define _builddir %{_topdir}/BUILD

%description
This tool allows takes as input virtex-2 bitstreams and can output
pseudo-xdl. For now this is proof-of-concept code, you can contact me
if you want more detailed reverse-engeneering or other services.

%prep
%setup -q

%build
./configure
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install-exec DESTDIR=$RPM_BUILD_ROOT prefix=/usr

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%dir "/"
%dir "/usr/"
%dir "/usr/bin/"
/usr/bin/*

%changelog
* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build
