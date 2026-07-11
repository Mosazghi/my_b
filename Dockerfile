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
    libgtest-dev \
    libgmock-dev \
    clangd \
    libclang-dev \
    && rm -rf /var/lib/apt/lists/*


RUN useradd -m -s /bin/bash mosa \
    && echo "mosa ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/mosa \
    && chmod 0440 /etc/sudoers.d/mosa

# install sfml 2.6
    RUN apt-get update && apt-get install -y libfreetype-dev libgl1-mesa-dev libxrandr-dev libxcursor-dev libudev-dev libopenal-dev libflac-dev libvorbis-dev \
    && git clone --depth 1 --branch 2.6.x https://github.com/SFML/SFML.git /tmp/sfml \
    && cmake -S /tmp/sfml -B /tmp/sfml/build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /tmp/sfml/build --target install \
    && rm -rf /tmp/sfml

USER mosa

CMD ["/bin/bash"]
