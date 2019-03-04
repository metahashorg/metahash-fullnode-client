#ifndef __CREATE_TX_BASE_HANDLER_H__
#define __CREATE_TX_BASE_HANDLER_H__

#include <string>
#include "network_handler.h"
#include "http_session_ptr.h"
#include "../wallet_storage/wallet_storage.h"

class create_tx_base_handler : public base_network_handler
{
public:
    create_tx_base_handler(http_session_ptr session);
    virtual ~create_tx_base_handler() override { }

protected:
    bool check_params();
    bool build_request();

protected:
    mh_count_t          m_fee;
    mh_count_t          m_value;
    mh_count_t          m_nonce;
    std::string         m_address;
    std::string         m_to;
    std::string         m_data;
    storage::crypt_keys m_keys;
};

#endif // __CREATE_TX_BASE_HANDLER_H__
