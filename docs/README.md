# eProsima DDS Recorder docs

This package generates the DDS Recorder documentation.
[Here](https://eprosima-dds-recorder.readthedocs.io/en/latest/) it can be seen the online documentation hosted in
[readthedocs](https://readthedocs.org/).
This packages is powered by [sphinx](https://www.sphinx-doc.org/en/master/).

---

## Documentation generation

### Dependencies

Before being able to build the documentation, some dependencies need to be installed:

```bash
sudo apt update
sudo apt install -y \
    doxygen \
    python3 \
    python3-pip \
    python3-venv \
    python3-sphinxcontrib.spelling \
    imagemagick
pip3 install -U -r src/ddsrecorder/docs/requirements.txt
```

### Build documentation

#### Virtual environment

This tutorial will create a python3 virtual environment to avoid polluting user's python installation.

```bash
# Create a python3 virtual environment
python3 -m venv recorder_venv
# Activate the environment
source recorder_venv/bin/activate
# Install dependencies within the environment
pip3 install -r docs/requirements.txt
```

#### Generate HTML docs

```bash
# Source the python virtual environment
source recorder_venv/bin/activate
# Change directories to the repository directory
cd docs
# Make sure that there are no build directories
make clean
# Generate HTML documentation
make html
# Open the documentation
xdg-open build/html/index.html
```

---

## Library documentation

This documentation is focused on the user manual for installing and working with DDS Recorder.
To learn about the repository structure, design decisions, development guidelines, etc.,
each package is documented separately and the source code is commented using Doxygen format.
In directory `.dev` there is a generic `README.md` with the main information needed by a developer.
