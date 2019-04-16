#ifndef __GET_BLOCK_BY_HASH_HANDLER_H__
#define __GET_BLOCK_BY_HASH_HANDLER_H__

#include "network_handler.h"

class get_block_by_hash_handler : public base_network_handler
{
public:
    get_block_by_hash_handler(http_session_ptr session);
    virtual ~get_block_by_hash_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
    virtual void process_response(json_rpc_reader &reader);

private:
    std::string m_hash;
    mh_count_t m_type;
    mh_count_t m_countTxs;
    mh_count_t m_beginTx;
};

#endif // __GET_BLOCK_BY_HASH_HANDLER_H__
