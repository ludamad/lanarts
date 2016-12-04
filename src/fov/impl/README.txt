permissive-fov Library Version 1.0

Copyright (c) 2007, Jonathon Duerig

This library is licensed under the BSD license. See LICENSE.txt for details.

--------
Overview
--------

This library provides an implementation of precise permissive field of
view calculations on coarse-grained maps. A map is a grid where
squares are either completely open or completely obstructed.

Permissive field of view means that a destination square is visible
from a source square if and only if there exists an unobstructed line
from some point in the source square to some point in the destination
square. This is equivalent to finding the shadows cast by an area (as
opposed to point) light source.

Precise field of view means that the algorithm does not make any
approximations, nor does it make use of floating point numbers. This
means that the algorithm produces (assuming it is bug-free) will
produce neither inaccuracies nor anomalies. It also means that there
will be no possibility of a rounding error.

The algorithm has a complexity of approximately O(n^2) where n is the
radius. I say approximately, because it is difficult to come up with a
worst case scenario. Generally, it will tend to perform worse on open
areas and better on cramped areas. On a completely open area, it
performs at exactly O(n^2).

--------
Manifest
--------

LICENSE.txt -- A formal declaration of the BSD license used for this library.

README.txt -- You are here.

permissive-fov.h -- C bindings for the algorithm. Note that while this
                    library can be used by a pure C program, the
                    library itself is in C++. Therefore a C++ compiler
                    on the target platform is required. Also,
                    libstdc++ must be linked into the final
                    executable. When using the C bindings, the user is
                    responsible for type safety.

permissive-fov-cpp.h -- C++ bindings for the algorithm. This provides
                        a type-safe and memory-safe wrapper for the C
                        bindings. These should be preferred to the C
                        bindings when using C++.

permissive-fov.cc -- The C++ implementation file.

example.mask -- An example file that shows what mask files should look
                like. This kind of mask might be used for the LoS of a
                limited-range torch.

-----
Usage
-----

At present the library is too simple to be pulled into a dynamic
library. To use it, just pull the source files into your project using
whatever platform-dependent tools you have. If things become more
complicated, then I'll add a makefile/solution/pre-compiled library.

Here is a quick high level overview of what the functions in the
library do. See permissive-fov.h and permissive-fov-cpp.h for a more
complete description.

C:   permissiveSquareFov()
C++: permissive::squareFov()
  This is the simplest function for calculating fov. It is used in the
  common case where a simple square centered on the source needs to be
  calculated. When first testing the system, use this function to make
  sure that everything works before starting to use masks.


C:   permissiveFov()
C++: permissive::fov()
  This function is identical to permissiveSquareFov() except that
  visitation masks are used rather than a simple radius. A visitation
  mask determines whether the user-supplied visit() function will be
  called on particular squares. Masks can be asymmetric, and they need
  not be contiguous.

C:   initPermissiveMask()
     loadPermissiveMask()
     cleanupPermissiveMask()
     savePermissiveMask()
     setPermissiveVisit()
     clearPermissiveVisit()
     doesPermissiveVisit()
C++: class permissive::maskT
  These functions are used to create, manipulated, and destroy
  masks. initPermissiveMask() and loadPermissiveMask() are used to
  initialize a mask. Only one of those two functions should be
  used. cleanupPermissiveMask() destroys an initialized mask. The
  loadPermissiveMask() and savePermissiveMask() functions allow you to
  edit a mask in a text file and then load that mask into your
  program. {set,clear,does}PermissiveVisit allow you to
  programatically manipulate a mask. permissive::maskT exports these
  functions in the form of a class for your convenience and memory
  safety.

----
TODO
----

* More robust error handling.
* Cleanly separate C error paths from C++ error paths.
* Stricter parsing of mask files.
