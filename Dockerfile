FROM ubuntu:latest
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get -y install gcc cmake git
WORKDIR /phev
RUN git clone https://github.com/ThrowTheSwitch/Unity.git
RUN git clone https://github.com/papawattu/msg-core
RUN git clone https://github.com/DaveGamble/cJSON
WORKDIR /phev/Unity
RUN mkdir -p build && cd build && cmake .. && make && make install
WORKDIR /phev/msg-core
RUN mkdir -p build && cd build && cmake .. && make && make install
WORKDIR /phev/cJSON
RUN mkdir build && cd build && cmake .. && make && make install
WORKDIR /phev/phevcore
COPY . .
RUN mkdir build && cd build && cmake -DBUILD_TESTS=true .. && make
WORKDIR /phev/phevcore/build
CMD ["ctest","-j6","-T","test","--output-on-failure"]
