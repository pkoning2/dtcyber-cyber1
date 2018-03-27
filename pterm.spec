Summary: PLATO terminal emulator
Name: pterm
Version: 6.0.2
Release: 1
License: DtCyber
Group: User Interface/Desktops
URL: http://www.cyber1.org
Packager: Paul Koning <paul@cyber1.org>
Source: /lhome/pkoning/pterm-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: SDL-devel python
Requires: libsndfile

%description
The pterm program emulates a PLATO terminal (at the moment the
"PLATO V" a.k.a. PPT, without the loadable program capability),
for use with the cyber1 CYBIS (PLATO) system.
%prep
%setup -q

%build
make pterm TYPE=static ARCHCFLAGS="-m32" ARCHLDFLAGS="-m32"
make mofiles

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/locale/nl/LC_MESSAGES

install -s -m 755 pterm $RPM_BUILD_ROOT/usr/bin/pterm
install -m 644 nl/pterm.mo $RPM_BUILD_ROOT/usr/share/locale/nl/LC_MESSAGES/pterm.mo

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc

/usr/bin/pterm
/usr/share/locale/nl/LC_MESSAGES/pterm.mo

%changelog
* Fri Apr 29 2005 Paul Koning <paul@cyber1.org> - 
- Initial build.

