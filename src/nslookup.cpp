#include "nslookup.h"
#include "check.h"
#include <curl/curl.h>

#include <array>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>

#include <duration.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include "settings/settings.h"
#include "log.h"

#include "common/stopProgram.h"
#include "http_json_rpc_request_ptr.h"
#include "http_json_rpc_request.h"
#include "json_rpc.h"
#include <boost/exception/all.hpp>
#include <boost/asio/io_context.hpp>

static std::string parse_record(unsigned char *buffer, size_t r, ns_sect s, int idx, ns_msg *m) {
    ns_rr rr;
    const int k = ns_parserr (m, s, idx, &rr);
    CHECK(k != -1, "ns_parserr error " + std::string(strerror(errno)));
       
    const size_t size = NS_MAXDNAME;
    unsigned char name[size];
    int t = ns_rr_type (rr);
    
    const u_char *data = ns_rr_rdata (rr);
    if (t == T_MX) {
        int pref = ns_get16 (data);
        ns_name_unpack (buffer, buffer + r, data + sizeof (u_int16_t),
                        name, size);
        char name2[size];
        ns_name_ntop (name, name2, size);
        return std::to_string(pref) + " " + name2;
    } else if (t == T_A) {
        unsigned int addr = ns_get32 (data);
        struct in_addr in;
        in.s_addr = ntohl (addr);
        char *a = inet_ntoa (in);
        return a;
    } else if (t == T_NS) {
        ns_name_unpack (buffer, buffer + r, data, name, size);
        char name2[size];
        ns_name_ntop (name, name2, size);
        return name2;
    } else {
        return "unhandled record";
    }
}

static void nsLookup(const std::string &server, std::vector<std::string>& result) {
    curl_global_init(CURL_GLOBAL_ALL);
    
    std::array<unsigned char, 8192> buffer;
    
    const char *host = server.c_str();
    
    const int r = res_query(host, C_IN, T_A, buffer.data(), buffer.size());
    CHECK(r != -1, std::to_string(h_errno) + " " + hstrerror(h_errno));
    CHECK(r <= static_cast<int>(buffer.size()), "Buffer too small reply truncated");
    
    const HEADER *hdr = reinterpret_cast<HEADER*>(buffer.data());
    CHECK(hdr->rcode == NOERROR, "ns error " + std::to_string(hdr->rcode));
    
    const int answers = ntohs(hdr->ancount);
           
    ns_msg m;
    const int k = ns_initparse(buffer.data(), r, &m);
    CHECK(k != -1, "ns_initparse error " + std::string(strerror(errno)));
      
    for (int i = 0; i < answers; ++i) {
        result.emplace_back(parse_record(buffer.data(), r, ns_s_an, i, &m));
    }
}

static int writer(char *data, size_t size, size_t nmemb, std::string *buffer) {
    int result = 0;
    if (buffer != NULL) {
        buffer->append(data, size * nmemb);
        result = size * nmemb;
    }
    return result;
}

static std::unique_ptr<CURL, void(*)(void*)> getInstance() {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    CHECK(curl != nullptr, "curl == nullptr");
    
    return curl;
}

static std::string request(CURL* instance, const std::string& url, const std::string& postData, const std::string& header, const std::string& password) {   
    CHECK(instance != nullptr, "Incorrect curl instance");
    CURL* curl = instance;
    CHECK(curl != nullptr, "curl == nullptr");
    
    /* enable TCP keep-alive for this transfer */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    /* keep-alive idle time to 120 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    /* interval time between keep-alive probes: 60 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
    /* complete connection within 4 seconds */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4L);

    std::string buffer;
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> headers(nullptr, curl_slist_free_all);
    if (!header.empty()) {
        headers.reset(curl_slist_append(headers.get(), header.c_str()));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get());
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (!postData.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    if (!password.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    }
    
    const CURLcode res = curl_easy_perform(curl);
    CHECK(res == CURLE_OK, "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    
    return buffer;
}

static std::string request(const std::string& url, const std::string &postData, const std::string& header, const std::string& password) {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> ci = getInstance();
    return request(ci.get(), url, postData, header, password);
}

static bool validateIpAddress(const std::string &ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool get_ip_addresses(const std::string &address, std::vector<NsResult>& ip)
{
    std::string server = address;
    const auto foundScheme = server.find("://");
    std::string scheme;
    if (foundScheme != server.npos) {
        scheme = server.substr(0, foundScheme + 3);
        server = server.substr(foundScheme + 3);
    }

    const auto foundPort = server.find(':');
    int port = 0;
    if (foundPort != server.npos) {
        port = std::stoi(server.substr(foundPort + 1));
        server = server.substr(0, foundPort);
    }

    if (validateIpAddress(server)) {
        ip.emplace_back(scheme + server + ((port != 0) ? (":" + std::to_string(port)) : ""), 0);
        return true;
    }

    std::vector<std::string> result;
    nsLookup(server, result);

    for (const std::string &r: result) {
        const std::string serv = scheme + r + ((port != 0) ? (":" + std::to_string(port)) : "");
        common::Timer tt;
        try {
            request(serv, "", "", "");
            tt.stop();
            ip.emplace_back(serv, tt.countMs());
        } catch (const common::exception &e) {
            ip.emplace_back(serv, milliseconds(999s).count());
        }
    }

    std::sort(ip.begin(), ip.end(), [](const NsResult &first, const NsResult &second) {
        return first.timeout < second.timeout;
    });

    return !ip.empty();
}

void on_get_count_blocks()
{

}

void lookup_best_ip()
{
    try {
        std::string tor = settings::server::get_tor();
        std::string proxy = settings::server::get_proxy();
        std::chrono::system_clock::time_point tp;

        pthread_t pt = pthread_self();
        struct sched_param params;
        params.sched_priority = sched_get_priority_min(SCHED_OTHER);
        pthread_setschedparam(pt, SCHED_OTHER, &params);

        std::vector<NsResult> tors;
        std::vector<NsResult> prxs;

        const json_response_type* response;
        http_json_rpc_request_ptr req = std::make_shared<http_json_rpc_request>("", 3000, 2000, 1);
        req->set_path("get-count-blocks");
        req->set_body("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\"}");

        json_rpc_reader reader;
        const rapidjson::Value* tmp;
        unsigned int max_blocks = 0;

        while (true) {
            try {
                tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(60);

                common::checkStopSignal();

                tors.clear();
                if (!get_ip_addresses(settings::server::torName, tors)) {
                    LOGERR << "Lookup ip. Could not get torrent ip-addresses from " << settings::server::torName;
                } else {
                    max_blocks = 0;
                    for (auto& i: tors) {
                        if (i.timeout >= 2000) {
                            continue;
                        }
                        req->set_host(i.server.c_str());
                        req->reset_attempts();
                        req->execute();
                        response = req->get_response();
                        if (!response) {
                            LOGERR << "Lookup ip. Could not get response from get-count-blocks";
                            continue;
                        }
                        if (response->get().body().empty()) {
                            LOGERR << "Lookup ip. Could not get get-count-blocks";
                            continue;
                        }
                        if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                            LOGERR << "Lookup ip. Could not parse get-count-blocks (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                            continue;
                        }
                        tmp = reader.get_result();
                        if (tmp == nullptr) {
                            LOGERR << "Lookup ip. Did not find result in get-count-blocks";
                            continue;
                        }
                        if (!reader.get_value(*tmp, "count_blocks", i.blocks_count)) {
                            LOGERR << "Lookup ip. Did not find field 'count_blocks' in get-count-blocks";
                            continue;
                        }
                        max_blocks = std::max(max_blocks, i.blocks_count);
                    }
                    if (max_blocks > 9) {
                        max_blocks -= 10;
                    }
                    std::vector<NsResult>::const_iterator it = tors.cend();
                    for (it = tors.cbegin(); it != tors.cend(); it++) {
                        if (it->blocks_count > max_blocks) {
                            break;
                        }
                    }
                    if (it != tors.cend()) {
                        if (tor != it->server) {
                            tor = it->server;
                            settings::server::set_tor(tor);
                            LOGINFO << "Changed torrent address: " << it->server << " " << it->timeout << " ms, blocks " << it->blocks_count;
                        }
                    }
                }

                common::checkStopSignal();

                prxs.clear();
                if (!get_ip_addresses(settings::server::proxyName, prxs)) {
                    LOGERR << "Lookup ip. Could not get proxy ip-address from " << settings::server::proxyName;
                } else {
                    if (proxy != prxs.begin()->server) {
                        proxy = prxs.begin()->server;
                        settings::server::set_proxy(proxy);
                        LOGINFO << "Lookup ip. Changed proxy address: " << prxs.begin()->server << " " << prxs.begin()->timeout << " ms";
                    }
                }

                common::checkStopSignal();
                std::this_thread::sleep_until(tp);

            } catch (const common::exception &e) {
                LOGERR << __func__ << " error: " << e;
                std::this_thread::sleep_until(tp);
            }
        }

    } catch (const std::exception &e) {
        LOGERR << __func__ << " std error: " << e.what();
    } catch (const common::StopException &e) {
        LOGINFO << __func__ << " Stoped";
    } catch (...) {
        LOGERR << __func__ << " Unknown error";
    }
}
