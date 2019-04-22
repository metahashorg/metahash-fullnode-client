FROM ubuntu:16.04
MAINTAINER metahash
EXPOSE 9999 9999
WORKDIR /opt/mhservice
RUN apt update
RUN apt install -y software-properties-common wget
RUN wget https://raw.githubusercontent.com/metahashorg/metahash-fullnode-client/master/pre_compiled/startup.sh
RUN wget -P /usr/lib/ https://raw.githubusercontent.com/metahashorg/metahash-fullnode-client/master/pre_compiled/libcrypto.so.1.1
RUN wget -P /usr/lib/ https://raw.githubusercontent.com/metahashorg/metahash-fullnode-client/master/pre_compiled/libssl.so.1.1
RUN wget https://raw.githubusercontent.com/metahashorg/metahash-fullnode-client/master/pre_compiled/metahash-fullnode-client
RUN chmod a+x startup.sh /usr/lib/libcrypto.so.1.1 /usr/lib/libssl.so.1.1 metahash-fullnode-client
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt update
RUN apt -y install gcc-8 g++-8
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8
ENTRYPOINT "/opt/mhservice/startup.sh"
