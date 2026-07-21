#!/usr/bin/env bash
set -e

lcov --directory . --capture --output-file coverage.info \
    --ignore-errors inconsistent,empty

lcov --remove coverage.info \
    '/usr/*' \
    "${HOME}/.cache/*" \
    '*/vcpkg_installed/*' \
    '*/_deps/*' \
    '*/tests/*' \
    --ignore-errors unused,inconsistent,empty \
    --output-file coverage.info

lcov --list coverage.info
