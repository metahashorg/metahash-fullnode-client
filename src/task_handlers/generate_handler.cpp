#include "generate_handler.h"
#include "../wallet_storage/wallet_storage.h"

#include "cpplib_open_ssl_decor/crypto.h"

// generate_handler
bool generate_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
        return true;
    }
    END_TRY(return false)
}

void generate_handler::execute()
{
    BGN_TRY
    {
        CRYPTO_wallet wallet;

        // can throw exception
        CRYPTO_generate_wallet(wallet);

        storage::crypt_keys keys;
        keys.pub_key = bin2hex(wallet.public_key_buf);
        keys.prv_key = bin2hex(wallet.private_key_buf);

        std::string address = "0x" + bin2hex(wallet.mh_address_buf);
        CHK_PRM(storage::keys::store(address, keys), "Failed on store keys")

        m_writer.add_result("address", address);

        // only for debug
        m_writer.add_result("pub_key", bin2hex(wallet.public_key_buf));
        m_writer.add_result("prv_key", bin2hex(wallet.private_key_buf));
    }
    END_TRY()
}
