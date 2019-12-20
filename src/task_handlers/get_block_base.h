#ifndef __GET_BLOCK_BASE_H__
#define __GET_BLOCK_BASE_H__

#include "network_handler.h"
#include <variant>

class get_block_base : public base_network_handler
{
public:
    get_block_base(session_context_ptr ctx);
    virtual ~get_block_base() override {}

    virtual void execute() override;

protected:
    virtual void set_block_id(const rapidjson::Value* params) = 0;
    virtual bool prepare_params() override;
    virtual void process_response(json_rpc_reader &reader) override;

    std::string get_block_id() const;

protected:
    std::variant<std::size_t, std::string> m_block_id;

private:
    mh_count_t  m_type;
    mh_count_t  m_countTxs;
    mh_count_t  m_beginTx;
};

#endif // __GET_BLOCK_BASE_H__
