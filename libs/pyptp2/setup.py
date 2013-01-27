'''
Created on October 26, 2012

@author: zachary.berkowitz
'''
from setuptools import setup, find_packages

setup(
    name='pyptp2',
    version='0.1',
    author='Zachary Berkowitz',
    author_email='zac.berkowitz@gmail.com',
    description='Python module for PTP camera connections over USB',
    url="http://code.google.com/p/pyptp2/",
    #long_description=__doc__,
    packages=['ptp2'],
    license = "GPLv3",
    #include_package_data=True,
    zip_safe=True,
    install_requires=['pyusb'],
    classifiers=['Development Status :: 3 - Alpha',
        'Programming Language :: Python :: 2.7',
        'Topic :: Software Development :: Libraries',
        'Topic :: Utilities']
)