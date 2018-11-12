# metahash-crypt-client

### Dependices:
```
boost 1.67
rapidjson
```

### Using:
```
run --help what would see allowed options
run --request what would will see description for requests
```

### Requests:
```
Generate wallet 
{"id":decimal, "version":"2.0","method":"generate", "params":{"password":str}}
В данной версии пароль не указывается

Balance of wallet 
{"id":decimal, "version":"2.0","method":"fetch-balance", "params":{"address":hexstr}}

History of wallet 
{"id":decimal, "version":"2.0","method":"fetch-history", "params":{"address":hexstr}}

Create transaction 
{"id":decimal, "version":"2.0","method":"create-tx", "params":{"address":hexstr, "password":str, "to":hexstr, "value":decimal/all, "fee":decimal/auto, "nonce":decimal}}

Send transaction 
{"id":decimal, "version":"2.0","method":"send-tx", "params":{"address":hexstr, "password":str, "to":hexstr, "value":decimal/all, "fee":decimal/auto, "nonce":decimal}}
nonce можно не узазывать, тогда произойдет автовычисление
```

