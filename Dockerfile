FROM ubuntu:24.04 

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    libicu-dev \
    libz-dev \
    libssl-dev \
    libsfml-dev \
    libgtest-dev \
    libgmock-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR  /workspace 

CMD ["/bin/bash"]
