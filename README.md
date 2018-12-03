# metahash-crypt-client

### Building:
```
apt install rapidjson-dev libgmp-dev libcurl4-gnutls-dev

cd /tmp/
git clone https://github.com/metahashorg/crypt_example_c
cd crypt_example_c/
mkdir build
cd build
cmake ..
make
```

### Using:
```
run --help what would see allowed options
run --request what would will see description for requests
```

### Requests:

#### Generate wallet 
```
{"id":decimal, "version":"2.0","method":"generate", "params":{"password":"str"}}
```
You don't need to enter the password in this version. 

#### Balance of wallet 
```
{"id":decimal, "version":"2.0","method":"fetch-balance", "params":{"address":"hexstr"}}
```

#### History of wallet 
```
{"id":decimal, "version":"2.0","method":"fetch-history", "params":{"address":"hexstr"}}
```

#### Create transaction 
```
{"id":decimal, "version":"2.0","method":"create-tx", "params":{"address":"hexstr", "password":"str", "to":"hexstr", "value":"decimal/all", "fee":"decimal", "nonce":"decimal", "data": "dataHex"}}
```

#### Send transaction 
```
{"id":decimal, "version":"2.0","method":"send-tx", "params":{"address":"hexstr", "password":"str", "to":"hexstr", "value":"decimal/all", "fee":"decimal", "nonce":"decimal", "data": "dataHex"}}
```
You may not provide nonce, in this case it will be calculated automatically.

