
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

**NOTE**: I had to let know ```b2``` that I wanted a build with ```C++11``` features
enabled. I used this command

```
echo "using gcc : 5.4 : /the/path/to/g++-5.4 : <cxxflags>-std=c++11 ;" > ./tools/build/src/user-config.jam
```

as per [this advice on Stackoverflow](https://stackoverflow.com/a/43716427/338107).

**NOTE #2**: if the client code is being compiled with c++-14 support (as is the case) for
```FS``` then it is safest to compiled the boost libraries also with c++-14 enabled. For that, instead
of the command above use instead


```
echo "using gcc : 5.4 : /the/path/to/g++-5.4 : <cxxflags>-std=c++14 ;" > ./tools/build/src/user-config.jam
```

See [here](https://github.com/boostorg/system/issues/24) for more details on the issue (which is acknowledged by boost as bug).

### Building and Deployment pyfs

To compile (debug mode for the moment)

```
.../pyfs$ CC="ccache gcc" python setup.py build --debug   
```

we recommend to use ```ccache``` to avoid unnecessary re-compilation.

Once built, the module can be installed with ```pip``` in dev install mode

```
.../pyfs$ pip install -e .
```

To test that the previous two steps worked out as expected, you may run this example

```
.../pyfs/examples$ python blocks.py
```
