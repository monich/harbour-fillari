Name:           harbour-fillari

Summary:        Helsinki city bike app
Version:        1.0.3
Release:        1
License:        BSD
URL:            https://github.com/monich/harbour-fillari
Source0:        %{name}-%{version}.tar.gz

Requires:       sailfishsilica-qt5
Requires:       qt5-qtsvg-plugin-imageformat-svg

BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  qt5-qttools-linguist

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

%description
Helsinki city bike app

%if "%{?vendor}" == "chum"
Categories:
 - Other
Icon: https://raw.githubusercontent.com/monich/harbour-fillari/master/icons/harbour-fillari.svg
Screenshots:
- https://home.monich.net/chum/harbour-fillari/screenshots/screenshot-001.png
- https://home.monich.net/chum/harbour-fillari/screenshots/screenshot-002.png
- https://home.monich.net/chum/harbour-fillari/screenshots/screenshot-003.png
- https://home.monich.net/chum/harbour-fillari/screenshots/screenshot-004.png
Url:
  Homepage: https://github.com/monich/harbour-fillari
%endif

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 %{name}.pro
%qtc_make %{?_smp_mflags}

%install
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
