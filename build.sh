#!/bin/sh
docker build -t in2601-builder . || exit 1
docker run --rm -v "$(pwd):/src" in2601-builder
