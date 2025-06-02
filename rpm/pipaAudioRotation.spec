Name:       pipaAudioRotation

Summary:    My Sailfish OS Application
Version:    0.1
Release:    1
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2
Requires:   pulseaudio
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Sensors)
BuildRequires:  desktop-file-utils
BuildRequires:  cmake
BuildRequires:  pulseaudio-devel
BuildRequires:  alsa-lib-devel
BuildRequires:  lipstick-qt5-devel
BuildRequires:  pkgconfig(mlite5)

%description
Short description of my Sailfish OS Application

%prep
%setup -q -n %{name}-%{version}

%build
%cmake

%make_build

%install
%make_install

mkdir -p %{buildroot}/usr/lib/systemd/user/user-session.target.wants/
ln -s ../pipa-audio-rotation.service %{buildroot}/usr/lib/systemd/user/user-session.target.wants/

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_prefix}/lib/systemd/user/pipa-audio-rotation.service
%{_prefix}/lib/systemd/user/user-session.target.wants/pipa-audio-rotation.service
