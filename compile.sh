#!/usr/bin/env bash

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j
