#
# spec file for package xmradio 
#
# Copyright (c) 2012 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Name:           xmradio
Version:	0.3.0
Release:	0
License:	GPL-3.0+
Summary:	Xia Mi Radio
Url:	https://github.com/timxx/xmradio
Group:	Productivity/Multimedia/Sound/Players
Source:	%{name}-%{version}.tar.bz2
BuildRequires:	cmake
BuildRequires:	gcc-c++
BuildRequires:	gettext
BuildRequires:	intltool
BuildRequires:	pkg-config
BuildRequires:	zlib-devel
BuildRequires:	sqlite3-devel
BuildRequires:	libnotify-devel
BuildRequires:	glib2-devel
BuildRequires:	libcurl-devel
BuildRequires:	libxml2-devel
BuildRequires:	gtk3-devel
BuildRequires:	dbus-1-glib-devel
BuildRequires:	gstreamer-0_10-devel
BuildRequires:	gstreamer-0_10-plugins-base-devel
BuildRequires:	libpeas-devel
BuildRequires:	update-desktop-files
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
%glib2_gsettings_schema_requires

%description
Linux client of http://www.xiami.com/radio

%package devel
Summary:	Development files of xmradio
Group:	Development/Libraries/C and C++

%description devel
Linux client of http://www.xiami.com/radio


%prep
%setup -q

%build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DLIB_INSTALL_DIR=%{_libdir}\
      -DENABLE_APPINDICATOR=OFF \
	..
make %{?_smp_mflags}

%install
cd build
%make_install
cd ..

sed -i "s/Categories=GTK;AudioVideo;Network;/Categories=GTK;AudioVideo;Player;/" %{buildroot}%{_datadir}/applications/xmradio.desktop

%suse_update_desktop_file %{name} AudioVideo Player

%find_lang %{name}

%post
/sbin/ldconfig
%glib2_gsettings_schema_post

%postun
/sbin/ldconfig
%glib2_gsettings_schema_postun

%files -f %{name}.lang
%defattr(-,root,root)
%doc AUTHORS ChangeLog README.md COPYING
%{_bindir}/xmradio
%{_libdir}/libxmr*.so
%{_libdir}/libxmr*.so.*
%{_libdir}/xmradio/
%{_datadir}/applications/xmradio.desktop
%{_datadir}/glib-2.0/schemas/com.timxx.xmradio.gschema.xml
%{_datadir}/icons/hicolor/*/apps/xmradio.png
%{_datadir}/xmradio/
%exclude %{_libdir}/libxmrservice.so

%files devel
%defattr(-,root,root)
%{_includedir}/xmradio
%{_includedir}/xmrservice/
%{_libdir}/libxmrservice.so
%{_libdir}/pkgconfig/*.pc

%changelog

