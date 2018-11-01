#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include <openssl/evp.h>

#include "crypto.h"

using namespace std;


int main(int argc, char *argv[])
{
  /* Load all digest and cipher algorithms */
  OpenSSL_add_all_algorithms();

  try {
    string private_key;
    string public_key;
    string mh_address;
  
    CRYPTO_generate_wallet(private_key, public_key, mh_address);
    
    cout << "Private key "    << private_key  << endl;
    cout << "Public key "     << public_key   << endl;
    cout << "Wallet address " << mh_address   << endl;

    vector<unsigned char> signature;

    string text = "Hello #MetaHash!";
    cout << "Signing text \"" << text << "\"" << endl;
    CRYPTO_sign_text(signature, private_key, text);

    cout << "Verifying signature" << endl;
    if(CRYPTO_check_sign_text(signature, public_key, text))
      cout << "Signing test pass" << endl;
    else
      cout << "Signing test fail" << endl;

    cout << "----------------------------" << endl;

    cout << "Generate public from private.." << endl;
    string public_key_new;
    CRYPTO_generate_public(private_key, public_key_new);
    if(public_key == public_key_new)
      cout << "Public key match" << endl;
    else
      cout << "Public key does not match" << endl;

    cout << "Generate address from public.." << endl;
    string mh_address_new;
    CRYPTO_generate_address(public_key_new, mh_address_new);
    if(mh_address == mh_address_new)
      cout << "Address match" << endl;
    else
      cout << "Address does not match" << endl;

    cout << "Verifying address.." << endl;
    if(CRYPTO_check_address(mh_address_new))
      cout << "Address good" << endl;
    else
      cout << "Address bad" << endl;

  } catch(exception &e) {
    cerr << "Caught exception: " << e.what() << endl;
    return -1;
  }

  /* Removes all digests and ciphers */
  EVP_cleanup();

  /* if you omit the next, a small leak may be left when you make 
     use of the BIO (low level API) for e.g. base64 transformations */
  CRYPTO_cleanup_all_ex_data();

  return 0;
}
