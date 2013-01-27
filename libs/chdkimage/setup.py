from distutils.core import setup, Extension

setup(
    name = 'chdkimage',
    version = '1.0',
    description = 'CHDK Live View image post-processing',
    ext_modules = [
        Extension(
            'chdkimage',
            sources = ['chdkimage.c'],
        ),
    ],
)
