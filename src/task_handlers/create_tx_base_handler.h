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

    virtual bool build_request(bool create_hash = false);
    virtual void make_json();

protected:
    mh_count_t          m_fee;
    mh_count_t          m_value;
    mh_count_t          m_nonce;
    std::string         m_address;
    std::string         m_to;
    std::string         m_data;
    std::string         m_hash;
    std::string         m_sign;
    std::string         m_transaction;
    storage::crypt_keys m_keys;
};

#endif // __CREATE_TX_BASE_HANDLER_H__
