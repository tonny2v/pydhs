FROM alpine:latest

RUN apk upgrade && \
    apk add --no-cache \
        boost-python3 \
        boost-dev \
        cmake \
        g++ \
        git \
        make \
        musl-dev \
        ninja \
        py3-numpy \
        py3-numpy-dev \
        py3-pip \
        python3 \
        python3-dev

RUN pip3 install --upgrade pip uv

WORKDIR /work
COPY . /work

RUN uv sync --all-extras --dev
RUN uv run pytest
RUN uv build
