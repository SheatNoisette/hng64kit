FROM --platform=linux/amd64 ubuntu:26.04

ARG WATCOM_URL=https://github.com/open-watcom/open-watcom-1.9/releases/download/ow1.9/open-watcom-c-linux-1.9

ENV WATCOM=/opt/watcom
ENV INCLUDE=${WATCOM}/h
ENV PATH=${WATCOM}/binl:${PATH}

RUN apt-get update
RUN apt-get install -y --no-install-recommends ca-certificates curl unzip make
RUN rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN curl -fsSLo watcom.bin ${WATCOM_URL}

RUN unzip -q watcom.bin "binl/*" "h/*" -d ${WATCOM} || [ $? -eq 1 ]
RUN chmod +x ${WATCOM}/binl/*

WORKDIR /
RUN rm -rf /build

ENTRYPOINT ["/bin/bash"]
