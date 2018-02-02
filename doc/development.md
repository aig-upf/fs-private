
# Development Notes

## Code Format

At the moment we're trying to enforce some code style rules for our the C++ code through `clang-format`.
For that, the root dir of the project contains a `.clang-format` file which specifies
the format rules we want to enforce. We are currently using clang-format 5.0, which is relevant,
as the style option names change quite often from version to version. All of the available options
can be found [here](http://releases.llvm.org/5.0.0/tools/clang/docs/ClangFormatStyleOptions.html).
A good option is to integrate format checks with whatever IDE / editor you are using, and then setting
some keyboard shortcut to apply it. There are instructions for a few IDEs
[here](https://github.com/KratosMultiphysics/Kratos/wiki/How-to-configure-clang%E2%80%90format).
Another option is to run it from the command line, either on one single file:

    clang-format -i -style=file core.cpp 

Or on the entire source tree of the project:

    find src -name '*.hxx' -or -name '*.cxx' | xargs clang-format -i -style=file $1

Of course, you should check in any case that the modified source looks good before committing
anything :-)

The current style book is still pretty much a work in progress, with many rules still up for 
discussion / adjustment.