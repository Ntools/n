Name:		eds
Summary:	n Editor for C
Version:	0.9.2
Release:	1
Group:		Applications/Editors
License:	BSD
Vendor:		ntools opensource projects
URL:		http://www.ntools.net/
Source:		http://www.ntools.net/pub/editors/eds-%{version}.tar.gz
BuildRoot:	/var/tmp/%{name}-root

Requires:	slang

%description
n is an updated and improved version of the n editor for dos.

%prep
%setup -q 

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_prefix}/bin
mkdir -p $RPM_BUILD_ROOT%{_prefix}/local/etc
install -c -m 755 n $RPM_BUILD_ROOT%{_prefix}/bin
install -c -m 644 keys.ini $RPM_BUILD_ROOT%{_prefix}/local/etc

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_prefix}/bin/n
%{_prefix}/local/etc/keys.ini

%changelog
* Mon Sep 9 2013 Nobby N Hirano <nobby@ntools.net>
- Build !
