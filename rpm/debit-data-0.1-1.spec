Summary: Data files for debit
Name: debit-data
Version: 0.1
Release: 1
Vendor: Jean-Baptiste Note <jean-baptiste.note@m4x.org>
Copyright: GPL
Group: Applications/Engineering
Source: ../debit-%{version}.tar.gz
Provides: %{name} = %{version}
BuildRoot: %{_tmppath}/%{name}-buildroot

#all this stuff allows in-tree, user building of the rpm
%define _topdir %(echo "$PWD")
%define _rpmtopdir %{_topdir}
%define _rpmdir %{_topdir}
%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
%define _unpackaged_files_terminate_build 0
%define _builddir %{_topdir}/BUILD

%description
The data files for debitting. These are architecture-independent. How
to generate them is kept secret for now.

%prep
%setup -q -n debit-%{version}

%build
./configure
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install-data DESTDIR=$RPM_BUILD_ROOT prefix=/usr

%clean
rm -rf $RPM_BUILD_ROOT

%files
%dir "/"
%dir "/usr/"
%dir "/usr/share/"
%dir "/usr/share/debit/"
/usr/share/debit/*

%changelog
* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build
