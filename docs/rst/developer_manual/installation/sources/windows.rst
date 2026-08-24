.. include:: ../../../exports/alias.include

.. _developer_manual_installation_sources_windows:
.. _windows_sources_requirements:
.. _windows_sources_dependencies:

##########################
Developer build on Windows
##########################

Follow :ref:`installation_manual_windows` in a Developer PowerShell. Install
the documentation requirements and enable tests/docs in the colcon build:

.. code-block:: powershell

   PS> Set-Location "$HOME\DDS-Record-Replay"
   PS> py -m venv .venv-docs
   PS> .\.venv-docs\Scripts\Activate.ps1
   PS> py -m pip install -r src\ddsrecordreplay\docs\requirements.txt
   PS> colcon build --merge-install --cmake-args `
         -DCMAKE_BUILD_TYPE=Debug `
         -DBUILD_TESTS=ON `
         -DBUILD_DOCS=ON
   PS> colcon test
   PS> colcon test-result --verbose

Documentation can also be built directly:

.. code-block:: powershell

   PS> sphinx-build -nW --keep-going -b html docs docs\_build\html
   PS> sphinx-build -W --keep-going -b spelling docs docs\_build\spelling
   PS> doc8 --ignore D001 docs

.. _windows_sources_colcon_installation:
.. _windows_sources_cmake_installation:
.. _windows_sources_local_installation:
.. _windows_sources_global_installation:
.. _windows_sources_cmake_pip3_wget_git:
.. _windows_sources_visual_studio:
.. _windows_sources_chocolatey:
.. _windows_sources_colcon_install:
.. _windows_sources_fastddspython:
.. _windows_sources_gtest:
.. _windows_sources_asiotinyxml2:
.. _windows_sources_openssl:
.. _windows_sources_yamlcpp:
.. _windows_sources_swig:
.. _windows_sources_PyQt6:
.. _windows_sources_mcap:
.. _windows_sources_eprosima_dependencies:

colcon is the supported path for the complete workspace because it activates
the Visual Studio environment and builds packages in dependency order. See
:ref:`cmake_options` for build switches.

.. _colcon: https://colcon.readthedocs.io/en/released/
.. _CMake: https://cmake.org
.. _pip3: https://docs.python.org/3/installing/index.html
.. _wget: https://www.gnu.org/software/wget/
.. _git: https://git-scm.com/
.. _vcstool: https://pypi.org/project/vcstool/
.. _Gtest: https://github.com/google/googletest
