FROM ubuntu:latest
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get -y install gcc cmake git
WORKDIR /phev/phevcore
COPY . .
RUN cmake -S . -B build -DBUILD_TESTS=ON
RUN cmake --build build
WORKDIR /phev/phevcore/build
CMD ["ctest", "--output-on-failure"]
