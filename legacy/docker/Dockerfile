FROM ubuntu

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -yqq update \
 && apt-get -yqq install \
      autoconf \
      build-essential \
      curl \
      git \
      libnetcdf-dev \
      vim \
 && apt-get clean

RUN useradd -ms /bin/bash icon
USER icon
WORKDIR /home/icon

