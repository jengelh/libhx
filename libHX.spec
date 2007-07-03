
Name:           libHX
Version:        1.10.0
Release:        jen0
Group:          System/Libraries
URL:            http://jengelh.hopto.org/f/%name/
Summary:        General-purpose library
License:        LGPL2
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

%debug_package
%prep
%setup

%build
./autogen.sh;
%configure
make %{?jobs:-j%jobs};
perl -i -pe 's/^shouldnotlink=yes/shouldnotlink=no/;' \
	src/%name.la src/.libs/%name.lai;

%install
b="%buildroot";
rm -Rf "$b";
mkdir "$b";
make install DESTDIR="$b";
mkdir -p "$b/%_docdir";
cp -a doc "$b/%_docdir/%name";

%post
%run_ldconfig

%postun
%run_ldconfig

%files
%defattr(-,root,root)
%attr(0644,root,root) %_libdir/%name.la
%_libdir/%{name}*.so*
%_includedir/%{name}*
%docdir %_docdir/%name
%_docdir/%name

%changelog -n libHX
* Tue Feb 27 2007 - jengelh
- BT_MAXDEP was set too low, crashing if more than 512K objects were in a tree
