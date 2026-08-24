# DDS Record & Replay documentation

The user documentation is built with [Sphinx](https://www.sphinx-doc.org/) and
published on [Read the Docs](https://dds-recorder.readthedocs.io/).

## Local build

From the DDS Record & Replay repository root, create an isolated environment
and install the pinned dependencies:

```bash
python3 -m venv .venv-docs
. .venv-docs/bin/activate
python -m pip install -r docs/requirements.txt
```

Build HTML with missing references and warnings treated as errors:

```bash
sphinx-build -nW --keep-going -b html docs docs/_build/html
```

Run the documentation checks:

```bash
sphinx-build -W --keep-going -b spelling docs docs/_build/spelling
doc8 --ignore D001 docs
python docs/test/validate_examples.py
```

The external link audit requires network access and is kept separate from the
deterministic checks:

```bash
sphinx-build -W --keep-going -b linkcheck docs docs/_build/linkcheck
```

## Colcon build

In a complete imported workspace:

```bash
colcon build --packages-select ddsrecorder_docs --cmake-args -DBUILD_DOCS=ON -DBUILD_TESTS=ON
colcon test --packages-select ddsrecorder_docs --event-handler console_direct+
colcon test-result --verbose
```

## Authoring rules

* Ground behavior and defaults in the CLI parsers, YAML readers, schemas, and
  runtime configuration constructors.
* Put reusable YAML under `docs/rst/examples/` and include it with
  `literalinclude`; the validation test checks every file in that directory.
* Use SVG for architecture/state diagrams and real captures only for UI steps.
* Preserve existing page paths and explicit Sphinx labels when moving content.
* Do not add build-time downloads to `conf.py`; docs builds must work offline.
