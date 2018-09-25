
# pyfs - The FS Planner Python Module


## Requirements

* Tarski
*

## Development notes

ATM the following libraries need to be on the `$LD_LIBRARY_PATH`:
* Boost (see notes below)


### Compiling Boost

Boost needs to be especifically configured to target the desired version
of Python and make sure that we're not using Python 2.x

```
./bootstrap.sh --with-python=/usr/bin/python3 --with-python-root=/usr
```

the above configured correctly ```Boost.build``` tool ```b2``` in my 16.04
system. Specifying ```--with-python-version=3.5``` resulted in an invalid
configuration, as Python interaces were compiling against ```libpython2```.
The build command used was

```
./b2 --build-dir=/tmp/build-boost --enable-unicode=ucs4 stage
```

Note that the UCS4 implementation of unicode used by Python 3.x. This I spotted
on [Stackoverflow](https://stackoverflow.com/questions/28830653/build-boost-with-multiple-python-versions)

> Also, be aware that Boost.Python and user code may need recompiled to match
the Python's CPU architecture and UCS-2 or UCS-4 unicode configurations.

and this [gist](https://gist.github.com/melvincabatuan/a5a4a10b15ef31a5a481).
