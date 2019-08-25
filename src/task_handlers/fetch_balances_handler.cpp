#include "fetch_balances_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "utils.h"

fetch_balances_handler::fetch_balances_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session)
{
    m_duration.set_message(__func__);
}

bool fetch_balances_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found");

        auto addresses = m_reader.get("addresses", *params);
        CHK_PRM(addresses, "addresess field not found");
        CHK_PRM(addresses->IsArray(), "addresses field must be array");

        std::string tmp;
        for (const auto& v: addresses->GetArray()) {
            if (!v.IsString() || v.GetStringLength() == 0) {
                continue;
            }
            tmp.clear();
            tmp.assign(v.GetString(), v.GetStringLength());

            // validation
            if (!utils::validate_address(tmp)) {
                continue;
            }
            m_addrs.emplace_back(std::move(tmp));
        }
        CHK_PRM(!m_addrs.empty(), "addresses has not valid values");

        if (!settings::system::useLocalDatabase) {
            rapidjson::Value arr(rapidjson::kArrayType);
            for (const auto& v: m_addrs) {
                arr.PushBack(rapidjson::Value(v.c_str(), static_cast<unsigned int>(v.size()), m_writer.get_allocator()), m_writer.get_allocator());
            }
            m_writer.get_params()->AddMember("addresses", arr, m_writer.get_allocator());
        }
        return true;
    }
    END_TRY_RET(false)
}

void fetch_balances_handler::execute() {
    BGN_TRY
    {
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();
            for (const auto& v: m_addrs) {
                const torrent_node_lib::BalanceInfo balance = sync.getBalance(torrent_node_lib::Address(v));
                balanceInfoToJson(v, balance, sync.getBlockchain().countBlocks(), false, JsonVersion::V1, m_writer.get_doc(), true);
            }
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}
