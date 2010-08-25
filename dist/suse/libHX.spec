
Name:		libHX
%define lname	libHX25
Version:	3.6
Release:	0
Group:		System/Libraries
URL:		http://libhx.sf.net/
Summary:	Useful collection of routines for C and C++ programming
License:	LGPLv2+
Source:		http://downloads.sf.net/libhx/libHX-%version.tar.xz
Source9:	baselibs.conf
BuildRoot:	%_tmppath/%name-%version-build
BuildRequires:	gcc-c++, pkg-config, xz
# no, libxml2-devel is NOT required because nothing
# that requires it is going to be compiled.
# gcc-c++ is pretty optional and only used for make check 

%define debug_package_requires %lname = %version-%release

%description
A library for:
- hash/rbtree-based maps/sets
- double-ended queues (stacks/fifos/lists)
- platform-independent opendir-style directory access
- platform-independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shconfig-style config file parser
- various string, memory and zvec ops
- more

Author(s):
----------
	Jan Engelhardt

%package -n %lname
Group:		System/Libraries
Summary:	Useful collection of routines for C and C++ programming

%description -n %lname
A library for:
- hash/rbtree-based maps/sets
- double-ended queues (stacks/fifos/lists)
- platform-independent opendir-style directory access
- platform-independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shconfig-style config file parser
- various string, memory and zvec ops
- more

Author(s):
----------
	Jan Engelhardt

%package devel
Group:		Development/Libraries/C and C++
Summary:	Development files for libHX
Requires:	%lname = %version
%if "%{?vendor_uuid}" != ""
Provides:	libHX-devel(vendor:%vendor_uuid) = %version-%release
%endif

%description -n libHX-devel
A library for:
- hash/rbtree-based maps/sets
- double-ended queues (stacks/fifos/lists)
- platform-independent opendir-style directory access
- platform-independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shconfig-style config file parser
- various string, memory and zvec ops
- more

Author(s):
----------
	Jan Engelhardt

%prep
%setup -q

%build
if [ ! -e configure ]; then
	./autogen.sh
fi;
%configure
make %{?_smp_mflags}

%install
b="%buildroot"
rm -Rf "$b"
mkdir "$b"
make install DESTDIR="$b" docdir="%_docdir/%name"
rm -f "$b/%_libdir/%name.la"
mkdir -p "$b/%_docdir/%name"
install -pm0644 doc/*.txt "$b/%_docdir/%name/"

%check
make check

%post -n %lname -p /sbin/ldconfig

%postun -n %lname -p /sbin/ldconfig

%files -n %lname
%defattr(-,root,root)
%_libdir/%{name}*.so.*

%files devel
%defattr(-,root,root)
%_libdir/%{name}*.so
%_libdir/pkgconfig/*
%_includedir/*
%docdir %_docdir/%name
%_docdir/%name

%changelog
