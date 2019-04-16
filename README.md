# Metahash Fullnode Client
This repository contains Metahash Fullnode Client's source code written in C++.

There are 2 ways to build MetaHash client you may choose from:
1) build docker container from Dockerfile, see [Create-docker-file](https://github.com/metahashorg/metahash-fullnode-client/wiki/Create-docker-file),
2) build from source code, see [Building](https://github.com/metahashorg/metahash-fullnode-client#building) section below.

For more details about Metahash fullnode client capabilities, installation and usage see [wiki](https://github.com/metahashorg/metahash-fullnode-client/wiki).

### Building:
```shell
sudo apt install rapidjson-dev libgmp-dev libcurl4-gnutls-dev

cd /tmp/
git clone https://github.com/metahashorg/metahash-fullnode-client
cd metahash_client/
mkdir build
cd build
cmake ..
make
```
Note: if you receive error like "Command 'cmake' not found." you need to install cmake:
```shell
sudo apt install cmake
```

### Running
```
run --help what would see allowed options
run --request what would will see description for requests
```

### Using:

#### Generate wallet 
```
{"id":decimal, "version":"2.0","method":"generate", "params":{"password":"str"}}
```
You don't need to enter the password in this version. 

#### Wallet balance
```
{"id":decimal, "version":"2.0","method":"fetch-balance", "params":{"address":"hexstr"}}
```

#### Wallet history
```
{"id":decimal, "version":"2.0","method":"fetch-history", "params":{"address":"hexstr"}}
```

#### Create transaction 
```
{"id":decimal, "version":"2.0","method":"create-tx", "params":{"address":"hexstr", "password":"str", "to":"hexstr", "value":"decimal/all", "fee":"decimal", "nonce":"decimal", "data": "str"}}
```
You don't need to enter the password in this version. 

#### Send transaction 
```
{"id":decimal, "version":"2.0","method":"send-tx", "params":{"address":"hexstr", "password":"str", "to":"hexstr", "value":"decimal/all", "fee":"decimal", "nonce":"decimal", "data": "str"}}
```
You may not provide nonce, in this case it will be calculated automatically.
You don't need to enter the password in this version. 

#### Geting blocks count
```
{"id":1, "version":"2.0", "method":"get-count-blocks"}
```

#### Geting last transactions
```
{"id":1, "version":"2.0", "method":"get-last-txs"}
```

#### Geting block by block hash
```
{"id":1, "version":"2.0", "method":"get-block-by-hash", "params":{"hash":"str"}}
```
If the block is state-block then probably it will rejected

#### Geting block by block number
```
{"id":1, "version":"2.0", "method":"get-block-by-number", "params":{"number":decimal}}
```
If the block is state-block then probably it will rejected

#### Geting blocks
```
{"id":1, "version":"2.0", "method":"get-blocks", "params":{"countBlocks":decimal, "beginBlock":decimal}}
```

#### Geting block dump by block number
```
{"id":1, "version":"2.0", "method":"get-dump-block-by-number", "params":{"number":decimal}}
```

#### Geting block dump by block hash
```
{"id":1, "version":"2.0", "method":"get-dump-block-by-hash", "params":{"hash":"str"}}
```

#### Geting transaction
```
{"id":1, "version":"2.0", "method":"get-tx", "params":{"hash":"str"}}
```

#### Geting status
```
{"id":1, "version":"2.0", "method":"status", "params":{"cmd":"str"}}
```
Params are optional.
Also you can get it by GET request.
