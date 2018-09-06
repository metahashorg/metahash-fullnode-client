#include "task_handlers.h"
#include "../wallet_storage/wallet_storage.h"

// generate_handler
bool generate_handler::prepare_params()
{
    CHK_PRM(m_id, "id field not found")
    return true;
}

void generate_handler::execute()
{
    try
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
            m_writer.set_error(-32603, "Failed on store");
            return;
        }

        m_writer.add_result("address", address);

        // only for debug
        m_writer.add_result("pub_key", bin2hex(wallet.public_key_buf));
        m_writer.add_result("prv_key", bin2hex(wallet.private_key_buf));
    }
    catch (std::exception& e)
    {
        m_writer.set_error(-32603, e.what());
    }
}
