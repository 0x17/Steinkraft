# Extends Ubuntu oldest still supported LTS
FROM ubuntu:22.04
RUN apt-get update && apt-get install -yq apt-transport-https apt-utils
RUN apt install -y cmake ninja-build build-essential libsdl1.2-dev libsdl-mixer1.2-dev libglew-dev