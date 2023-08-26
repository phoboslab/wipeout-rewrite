# wipeout wasm in docker
FROM debian:bullseye

LABEL Greg Lebreton <greg.lebreton@hotmail.com>

ENV ROOT_WWW_PATH /var/www/html

RUN apt-get update && apt-get install -y \
    git \
    make \
    libx11-dev \
    libxcursor-dev \
    libxi-dev \
    libasound2-dev \
	ca-certificates \
	unzip \
	sed \
	p7zip-full \
    emscripten \
	coffeescript \
	xz-utils \
	nginx \
	wget \
	--no-install-recommends \
	&& rm -rf /var/lib/apt/lists/*

# sinon le build failed
RUN emcc --version

# clone repo git
RUN cd / \
    && git clone https://github.com/phoboslab/wipeout-rewrite.git \
    && cd wipeout-rewrite

# copie des fichiers necessaires (wipeout-datas)
# COPY ./wipeout /wipeout-rewrite

# wipeout datas
RUN cd /tmp \
    && wget https://phoboslab.org/files/wipeout-data-v01.zip \
    && unzip wipeout-data-v01.zip -d wipeout \
    && cp -r wipeout/* /wipeout-rewrite

# build
RUN cd wipeout-rewrite \
    && make wasm

# mise en place des fichiers
RUN cp /wipeout-rewrite/build/wasm/* /var/www/html \
    && cp /wipeout-rewrite/src/wasm-index.html /var/www/html/index.html \
    && cp -r /wipeout-rewrite/wipeout /var/www/html


WORKDIR ${ROOT_WWW_PATH}

EXPOSE 80

COPY entrypoint.sh /

CMD [ "sh", "/entrypoint.sh"]
