FROM ubuntu:latest
RUN apt-get update && apt-get -y install gcc cmake
WORKDIR /phev/
COPY . .
RUN mkdir build && cd build && cmake .. && make && ctest -j6 -T test --output-on-failure
