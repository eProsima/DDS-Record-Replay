#!/usr/bin/env python3
"""Validate every documented YAML example against schema and runtime rules."""

from __future__ import annotations

import ipaddress
import json
import re
import sys
from datetime import datetime, timedelta
from pathlib import Path
from typing import Any

import yaml
from jsonschema import Draft7Validator, FormatChecker


DOCS_DIR = Path(__file__).resolve().parents[1]
REPOSITORY_DIR = DOCS_DIR.parent
EXAMPLES_DIR = DOCS_DIR / 'rst' / 'examples'
SHIPPED_CONFIG_DIR = REPOSITORY_DIR / 'resources' / 'configurations'
SCHEMAS = {
    'recorder': REPOSITORY_DIR / 'resources' / 'configurations' /
        'ddsrecorder_config_schema.json',
    'replayer': REPOSITORY_DIR / 'resources' / 'configurations' /
        'ddsreplayer_config_schema.json',
}
CONFIGURATION_REFERENCES = (
    DOCS_DIR / 'rst' / 'configuration' / 'common.rst',
    DOCS_DIR / 'rst' / 'recording' / 'usage' / 'configuration.rst',
    DOCS_DIR / 'rst' / 'replaying' / 'usage' / 'configuration.rst',
)
UNITS = {
    'B': 1,
    'KB': 1000,
    'MB': 1000**2,
    'GB': 1000**3,
    'TB': 1000**4,
    'PB': 1000**5,
    'KIB': 1024,
    'MIB': 1024**2,
    'GIB': 1024**3,
    'TIB': 1024**4,
    'PIB': 1024**5,
}
MIN_SAFETY_MARGIN = 10 * 1024 * 1024

format_checker = FormatChecker()


@format_checker.checks('v4')
def is_ipv4(value: object) -> bool:
    try:
        return isinstance(value, str) and ipaddress.ip_address(value).version == 4
    except ValueError:
        return False


def byte_quantity(value: str) -> int:
    match = re.fullmatch(r'(\d+(?:\.\d+)?)\s*([A-Za-z]+)', value)
    if not match or match.group(2).upper() not in UNITS:
        raise ValueError(f'invalid byte quantity {value!r}')
    return int(float(match.group(1)) * UNITS[match.group(2).upper()])


def limits_from(block: dict[str, Any], *, sql: bool) -> tuple[int, int, bool]:
    max_size = byte_quantity(block['max-size']) if 'max-size' in block else 0
    max_file_size = max_size
    if 'max-file-size' in block:
        max_file_size = byte_quantity(block['max-file-size'])

    if sql and bool(max_size) != bool(max_file_size):
        max_size = max_size or max_file_size
        max_file_size = max_file_size or max_size

    if max_size and not max_file_size:
        raise ValueError('max-file-size cannot be unlimited when max-size is limited')
    if max_size and max_size < max_file_size:
        raise ValueError('max-size cannot be smaller than max-file-size')
    if sql and max_size != max_file_size:
        raise ValueError('SQL max-size and max-file-size must be equal')
    return max_size, max_file_size, block.get('log-rotation', False)


def validate_recorder_runtime(data: dict[str, Any], text: str) -> None:
    recorder = data.get('recorder') or {}
    sql_block = recorder.get('sql')
    mcap_block = recorder.get('mcap')

    sql_enabled = bool(sql_block and sql_block['enable'])
    mcap_enabled = not sql_enabled
    if mcap_block is not None:
        mcap_enabled = mcap_block['enable']
    if not mcap_enabled and not sql_enabled:
        raise ValueError('at least one of MCAP or SQL must be enabled')

    output = recorder.get('output') or {}
    safety_margin = byte_quantity(output.get('safety-margin', '10MiB'))
    has_extra_margin = safety_margin > MIN_SAFETY_MARGIN

    for name, block, enabled in (
            ('MCAP', mcap_block, mcap_enabled),
            ('SQL', sql_block, sql_enabled)):
        if not enabled or not block or 'resource-limits' not in block:
            continue
        max_size, max_file_size, rotation = limits_from(
            block['resource-limits'], sql=name == 'SQL')
        if rotation and not max_file_size:
            raise ValueError(f'{name} rotation requires a finite file limit')
        if rotation and not max_size and not has_extra_margin:
            raise ValueError(
                f'{name} rotation needs max-size or safety-margin greater than 10MiB')

    if '# use: replay' in text and sql_enabled:
        if sql_block.get('data-format', 'both') not in {'cdr', 'both'}:
            raise ValueError('a replay example must store SQL CDR data')


def parse_timestamp(value: dict[str, Any]) -> datetime:
    parsed = datetime.strptime(
        value['datetime'], value.get('format', '%Y-%m-%d_%H-%M-%S'))
    return parsed + timedelta(
        milliseconds=value.get('milliseconds', 0),
        microseconds=value.get('microseconds', 0) +
        value.get('nanoseconds', 0) / 1000,
    )


def validate_replayer_runtime(data: dict[str, Any], _text: str) -> None:
    replayer = data.get('replayer') or {}
    if 'begin-time' in replayer and 'end-time' in replayer:
        if parse_timestamp(replayer['begin-time']) >= parse_timestamp(replayer['end-time']):
            raise ValueError('begin-time must be earlier than end-time')


def schema_kind(text: str, path: Path) -> str:
    match = re.search(r'^#\s*schema:\s*(recorder|replayer)\s*$', text, re.MULTILINE)
    if not match:
        raise ValueError(f'{path}: missing "# schema: recorder|replayer" marker')
    return match.group(1)


def validate_example(path: Path, forced_kind: str | None = None) -> list[str]:
    text = path.read_text(encoding='utf-8')
    kind = forced_kind or schema_kind(text, path)
    data = yaml.safe_load(text)
    if not isinstance(data, dict):
        return [f'{path}: example must contain a YAML mapping']

    with SCHEMAS[kind].open(encoding='utf-8') as schema_handle:
        schema = json.load(schema_handle)
    validator = Draft7Validator(schema, format_checker=format_checker)
    errors = []
    for error in sorted(validator.iter_errors(data), key=lambda item: list(item.path)):
        location = '.'.join(str(part) for part in error.absolute_path) or '<root>'
        errors.append(f'{path}:{location}: {error.message}')

    if not errors:
        try:
            if kind == 'recorder':
                validate_recorder_runtime(data, text)
            else:
                validate_replayer_runtime(data, text)
        except (KeyError, TypeError, ValueError) as error:
            errors.append(f'{path}: {error}')
    return errors


def validate_cross_field_guards() -> list[str]:
    """Prove that non-schema runtime constraints reject unsafe examples."""
    errors = []
    recorder_cases = (
        (
            {'recorder': {'mcap': {'enable': False}, 'sql': {'enable': False}}},
            '',
            'at least one',
        ),
        (
            {
                'recorder': {
                    'mcap': {'enable': False},
                    'sql': {'enable': True, 'data-format': 'json'},
                },
            },
            '# use: replay',
            'must store SQL CDR',
        ),
        (
            {
                'recorder': {
                    'mcap': {
                        'enable': True,
                        'resource-limits': {'log-rotation': True},
                    },
                },
            },
            '',
            'requires a finite file limit',
        ),
    )
    for data, marker, expected in recorder_cases:
        try:
            validate_recorder_runtime(data, marker)
        except ValueError as error:
            if expected not in str(error):
                errors.append(
                    f'cross-field guard expected {expected!r}, got {str(error)!r}')
        else:
            errors.append(f'cross-field guard did not reject {expected!r}')

    invalid_range = {
        'replayer': {
            'begin-time': {'datetime': '2026-08-24_14-30-00'},
            'end-time': {'datetime': '2026-08-24_14-30-00'},
        },
    }
    try:
        validate_replayer_runtime(invalid_range, '')
    except ValueError:
        pass
    else:
        errors.append('cross-field guard did not reject an empty replay range')
    return errors


def validate_public_schema_keys_documented() -> tuple[list[str], int]:
    """Require every public JSON-schema property to appear in the reference."""
    public_keys = set()
    for schema_path in SCHEMAS.values():
        with schema_path.open(encoding='utf-8') as schema_handle:
            schema = json.load(schema_handle)
        for definition in schema.get('definitions', {}).values():
            public_keys.update((definition.get('properties') or {}).keys())

    reference_text = '\n'.join(
        path.read_text(encoding='utf-8') for path in CONFIGURATION_REFERENCES)
    literals = re.findall(r'``([^`]+)``', reference_text)
    literal_parts = {
        part
        for literal in literals
        for part in re.split(r'[^A-Za-z0-9_-]+', literal)
        if part
    }
    missing = sorted(public_keys - literal_parts)
    return (
        [f'public schema key is not documented: {key}' for key in missing],
        len(public_keys),
    )


def validate_empty_configurations() -> list[str]:
    """Keep the documented null/empty built-in configuration behavior valid."""
    errors = []
    for kind, schema_path in SCHEMAS.items():
        with schema_path.open(encoding='utf-8') as schema_handle:
            validator = Draft7Validator(json.load(schema_handle))
        for empty_value in (None, {}):
            for error in validator.iter_errors(empty_value):
                errors.append(
                    f'{kind} schema rejected empty configuration: {error.message}')
    return errors


def main() -> int:
    documentation_paths = sorted(EXAMPLES_DIR.glob('*.yaml'))
    shipped_paths = [
        (path, kind)
        for kind in ('recorder', 'replayer')
        for path in sorted((SHIPPED_CONFIG_DIR / kind).glob('*.yaml'))
    ]
    if not documentation_paths:
        print(f'No YAML examples found under {EXAMPLES_DIR}', file=sys.stderr)
        return 1

    errors = [
        error
        for path in documentation_paths
        for error in validate_example(path)
    ]
    errors.extend(
        error
        for path, kind in shipped_paths
        for error in validate_example(path, kind)
    )
    errors.extend(validate_cross_field_guards())
    errors.extend(validate_empty_configurations())
    documentation_errors, public_key_count = validate_public_schema_keys_documented()
    errors.extend(documentation_errors)
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        return 1

    print(
        f'Validated {len(documentation_paths)} documentation examples and '
        f'{len(shipped_paths)} shipped configurations; documented '
        f'{public_key_count} public schema keys.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
