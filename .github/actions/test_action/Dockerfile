FROM ubuntu:18.04

COPY entrypoint.sh /entrypoint.sh

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y build-essential binutils-aarch64-linux-gnu

ENTRYPOINT ["/entrypoint.sh"]
