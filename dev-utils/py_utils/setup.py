"""Setup file to create py_utils library."""
from setuptools import setup

package_name = 'py_utils'

description = 'Developer Python utils'
long_description = description

file_packages = [
    package_name,
    package_name + '/debugging',
    package_name + '/logging',
    package_name + '/time',
    package_name + '/wait',
]

setup(
    name=package_name,
    version='0.6.0',
    packages=file_packages,
    long_description=long_description,
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='eprosima',
    maintainer_email='raul@eprosima.com',
    description=description,
    license='Apache License, Version 2.0',
    tests_require=['pytest'],
    test_suite='test',
    entry_points={},
)
