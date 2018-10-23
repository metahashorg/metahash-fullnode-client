# metahash_crypt_example_c

### Dependices:
```
boost 1.67
rapidjson
cpp_lib_open_ssl_decor
```

### Requests:
```
Create wallet
{"id":decimal, "version":"1.0.0","method":"generate"}

Balance
{"id":decimal, "version":"1.0.0","method":"fetch-balance", "params": {"address":hexstr}}

History
{"id":decimal, "version":"1.0.0","method":"fetch-history", "params": {"address":hexstr}}

Create transaction
{"id":decimal, "version":"1.0.0","method":"create-tx", "params":{"address":hexstr, "to":hexstr, "value":decimal, "nonce":decimal}}

Send transaction
{"id":decimal, "version":"1.0.0","method":"create-tx", "params":{"address":hexstr, "to":hexstr, "value":decimal, "nonce":decimal}}

Get block by hash
{"id":decimal, "version":"1.0.0","method":"get-block-by-hash", "params":{"hash":hexstr, "type":decimal, "countTxs":decimal, "beginTx":decimal}}

Get block by number
{"id":decimal, "version":"1.0.0","method":"get-block-by-number", "params":{"number":decimal, "type":decimal, "countTxs":decimal, "beginTx": decimal}}

Get blocks
{"id":decimal, "version":"1.0.0","method":"get-blocks", "params":{"countBlocks":decimal, "beginBlock":decimal}}

Get blocks count
{"id":decimal, "version":"1.0.0","method":"get-count-blocks"}

Get block dump by hash
{"id":decimal, "version":"1.0.0","method":"get-dump-block-by-hash", "params":{"hash":hexstr}}

Get block dump by number
{"id":decimal, "version":"1.0.0","method":"get-dump-block-by-hash", "params":{"number":decimal}}

Get last transactions
{"id":decimal, "version":"1.0.0","method":"get-last-txs"}

Get transaction
{"id":decimal, "version":"1.0.0","method":"get-tx", "params":{"hash":hexstr}}
```
