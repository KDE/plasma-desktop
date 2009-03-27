%define name		fcitx
%define version		3.6.0rc
%define release		1

%define prefix		/usr
%define fcitxbindir	/usr/bin

Name:		%{name}
Version:	%{version}
Release:	%{release}
Summary:	Free Chinese Input Toy for X (XIM)
Packager:	Xie Yanbo <xyb76@sina.com>
URL:		http://www.fcitx.org/
Group:		User Interface/X
Group(zh_CN):	用户界面/桌面
License:	GPL
Source:		%{name}-%{version}.tar.gz
#BuildRequires:	XFree86, fontconfig
BuildRequires:	XFree86-devel
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}

%description
FCITX is a simplified Chinese input server. It supports Tables,
Pinyin and QuWei input method. It's small and fast.

%description -l zh_CN
Fcitx──小企鹅输入法即Free Chinese Input Toy for X，它是一个以GPL方式
发布的、基于XIM的简体中文输入法(即原来的G五笔)，包括码表、全拼拼音、双拼
拼音输入法,并可运行在Linux及其它类UNIX平台上。
Designed by Yuking <yuking_net@suho.com>

%prep
%setup -q

%build
%ifarch i386 i486 i586 i686
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS"		./configure --prefix=%{prefix} 	--host=i386-pc-linux-gnu
%else
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS"		./configure --prefix=%{prefix}
%endif
make

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL README
%doc doc/*.txt doc/*.htm
%{_bindir}/*
%dir %{_datadir}/fcitx/data
%{_datadir}/fcitx/data/*.mb
%{_datadir}/fcitx/data/*.dat
%{_datadir}/fcitx/data/tables.conf

%changelog
* Fri Jun 11 2004 xyb <xyb76@sina.com>
- Add data/tables.conf

* Mon Feb 2 2004 xyb <xyb76@sina.com>
- Fix spec bug(patch by hamigua <hamigua@linuxsir.org>).

* Thu Jan 15 2004 xyb <xyb76@sina.com>
- skeleton RPM
