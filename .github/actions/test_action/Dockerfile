FROM ubuntu:latest

COPY entrypoint.sh /entrypoint.sh

RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y g++-aarch64-linux-gnu

ENTRYPOINT ["/entrypoint.sh"]
