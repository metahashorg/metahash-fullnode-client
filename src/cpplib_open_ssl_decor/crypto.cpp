#include <vector>
#include <string>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/x509.h>

#include "crypto.h"



std::string bin2hex(
  const std::vector<unsigned char> & message)
{
  static const char HexLookup[513]=
  {
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
  };

  std::string res;
  res.reserve(message.size() * 2 + 1);

  for (uint i = 0; i < message.size(); i++)
  {
    const char * hex = HexLookup + 2 * ((unsigned char)message[i]);
    res.insert(res.end(), hex, hex + 2);
  }

  return res;
}



std::vector<unsigned char> hex2bin(const std::string & src)
{
  static const unsigned char DecLookup[256] =
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // gap before first hex digit
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,       // 0123456789
    0,0,0,0,0,0,0,             // :;<=>?@ (gap)
    10,11,12,13,14,15,         // ABCDEF
    0,0,0,0,0,0,0,0,0,0,0,0,0, // GHIJKLMNOPQRS (gap)
    0,0,0,0,0,0,0,0,0,0,0,0,0, // TUVWXYZ[/]^_` (gap)
    10,11,12,13,14,15,         // abcdef
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 // fill zeroes
  };

  uint i = 0;
  if (src.size() > 2 && src[0] == '0' && src[1] == 'x')
    i = 2;

  std::vector<unsigned char> dest;
  dest.reserve(src.length()/2);
  for (; i < src.length(); i += 2 )
  {
    unsigned char d =  DecLookup[(unsigned char)src[i]] << 4;
    d |= DecLookup[(unsigned char)src[i + 1]];
    dest.push_back(d);
  }

  return dest;
}



void crypto_generate_keypair(EVP_PKEY **ppkey)
{
  // create private/public key pair
  EVP_PKEY_CTX *keyCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
  if(!keyCtx)
    throw std::runtime_error("Cannot create key context");

  try {
    if(EVP_PKEY_paramgen_init(keyCtx) != 1)
      throw std::runtime_error("Cannot initialise key parameters");

    if(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(keyCtx, NID_secp256k1) <= 0) // see man page
      throw std::runtime_error("Cannot set secp256k1 algorithm");

    if(EVP_PKEY_keygen_init(keyCtx) != 1)
      throw std::runtime_error("Cannot initialise keys generator");

    *ppkey = NULL; // SIGSEGV otherwise
    if(EVP_PKEY_keygen(keyCtx, ppkey) != 1)
      throw std::runtime_error("Cannot generate keys");
  
  } catch(...) {
      EVP_PKEY_CTX_free(keyCtx);
      throw;
  }
  
  EVP_PKEY_CTX_free(keyCtx);
}


void crypto_dump_key(
  EVP_PKEY *pkey,
  bool pubkey,
  std::vector<unsigned char> &key_buf)
{
  // create a place to dump the IO, in this case in memory
  BIO *bio = BIO_new(BIO_s_mem());
  if(!bio)
    throw std::runtime_error("Cannot create BIO object");

  try {
    if(pubkey)
    {
      // dump key to IO
      if(i2d_PUBKEY_bio(bio, pkey) != 1)
        throw std::runtime_error("Cannot write public key to BIO");
    }
    else
    {
      // dump key to IO
      if(i2d_PrivateKey_bio(bio, pkey) != 1)
        throw std::runtime_error("Cannot private write key to BIO");
    }

    // get buffer size
    int size = BIO_pending(bio);
    
    // allocate enough space for key data
    key_buf.resize(size);

    if(BIO_read(bio, key_buf.data(), size) <= 0)
      throw std::runtime_error("Cannot read BIO");

  } catch(...) {
    BIO_free(bio);
    throw;
  }

  BIO_free(bio);
}


inline void crypto_dump_private_key(
  EVP_PKEY *pkey,
  std::vector<unsigned char> &private_key_buf)
{
  crypto_dump_key(pkey, false, private_key_buf);
}


inline void crypto_dump_public_key(
  EVP_PKEY *pkey,
  std::vector<unsigned char> &public_key_buf)
{
  crypto_dump_key(pkey, true, public_key_buf);
}


void crypto_load_key(
  const std::vector<unsigned char> &key_buf,
  bool pubkey,
  EVP_PKEY **ppkey)
{
  // create a place to dump the IO, in this case in memory
  BIO *bio = BIO_new_mem_buf(key_buf.data(), key_buf.size());
  if(!bio)
    throw std::runtime_error("Cannot create BIO object");

  try {
    if(pubkey)
    {
      // dump key buf from IO
      *ppkey = d2i_PUBKEY_bio(bio, NULL);
      if(!*ppkey)
        throw std::runtime_error("Cannot load public key from BIO");
    }
    else
    {
      // dump key buf from IO
      *ppkey = d2i_PrivateKey_bio(bio, NULL);
      if(!*ppkey)
        throw std::runtime_error("Cannot load private key from BIO");
    }

  } catch(...) {
    BIO_free(bio);
    throw;
  }

  BIO_free(bio);
}

inline void crypto_load_private_key(
  const std::vector<unsigned char> &private_key_buf,
  EVP_PKEY **ppkey)
{
  crypto_load_key(private_key_buf, false, ppkey);
}


inline void crypto_load_public_key(
  const std::vector<unsigned char> &public_key_buf,
  EVP_PKEY **ppkey)
{
  crypto_load_key(public_key_buf, true, ppkey);
}


void digest_message(
  const EVP_MD *type,
  const std::vector<unsigned char> &data,
  std::vector<unsigned char> &pdigest_buf)
{
  EVP_MD_CTX *mdctx;

  mdctx = EVP_MD_CTX_create();
  if(!mdctx)
    throw std::runtime_error("Digest context create problem");

  try {
    if(EVP_DigestInit_ex(mdctx, type, NULL) != 1)
      throw std::runtime_error("Digest init problem");

    if(EVP_DigestUpdate(mdctx, data.data(), data.size()) != 1)
      throw std::runtime_error("Digest update problem");

    unsigned int pdigest_size;
    pdigest_buf.resize(EVP_MD_size(type));

    if(EVP_DigestFinal_ex(mdctx, pdigest_buf.data(), &pdigest_size) != 1)
      throw std::runtime_error("Digest processing problem");

    pdigest_buf.resize(pdigest_size);

    } catch(...) {
    EVP_MD_CTX_destroy(mdctx);
    throw;
  }

  EVP_MD_CTX_destroy(mdctx);
}


void crypto_generate_address(
  const std::vector<unsigned char> &public_key_buf,
  std::vector<unsigned char> &address_buf)
{
  // Take last 65 bytes of public key
  std::vector<unsigned char> buf1;
  buf1.assign(public_key_buf.end() - 65, public_key_buf.end());

  // sha256
  std::vector<unsigned char> buf2;
  digest_message(EVP_sha256(), buf1, buf2);
  
  // ripemd160
  std::vector<unsigned char> buf3;
  digest_message(EVP_ripemd160(), buf2, buf3);

  // Add 0 byte upfront
  buf3.insert(buf3.begin(), 0);

  // sha256 twice
  digest_message(EVP_sha256(), buf3, buf1);
  digest_message(EVP_sha256(), buf1, buf2);
  
  // concatenate
  address_buf.assign(buf3.begin(), buf3.end());
  address_buf.insert(address_buf.end(), buf2.begin(), buf2.begin() + 4);
}


void crypto_sign_data(
  std::vector<unsigned char>& sign, 
  const std::string& private_key, 
  const unsigned char *data, 
  int data_size, 
  const std::string& password)
{
  EVP_PKEY *pkey = NULL;

  std::vector<unsigned char> private_key_buf = hex2bin(private_key);
  crypto_load_private_key(private_key_buf, &pkey);

  /* Create the Message Digest Context */
  EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
  if(!mdctx)
    throw std::runtime_error("Cannot create digest context");
  
  try {
    // Initialise the DigestSign operation - SHA-256 has been selected as the message digest function
    if(EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey) != 1)
      throw std::runtime_error("Cannot initialise SHA-256 signing");

    // Call update with the message
    if(EVP_DigestSignUpdate(mdctx, data, data_size) != 1)
      throw std::runtime_error("Cannot sign the data");
    
    // Finalise the DigestSign operation
    // First call EVP_DigestSignFinal to obtain the length of the signature.
    size_t signature_size = 0;
    if(EVP_DigestSignFinal(mdctx, NULL, &signature_size) != 1)
      throw std::runtime_error("Cannot obtain signature size");
    
    // Reserve memory for the signature based on size returned
    sign.resize(signature_size);
    
    // Obtain the signature
    if(EVP_DigestSignFinal(mdctx, sign.data(), &signature_size) != 1)
      throw std::runtime_error("Cannot obtain signature");

    sign.resize(signature_size);
  } catch(...) {
    if(pkey) EVP_PKEY_free(pkey);
    EVP_MD_CTX_destroy(mdctx);
    throw;
  }

  if(pkey) EVP_PKEY_free(pkey);
  EVP_MD_CTX_destroy(mdctx);
}


bool crypto_check_sign_data(
  const std::vector<unsigned char>& sign, 
  const std::string& public_key, 
  const unsigned char *data,
  int data_size)
{
  bool res = false;

  EVP_PKEY *pkey = NULL;

  /* Create the Message Digest Context */
  EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
  if(!mdctx)
    throw std::runtime_error("Cannot create digest context");
  
  try {
    std::vector<unsigned char> public_key_buf = hex2bin(public_key);
    crypto_load_public_key(public_key_buf, &pkey);

    // Initialise the DigestSign operation - SHA-256 has been selected as the message digest function
    if(EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey) != 1)
      throw std::runtime_error("Cannot initialise SHA-256 verifying");

    // Call update with the message
    if(EVP_DigestVerifyUpdate(mdctx, data, data_size) != 1)
      throw std::runtime_error("Cannot verify the data");

    // Finalise the DigestVerify operation
    int rc = EVP_DigestVerifyFinal(mdctx, sign.data(), sign.size());
    if(rc == 0)
      res = false;
    else if(rc == 1)
      res = true;
    else
      throw std::runtime_error("Serious error of signature verification");

  } catch(...) {
    if(pkey) EVP_PKEY_free(pkey);
    EVP_MD_CTX_destroy(mdctx);
    throw;
  }

  if(pkey) EVP_PKEY_free(pkey);
  EVP_MD_CTX_destroy(mdctx);
  
  return res;
}


void CRYPTO_generate_wallet(
  CRYPTO_wallet& wallet,
  const std::string& password)
{
  EVP_PKEY *pkey;
  crypto_generate_keypair(&pkey);

  try {
    crypto_dump_public_key(pkey, wallet.public_key_buf);
    crypto_dump_private_key(pkey, wallet.private_key_buf);

  } catch(...) {
    EVP_PKEY_free(pkey);
    throw;
  }
  if(pkey) EVP_PKEY_free(pkey);

  crypto_generate_address(wallet.public_key_buf, wallet.mh_address_buf);
}


void CRYPTO_generate_wallet(
  std::string& private_key, 
  std::string& public_key, 
  std::string& mh_address, 
  const std::string& password)
{
  CRYPTO_wallet wallet;
  CRYPTO_generate_wallet(wallet, password);

  private_key = bin2hex(wallet.private_key_buf);
  public_key  = bin2hex(wallet.public_key_buf);
  mh_address  = bin2hex(wallet.mh_address_buf);
}


void CRYPTO_init_wallet(
  CRYPTO_wallet& wallet,
  const std::string& private_key, 
  const std::string& public_key, 
  const std::string& mh_address)
{
  wallet.private_key_buf  = hex2bin(private_key);
  wallet.public_key_buf   = hex2bin(public_key);
  wallet.mh_address_buf   = hex2bin(mh_address);
}


void CRYPTO_sign_text(
  std::vector<unsigned char>& sign, 
  const std::string& private_key, 
  const std::string& text, 
  const std::string& password)
{
  crypto_sign_data(sign, private_key, (const unsigned char *)text.c_str(), text.length(), password);
}


void CRYPTO_sign_data(
  std::vector<unsigned char>& sign, 
  const std::string& private_key, 
  const std::vector<unsigned char>& data, 
  const std::string& password)
{
  crypto_sign_data(sign, private_key, data.data(), data.size(), password);
}


bool CRYPTO_check_sign_text(
  const std::vector<unsigned char>& sign, 
  const std::string& public_key, 
  const std::string& text)
{
  return crypto_check_sign_data(sign, public_key, (const unsigned char *)text.c_str(), text.length());
}


bool CRYPTO_check_sign_data(
  const std::vector<unsigned char>& sign, 
  const std::string& public_key, 
  const std::vector<unsigned char>& data)
{
  return crypto_check_sign_data(sign, public_key, data.data(), data.size());
}


void CRYPTO_generate_public(
  const std::string& private_key,
  std::string& public_key,
  const std::string& password)
{
  EVP_PKEY *pkey = NULL;
  std::vector<unsigned char> private_key_buf, public_key_buf;

  private_key_buf = hex2bin(private_key);
  crypto_load_private_key(private_key_buf, &pkey);
  
  try {
    crypto_dump_public_key(pkey, public_key_buf);

  } catch(...) {
    if(pkey) EVP_PKEY_free(pkey);
    throw;
  }

  if(pkey) EVP_PKEY_free(pkey);

  public_key = bin2hex(public_key_buf);
}


void CRYPTO_generate_address(
  const std::string& public_key,
  std::string& mh_address)
{
  std::vector<unsigned char> public_key_buf;
  std::vector<unsigned char> mh_address_buf;

  public_key_buf  = hex2bin(public_key);
  crypto_generate_address(public_key_buf, mh_address_buf);
  mh_address = bin2hex(mh_address_buf);
}


bool CRYPTO_check_address(
  const std::string& mh_address)
{
  std::vector<unsigned char> buf1, buf2, mh_address_buf;

  mh_address_buf  = hex2bin(mh_address);

  buf1.assign(mh_address_buf.begin(), mh_address_buf.end() - 4);

  // sha256 twice
  digest_message(EVP_sha256(), buf1, buf2);
  digest_message(EVP_sha256(), buf2, buf1);
  
  // compare checksum
  buf1.resize(4);
  buf2.assign(mh_address_buf.end() - 4, mh_address_buf.end());

  return buf1 == buf2;
}
