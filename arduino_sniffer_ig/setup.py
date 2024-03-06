from setuptools import setup, find_packages

setup(name='ArduinoBLESniffer',
      version='0.0.2',
      sdk_version='1.3.5',
      author='Avi-on',
      author_email='fp@avi-on.com',
      description='Arduino BLE Sniffer',
      license='PRIVATE',
      packages=find_packages('src'),
      package_dir={'': 'src'},
      zip_safe=False,
      install_requires=[],
      entry_points="""
      [console_scripts]
      ArduinoBLESniffer = Application:main
      """)
