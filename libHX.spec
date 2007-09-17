
Name:           libHX
Version:        1.10.1
Release:        jen0
Group:          System/Libraries
URL:            http://jengelh.hopto.org/p/%name/
Summary:        General-purpose library
License:        LGPL2 LGPL3 but NOT LATER
Source:         http://jengelh.hopto.org/f/%name/%name-%version.tar.bz2
BuildRoot:      %_tmppath/%name-%version-build
BuildRequires:	gcc-c++ perl

%description
A library for:
- A+R/B trees to use for lists or maps (associative arrays)
- Deques (double-ended queues) (Stacks (LIFO) / Queues (FIFOs))
- platform independent opendir-style directory access
- platform independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shell-style config file parser
- platform independent random number generator with transparent
  /dev/urandom support
- various string, memory and zvec ops

%package -n libHX-devel
Group:		Development/Libraries/C and C++
Summary:	Development files for libHX

%description -n libHX-devel
A library for:
- A+R/B trees to use for lists or maps (associative arrays)
- Deques (double-ended queues) (Stacks (LIFO) / Queues (FIFOs))
- platform independent opendir-style directory access
- platform independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shell-style config file parser
- platform independent random number generator with transparent
  /dev/urandom support
- various string, memory and zvec ops

%debug_package
%prep
%setup

%build
%configure
make %{?jobs:-j%jobs};
perl -i -pe 's/^shouldnotlink=yes/shouldnotlink=no/;' \
	src/%name.la src/.libs/%name.lai;

%install
b="%buildroot";
rm -Rf "$b";
mkdir "$b";
make install DESTDIR="$b";
rm -f "$b/%_libdir/%name.la";
mkdir -p "$b/%_docdir";
cp -a doc "$b/%_docdir/%name";

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%_libdir/%{name}*.so.*

%files -n libHX-devel
%defattr(-,root,root)
%_libdir/%{name}*.so
%_libdir/pkgconfig/*
%_includedir/%{name}*
%docdir %_docdir/%name
%_docdir/%name
