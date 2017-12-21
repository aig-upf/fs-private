
FS planner - Manual Installation
=================================

The following installation steps should build the planner from source. They assume a working
`Ubuntu` environment and have been tested with the `16.04LTS` release, although they probably work with 
more recent releases as well. Support for other platforms is currently not available.


## 1. Install basic packages

To begin with, you will need a few basic build packages:

```
sudo apt-get install --no-install-recommends \
     build-essential g++ python3 git scons libboost-all-dev pkg-config
```


## 2. Install Gecode

At the moment, the planner requires a working installation of the [Gecode CSP Solver](http://www.gecode.org/).
The source code is ensured to work with `Gecode 5.1.0`; older versions might work as well, but we have not tested
them. Some Linux distributions provide precompiled binaries, but some of them package older Gecode versions only.
Installing Gecode [from source](http://www.gecode.org/download/gecode-5.1.0.tar.gz) is however not too difficult;
detailed instructions can be found on the excellent [documentation of the toolkit](http://www.gecode.org/doc-latest/MPG.pdf).
The following command will for instance perform an installation of the Gecode modules that are necessary for the planner
on `~/local`, for which no root permissions are necessary:

```
curl -SL http://www.gecode.org/download/gecode-5.1.0.tar.gz | tar xz \
        && cd gecode-5.1.0 \
        && ./configure --prefix=$HOME/local \
        --disable-minimodel \
        --disable-examples \
    	--disable-flatzinc \
    	--disable-gist \
    	--disable-driver \
    	--disable-qt \
    	--disable-mpfr \
    	--disable-doc-tagfile \
    	--disable-doc-dot \
    	--disable-thread \
    	&& make -j8 && make install
```


Once you have installed Gecode either from source or with precompiled packages, you need to make sure that the Gecode
libraries are placed somewhere in the `LIBRARY_PATH` and `LD_LIBRARY_PATH` environment variables, e.g. by placing something
like the following in your `.bashrc` script:

```
	export LIBRARY_PATH=$LIBRARY_PATH:${HOME}/local/lib
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/local/lib
```

## 3. Download the planner and related dependencies

Next, you should download the planner from the main repository and pull related dependencies.
The planner does currently rely on the [LAPKT Planning Toolkit](http://lapkt.org/) and
on the [Rapidjson](https://github.com/Tencent/rapidjson) library, which will get installed
as Git submodules.

```
git clone --branch=v2 https://github.com/aig-upf/fs.git fs-planner
cd fs-planner
git submodule update --init
```

## 4. Build the planner
Once all this is set up, you can invoke the `build.py` build script, which is a thin wrapper
around the `SCons` build system. Invoke `./build.py -h` to obtain a list of possible
releases you can build: most likely, you will want to build the _production_ release, 
which is optimized for performance, or (let's hope not!!) the _debug_ release, which is geared towards
ease of debugging. If you want to build the _production_ release, for instance, 
you just need to issue the following command:

```shell
./build.py -p
```
