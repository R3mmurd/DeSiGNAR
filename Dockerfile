# Builds and tests Designar in a clean, reproducible Linux environment —
# useful when you don't want to install a C++17 toolchain + CMake locally,
# or want to reproduce a CI failure exactly.
#
# Build the image:
#   docker build -t designar .
#
# Run the test suite:
#   docker run --rm designar
#
# Get a shell with the built library for interactive poking around:
#   docker run --rm -it --entrypoint bash designar

FROM ubuntu:24.04 AS build

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /designar
COPY . .

# Deliberately no -DCMAKE_BUILD_TYPE=Release: that defines NDEBUG, which
# compiles out every assert() — and this test suite uses assert() as its
# check mechanism, so half its local variables would become genuinely
# unused and fail the build under -Werror (see the CMAKE_BUILD_TYPE note
# CMakeLists.txt prints when it is left unset).
RUN mkdir build && cd build && \
    cmake -DDESIGNAR_WARNINGS_AS_ERRORS=ON .. && \
    cmake --build . --parallel

WORKDIR /designar/build

# Default action is to run the test suite; override the entrypoint (see the
# usage note above) to get an interactive shell instead.
CMD ["ctest", "--output-on-failure"]
