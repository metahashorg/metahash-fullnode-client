#!/bin/bash

errors=0
addr=0.0.0.0:9995
wallet=0x00e5cc4426dffdb85ab2773c4d2537b9f319989eac9ab9ba22


check_response() {
result=$(echo "$1" | grep result)
error=$(echo "$1" | grep error)
success=$(echo "$1" | grep mhc_send)

if [ "$error" != "" ]; then
    ((errors=$errors+1))
    echo "BAD"
    echo $1
elif [[ ( "$result" = "" && "$success" = "" ) ]]; then
    ((errors=$errors+1))
    echo "BAD"
    echo $1
else
    echo "OK"
fi
}


echo -n "test generate..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"generate"}' $addr)

echo -n "test get-count-blocks..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-count-blocks"}' $addr)

echo -n "test get-last-txs..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-last-txs"}' $addr)

echo -n "test create-tx..."
check_response $(curl -s -X POST --data '{"id":1, "version":"1.0.0", "method":"create-tx", "params":{"address":"'$wallet'","to":"0x001b3b70b8710b7fc1b12b3c1668d667dce83b8dac7d987c33","value":"500", "data":"qwertyuiopasdfghjklzxcvbnm1234567890qwertyuiopasdfghjklzxcvbnmasdfghjklasdfghjkwertyuizxcvbnmasdrtyuwiehksdfhskjdfhsjkfhskjhwuieryuwier", "nonce":"12313212313212331"}}' $addr)

echo -n "test send-tx..."
check_response $(curl -s -X POST --data '{"id":1, "version":"1.0.0", "method":"send-tx", "params":{"address":"'$wallet'","to":"0x001b3b70b8710b7fc1b12b3c1668d667dce83b8dac7d987c33","value":"500", "data":"qwertyuiopasdfghjklzxcvbnm1234567890qwertyuiopasdfghjklzxcvbnmasdfghjklasdfghjkwertyuizxcvbnmasdrtyuwiehksdfhskjdfhsjkfhskjhwuieryuwier"}}' $addr)

echo -n "test get-block-by-hash..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-block-by-hash", "params":{"hash":"7e54007044d6ec1fe23d7f7e5f292de5f4129bb0a417ed04d55fbfe94799f7d4"}}' $addr)

echo -n "test get-block-by-number..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-block-by-number", "params":{"number":2323}}' $addr)

echo -n "test get-blocks..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-blocks", "params":{"countBlocks":2, "beginBlock":5}}' $addr)

echo -n "test get-dump-block-by-number..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-dump-block-by-number", "params":{"number":2323}}' $addr)

echo -n "test get-dump-block-by-hash..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-dump-block-by-hash", "params":{"hash":"7e54007044d6ec1fe23d7f7e5f292de5f4129bb0a417ed04d55fbfe94799f7d4"}}' $addr)

echo -n "test get-tx..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"get-tx", "params":{"hash":"d7a8f692147872885117e055ca4d8a9a118aa0c67991ab695f51f726a40ed0fc"}}' $addr)

echo -n "test fetch-balance..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"fetch-balance", "params":{"address":"'$wallet'"}}' $addr)

echo -n "test fetch-history..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"fetch-history", "params":{"address":"'$wallet'"}}' $addr)

echo -n "test status..."
check_response $(curl -s -X POST --data '{"id":1, "version":"2.0", "method":"status"}' $addr)

echo "Errors: " $errors

