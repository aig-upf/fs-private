ANTLR 4 Parser for Functional Strips
====================================

Installing the thing
--------------------

Instructions for installing ANTLRv4 retrieved from https://theantlrguy.atlassian.net/wiki/display/ANTLR4/Getting+Started+with+ANTLR+v4

* Install Java (version 1.6 or higher)
* Download

$ cd /usr/local/lib
$ curl -O http://www.antlr.org/download/antlr-4.5-complete.jar

Or just download in browser from website:
    http://www.antlr.org/download.html
and put it somewhere rational like /usr/local/lib.

* Add antlr-4.5-complete.jar to your CLASSPATH:

$ export CLASSPATH=".:/usr/local/lib/antlr-4.5-complete.jar:$CLASSPATH"

It's also a good idea to put this in your .bash_profile or whatever your startup script is.

* Create aliases for the ANTLR Tool, and TestRig.

$ alias antlr4='java -Xmx500M -cp "/usr/local/lib/antlr-4.5-complete.jar:$CLASSPATH" org.antlr.v4.Tool'
$ alias grun='java org.antlr.v4.runtime.misc.TestRig'

Python Support
--------------
Install with:

pip install antlr4-python2-runtime

pip install antlr4-python3-runtime

Or, you can download and untar the appropriate package from:

https://pypi.python.org/pypi/antlr4-python2-runtime

https://pypi.python.org/pypi/antlr4-python3-runtime

The runtimes are provided in the form of source code, so no additional installation is required.

See [Python runtime targets](https://theantlrguy.atlassian.net/wiki/display/ANTLR4/Python+Target) for more information.

TODO List
---------

- Upgrade the grammar from ANTLR v3 to v4 (see [here](https://github.com/antlr/antlr4/issues/464) for directions
on how to proceed).
