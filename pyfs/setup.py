# coding=utf-8
from setuptools import setup, find_packages
from setuptools.command.build_ext import build_ext
from distutils.core import setup, Extension
from codecs import open
from os import path
import sys

here = path.abspath(path.dirname(__file__))
fsdir = path.abspath(path.join(here, '..', 'src'))
vendordir = path.abspath(path.join(here, '..', 'vendor'))
fs_builddir = path.abspath(path.join(here, '..', '.build'))


class BuildExt(build_ext):
    def build_extensions(self):
        # Avoid meaningless gcc warning - see https://stackoverflow.com/a/49041815
        self.compiler.compiler_so = [x for x in self.compiler.compiler_so if x != '-Wstrict-prototypes']
        super(BuildExt, self).build_extensions()


# Get the long description from the README file
with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()


def main():
    debug_mode = '--debug' in sys.argv  # Haven't found yet a more robust alternative to this

    fs_libname = 'fs-debug' if debug_mode else 'fs'

    module = Extension(
        'pyfs.extension._pyfs',

        language="c++",

        sources=[
            'src/extension/factories.cxx',
            'src/extension/fstrips.cxx',
            'src/extension/language_info.cxx',
            'src/extension/module.cxx',
            'src/extension/search.cxx',
         ],

        include_dirs=[
            fsdir,
            path.join(vendordir, 'rapidjson', 'include'),  # TODO This should be removed in the near future
            path.join(vendordir, 'lapkt-base', 'src'),
            path.join(vendordir, 'lapkt-novelty', 'src'),
        ],

        library_dirs=[fs_builddir],

        runtime_library_dirs=[fs_builddir],

        extra_objects=[],  # Static library could go in here, as per https://stackoverflow.com/a/49139257

        extra_compile_args=['-std=c++17'],

        libraries=['boost_python35',
                   'boost_program_options', 'boost_serialization', 'boost_system', 'boost_timer', 'boost_chrono',
                   'boost_filesystem', 'rt', 'm',
                   fs_libname
                   ]
    )

    setup(
        name='pyfs',
        version='0.1.0',
        description='pyfs: a Functional STRIPS classical planner',
        long_description=long_description,
        url='https://github.com/aig-upf/fs',
        author='Miquel Ramírez and Guillem Francès',
        author_email='-',

        keywords='planning logic STRIPS',
        classifiers=[
            'Development Status :: 3 - Alpha',

            'Intended Audience :: Science/Research',
            'Topic :: Scientific/Engineering :: Artificial Intelligence',

            'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',

            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.4',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: 3.6',
        ],

        # You can just specify package directories manually here if your project is
        # simple. Or you can use find_packages().
        packages=find_packages('src'),  # include all packages under src
        package_dir={'': 'src'},  # tell distutils packages are under src


        # This field lists other packages that your project depends on to run.
        # Any package you put here will be installed by pip when your project is
        # installed, so they must be valid existing projects.
        #
        # For an analysis of "install_requires" vs pip's requirements files see:
        # https://packaging.python.org/en/latest/requirements.html
        install_requires=[
            'setuptools',
            "tarski @ git+ssh://git@github.com/aig-upf/tarski.git@bbea0f3#egg=tarski-dev-0.2.0"
        ],

        # List additional groups of dependencies here (e.g. development
        # dependencies). Users will be able to install these using the "extras"
        # syntax, for example:
        #
        #   $ pip install sampleproject[dev]
        #
        extras_require={
            'dev': ['pytest', 'tox'],
            'test': ['pytest', 'tox'],
        },


        # To provide executable scripts, use entry points in preference to the
        # "scripts" keyword. Entry points provide cross-platform support and allow
        # `pip` to create the appropriate form of executable for the target
        # platform.
        #
        # For example, the following would provide a command called `sample` which
        # executes the function `main` from this package when invoked:
        # entry_points={
        #     'console_scripts': [
        #         'sample=sample:main',
        #     ],
        # },

        # This will include non-code files specified in the manifest, see e.g.
        # http://python-packaging.readthedocs.io/en/latest/non-code-files.html
        # include_package_data=True,

        cmdclass={'build_ext': BuildExt},
        ext_modules=[module],
    )


if __name__ == '__main__':
    main()
