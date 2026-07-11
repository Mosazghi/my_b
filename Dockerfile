FROM mcr.microsoft.com/devcontainers/cpp:ubuntu

ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=host.docker.internal:0.0

RUN apt-get update && apt-get install -y --no-install-recommends \
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
    clangd \
    libclang-dev \
    && rm -rf /var/lib/apt/lists/*


RUN useradd -m -s /bin/bash mosa \
    && echo "mosa ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/mosa \
    && chmod 0440 /etc/sudoers.d/mosa

USER mosa

CMD ["/bin/bash"]
