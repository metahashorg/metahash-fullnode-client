#include "send_tx_handler.h"
#include "../http_json_rpc_request.h"

bool send_tx_handler::get_nonce(mh_count_t& result)
{
    json_rpc_writer writer;
    writer.set_id(1);
    writer.add_param("address", m_address);

    asio::io_context ctx;
    auto req = std::make_shared<http_json_rpc_request>(settings::server::tor, ctx);
    req->set_path("fetch-balance");
    req->set_body(writer.stringify());
    req->execute();

    json_rpc_reader reader;
    if (!reader.parse(req->get_result()))
        return false;

    auto res = reader.get_result();
    if (!res)
        return false;

    mh_count_t count_spent(0);
    if (!reader.get_value(*res, "count_spent", count_spent))
        return false;

    result = count_spent + 1;
    return true;
}
