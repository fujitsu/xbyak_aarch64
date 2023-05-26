FROM ubuntu:20.04

COPY entrypoint.sh /entrypoint.sh

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y build-essential binutils-aarch64-linux-gnu crossbuild-essential-arm64 linux-headers-azure pkg-config-aarch64-linux-gnu python3 wget libglib2.0-dev ninja-build
ENTRYPOINT ["/entrypoint.sh"]
