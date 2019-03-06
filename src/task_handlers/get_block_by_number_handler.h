#ifndef __GET_BLOCK_BY_NUMBER_HANDLER_H__
#define __GET_BLOCK_BY_NUMBER_HANDLER_H__

#include "network_handler.h"

class get_block_by_number_handler : public base_network_handler
{
public:
    get_block_by_number_handler(http_session_ptr session);
    virtual ~get_block_by_number_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
    virtual void processResponse(json_rpc_reader &reader) override;

private:
    int64_t m_number;
    int64_t m_type;
    int64_t m_countTxs;
    int64_t m_beginTx;
};

#endif // __GET_BLOCK_BY_NUMBER_HANDLER_H__
