
Name:		libHX10
%define lname	libHX
Version:	1.15
Release:	0
Group:		System/Libraries
URL:		http://jengelh.hopto.org/projects/libHX/
Summary:	General-purpose library for typical low-level operations
License:	LGPL2 LGPL3 but NOT LATER
Source:		http://jengelh.hopto.org/files/libHX/libHX-%version.tar.bz2
BuildRoot:	%_tmppath/%name-%version-build
BuildRequires:	gcc-c++

%description
A library for:
- rbtree with key-value pair extension
- deques (double-ended queues) (Stacks (LIFO) / Queues (FIFOs))
- platform independent opendir-style directory access
- platform independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shconfig-style config file parser
- platform independent random number generator with transparent
  /dev/urandom support
- various string, memory and zvec ops

%package -n libHX-devel
Group:		Development/Libraries/C and C++
Summary:	Development files for libHX
Requires:	libHX10 = %version

%description -n libHX-devel
A library for:
- rbtree with key-value pair extension
- deques (double-ended queues) (Stacks (LIFO) / Queues (FIFOs))
- platform independent opendir-style directory access
- platform independent dlopen-style shared library access
- auto-storage strings with direct access
- command line option (argv) parser
- shconfig-style config file parser
- platform independent random number generator with transparent
  /dev/urandom support
- various string, memory and zvec ops

%debug_package
%prep
%setup -n %lname-%version

%build
%configure
make %{?jobs:-j%jobs};

%install
b="%buildroot";
rm -Rf "$b";
mkdir "$b";
make install DESTDIR="$b";
rm -f "$b/%_libdir/%lname.la";
mkdir -p "$b/%_docdir";
cp -a doc "$b/%_docdir/%lname";

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%_libdir/%{lname}*.so.*

%files -n libHX-devel
%defattr(-,root,root)
%_libdir/%{lname}*.so
%_libdir/pkgconfig/*
%_includedir/*
%docdir %_docdir/%lname
%_docdir/%lname
