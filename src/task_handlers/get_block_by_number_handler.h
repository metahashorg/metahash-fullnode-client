#ifndef __GET_BLOCK_BY_NUMBER_HANDLER_H__
#define __GET_BLOCK_BY_NUMBER_HANDLER_H__

#include "network_handler.h"

class get_block_by_number_handler : public base_network_handler
{
public:
    get_block_by_number_handler(session_context_ptr ctx);
    virtual ~get_block_by_number_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
    virtual void process_response(json_rpc_reader &reader) override;

private:
    mh_count_t m_number;
    mh_count_t m_type;
    mh_count_t m_countTxs;
    mh_count_t m_beginTx;
    bool m_from_cache;
};

#endif // __GET_BLOCK_BY_NUMBER_HANDLER_H__
