FROM ubuntu:latest
RUN apt-get update && apt-get -y install gcc cmake git
WORKDIR /phev/
COPY . .
RUN git submodule update --recursive --init
RUN mkdir build && cd build && cmake .. && make && ctest -j6 -T test --output-on-failure
