#include "generate_handler.h"
#include "../wallet_storage/wallet_storage.h"

// generate_handler
bool generate_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        return true;
    }
    END_TRY_RET(false)
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
        if (!storage::keys::store(address, keys))
        {
            m_writer.set_error(-32604, "Failed on store");
            return;
        }

        m_writer.add_result("address", address);

        // only for debug
        m_writer.add_result("pub_key", bin2hex(wallet.public_key_buf));
        m_writer.add_result("prv_key", bin2hex(wallet.private_key_buf));
    }
    END_TRY
}
