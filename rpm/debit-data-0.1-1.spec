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
%dir "/usr/share/debit/clb/"
"/usr/share/debit/clb/control.db"
"/usr/share/debit/clb/data.db"
%dir "/usr/share/debit/tterm/"
"/usr/share/debit/tterm/control.db"
"/usr/share/debit/tterm/data.db"
%dir "/usr/share/debit/bterm/"
"/usr/share/debit/bterm/control.db"
"/usr/share/debit/bterm/data.db"
%dir "/usr/share/debit/rterm/"
"/usr/share/debit/rterm/control.db"
"/usr/share/debit/rterm/data.db"
%dir "/usr/share/debit/lterm/"
"/usr/share/debit/lterm/control.db"
"/usr/share/debit/lterm/data.db"
%dir "/usr/share/debit/lioi/"
"/usr/share/debit/lioi/control.db"
"/usr/share/debit/lioi/data.db"
%dir "/usr/share/debit/rioi/"
"/usr/share/debit/rioi/control.db"
"/usr/share/debit/rioi/data.db"
%dir "/usr/share/debit/tioi/"
"/usr/share/debit/tioi/control.db"
"/usr/share/debit/tioi/data.db"
%dir "/usr/share/debit/bioi/"
"/usr/share/debit/bioi/control.db"
"/usr/share/debit/bioi/data.db"
%dir "/usr/share/debit/bram/"
"/usr/share/debit/bram/control.db"
"/usr/share/debit/bram/data.db"
%dir "/usr/share/debit/ttermbram/"
"/usr/share/debit/ttermbram/control.db"
"/usr/share/debit/ttermbram/data.db"
%dir "/usr/share/debit/btermbram/"
"/usr/share/debit/btermbram/control.db"
"/usr/share/debit/btermbram/data.db"
%dir "/usr/share/debit/bioibram/"
"/usr/share/debit/bioibram/control.db"
"/usr/share/debit/bioibram/data.db"
%dir "/usr/share/debit/tioibram/"
"/usr/share/debit/tioibram/control.db"
"/usr/share/debit/tioibram/data.db"
"/usr/share/debit/wires.db"

%changelog
* Thu Jul 20 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
- Initial rpm build
