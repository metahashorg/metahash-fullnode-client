#!/bin/bash

workdir=/opt/mhp
docker_image=$1
net=$2

if [ -z $docker_image ]
then
	echo "usage: $0 docker_image_name network(default is dev)"
	exit 2
fi


if [ -z "$net" ]
then
	net="dev"
fi

echo $net

if [ ! -d $workdir ]
then
	mkdir -p $workdir
fi

cd $workdir

if [ ! -f settings.json.$net ]
then
	wget -q https://raw.githubusercontent.com/metahashorg/crypt_example_c/master/pre_compiled/settings.json.$net

	if [ $? -ne 0 ]
	then
		echo failed to download config file for net $net
		exit 2
	fi
fi


docker run -it -p 9999:9999 -d -v $workdir/wallet.$net:/opt/mhservice/wallet -v $workdir/settings.json.$net:/opt/mhservice/settings.json -v $workdir/leveldb.$net:/opt/mhservice/leveldb.$net -v $workdir/blocks.$net:/opt/mhservice/blocks $docker_image