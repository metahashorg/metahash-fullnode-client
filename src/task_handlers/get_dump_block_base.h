#ifndef __GET_DUMP_BLOCK_BASE_H__
#define __GET_DUMP_BLOCK_BASE_H__

#include "network_handler.h"
#include <variant>

class get_dump_block_base : public base_network_handler
{
public:
    get_dump_block_base(session_context_ptr ctx);
    virtual ~get_dump_block_base() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
    virtual void set_block_id(const rapidjson::Value* params) = 0;

    std::string get_block_id() const;

protected:
    std::variant<std::size_t, std::string> m_block_id;

private:
    mh_count_t  m_fromByte;
    mh_count_t  m_toByte;
    bool        m_isHex;
};

#endif // __GET_DUMP_BLOCK_BASE_H__
