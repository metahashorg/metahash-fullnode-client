#ifndef __GET_DUMP_BLOCK_BY_NUMBER_H__
#define __GET_DUMP_BLOCK_BY_NUMBER_H__

#include "network_handler.h"

class get_dump_block_by_number : public base_network_handler
{
public:
    get_dump_block_by_number(http_session_ptr session);
    virtual ~get_dump_block_by_number() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    mh_count_t m_number;
    mh_count_t m_fromByte;
    mh_count_t m_toByte;
    bool m_isHex;
    bool m_from_cache;
};

#endif // __GET_DUMP_BLOCK_BY_NUMBER_H__
