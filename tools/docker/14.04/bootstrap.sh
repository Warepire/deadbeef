#!/bin/sh

# run this script once to create the base container image,
# which would contain all necessary dependencies and build tools

docker build -f tools/docker/14.04/Dockerfile-builder -t deadbeef-builder-player-clang-14.04 .
