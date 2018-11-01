# cpplib_open_ssl_decor

This repository contains Metahash wallet and signature c++ API sample code.
For more details about Metahash address generation, please read the [article](https://developers.metahash.org/hc/en-us/articles/360002712193-Getting-started-with-Metahash-network#h_683619682421524476003219).

## Get the source code
Clone the repository by:
```shell
git clone https://github.com/metahashorg/cpplib_open_ssl_decor
```

## API
```
#include "crypto.h"

void CRYPTO_generate_wallet(
  CRYPTO_wallet& wallet, 
  const std::string& password = "");

void CRYPTO_generate_wallet(
  std::string& private_key, 
  std::string& public_key, 
  std::string& mh_address, 
  const std::string& password = "");

void CRYPTO_init_wallet(
  CRYPTO_wallet& wallet,
  const std::string& private_key, 
  const std::string& public_key, 
  const std::string& mh_address);

void CRYPTO_sign_text(
  std::vector<unsigned char>& sign, 
  const std::string& private_key, 
  const std::string& text, 
  const std::string& password = "");

void CRYPTO_sign_data(
  std::vector<unsigned char>& sign, 
  const std::string& private_key, 
  const std::vector<unsigned char>& data, 
  const std::string& password = "");

bool CRYPTO_check_sign_text(
  const std::vector<unsigned char>& sign, 
  const std::string& public_key, 
  const std::string& text);

bool CRYPTO_check_sign_data(
  const std::vector<unsigned char>& sign, 
  const std::string& public_key, 
  const std::vector<unsigned char>& data);
```

`CRYPTO_generate_wallet`
> builds new wallet. 
>
> Wallet is represented by 3 strings: private and public key and wallet address.

`CRYPTO_wallet` 
> struct can be used to hide that fact. 
> 
> `$password` argument is optional and not implemented in that version.

`CRYPTO_init_wallet` 
> builds `CRYPTO_wallet` struct variable with help of given values.

`CRYPTO_sign_data` 
> uses private key to build a signature for given binary data.

`CRYPTO_check_sign_data` 
> uses public key to check if given signature matches data and returns appropriate boolean result.

`CRYPTO_sign_text` and `CRYPTO_check_sign_text` 
> They are similar to previous functions but designed to work with text data.
