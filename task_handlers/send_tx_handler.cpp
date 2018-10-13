#include "send_tx_handler.h"
#include "task_handlers.h"
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
    CHK_PRM(reader.parse(req->get_result()), "failed on retrieve nonce: json parse error")

    auto res = reader.get_result();
    CHK_PRM(res, "failed on retrieve nonce: result not found")

    mh_count_t count_spent(0);
    CHK_PRM(reader.get_value(*res, "count_spent", count_spent), "failed on retrieve nonce: can't get spent count")

    result = count_spent + 1;
    return true;
}
