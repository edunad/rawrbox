FROM ubuntu:22.04
# Same version as our runner in rawrbox

LABEL org.opencontainers.image.source=https://github.com/edunad/rawrbox
LABEL org.opencontainers.image.description="RAWRBOX-DEV-LINUX SETUP"

ENV DEBIAN_FRONTEND=noninteractive

# Setup all dependencies
RUN dpkg --add-architecture i386 && \
apt update && \
apt full-upgrade -y && \
apt install -y build-essential \
    wget \
    cmake \
    libegl1-mesa-dev \
    libwayland-dev \
    libxkbcommon-dev \
    wayland-protocols \
    libvulkan-dev \
    libssl-dev \
    libvpx-dev \
    gcc-11 \
    libgl1-mesa-glx \
    libgl1-mesa-dev \
    libglew-dev \
    ninja-build \
    python3-pip \
    libx11-dev \
    libx11-xcb-dev \
    libfontenc-dev \
    libice-dev \
    libsm-dev \
    libxau-dev \
    libxaw7-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxdamage-dev \
    libxdmcp-dev \
    libxext-dev \
    libxfixes-dev \
    libxft-dev \
    libxi-dev \
    libxinerama-dev \
    libxkbfile-dev \
    libxmu-dev \
    libxmuu-dev \
    libxpm-dev \
    libxrandr-dev \
    libxrender-dev \
    libxres-dev \
    libxss-dev \
    libxt-dev \
    libxtst-dev \
    libxv-dev \
    libxvmc-dev \
    libxxf86vm-dev \
    xtrans-dev \
    libxcb-render0-dev \
    libxcb-render-util0-dev \
    libxcb-xkb-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-shape0-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-xinerama0-dev \
    xkb-data \
    libxcb-dri3-dev \
    uuid-dev \
    libxcb-util-dev \
	git

WORKDIR /root

# Clone
RUN git config --global user.email "github.bot@users.noreply.github.com" && \
git config --global user.name "github.bot" && \
git config --global credential.helper store

RUN git clone "https://github.com/edunad/rawrbox"

WORKDIR /root/rawrbox
RUN echo "alias rawrbox=\"cd /root/rawrbox\"" >> /root/.bashrc
RUN echo "alias cmake_rawrbox=\"rawrbox;mkdir -p build;cd build;CXX=g++ cmake .. -G Ninja -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=\"ON\" -DRAWRBOX_DEV_MODE:BOOL=\"ON\";cd ../\"" >> /root/.bashrc
RUN echo "alias build_rawrbox=\"cd build && ninja\"" >> /root/.bashrc

CMD /bin/bash
ENTRYPOINT /bin/bash

RUN rawrbox -m myuser
USER rawrbox
# Done 🐲
