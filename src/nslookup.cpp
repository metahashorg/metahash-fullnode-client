#include "nslookup.h"
#include "check.h"
#include <curl/curl.h>

#include <array>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <iomanip>

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

static std::vector<std::string> nsLookup(const std::string &server) {
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
      
    std::vector<std::string> result;
    for (int i = 0; i < answers; ++i) {
        result.emplace_back(parse_record(buffer.data(), r, ns_s_an, i, &m));
    }
    return result;
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

NsResult getBestIp(const std::string &address, const char* print) {
    std::string server = address;
    const auto foundScheme = server.find("://");
    std::string scheme;
    if (foundScheme != server.npos) {
        scheme = server.substr(0, foundScheme + 3);
        server = server.substr(foundScheme + 3);
    }
    
    const auto foundPort = server.find(':');
    int port = 0;
    if (foundPort == server.npos) {
        server = server;
    } else {
        port = std::stoi(server.substr(foundPort + 1));
        server = server.substr(0, foundPort);
    }
    
    if (validateIpAddress(server)) {
        return NsResult(scheme + server + ((port != 0) ? (":" + std::to_string(port)) : ""), 0);
    }
    
    const std::vector<std::string> result = nsLookup(server);
    
    std::vector<NsResult> pr;
    for (const std::string &r: result) {
        const std::string serv = scheme + r + ((port != 0) ? (":" + std::to_string(port)) : "");
        common::Timer tt;
        try {
            request(serv, "", "", "");
            tt.stop();
            pr.emplace_back(serv, tt.countMs());
        } catch (const common::exception &e) {
            pr.emplace_back(serv, milliseconds(999s).count());
        }
    }

    std::sort(pr.begin(), pr.end(), [](const NsResult &first, const NsResult &second) {
        return first.timeout < second.timeout;
    });

    if (print) {
        std::cout << print << std::endl;
        LOGINFO << print;
        for (const auto& i: pr) {
            std::cout << std::left << std::setfill(' ') << std::setw(25) << i.server << i.timeout << " ms" << std::endl;
            LOGINFO << i.server << " " << i.timeout << " ms";
        }
    }
    
    const auto found = pr.begin();

    CHECK(found != pr.end(), "Servers empty");
    return *pr.begin();
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

        NsResult res;

        while (true) {
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(60);

            common::checkStopSignal();

            res = getBestIp(settings::server::torName);
            if (tor != res.server) {
                tor = res.server;
                settings::server::set_tor(tor);
                LOGINFO << "Changed torrent address: " << res.server << " " << res.timeout << " ms";
            }

            common::checkStopSignal();

            res = getBestIp(settings::server::proxyName);
            if (proxy != res.server) {
                proxy = res.server;
                settings::server::set_proxy(proxy);
                LOGINFO << "Changed proxy address: " << res.server << " " << res.timeout << " ms";
            }

            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    } catch (const common::exception &e) {
        LOGERR << __func__ << " error: " << e;
    } catch (const std::exception &e) {
        LOGERR << __func__ << " error: " << e.what();
    } catch (const common::StopException &e) {
        LOGINFO << __func__ << " Stoped";
    } catch (...) {
        LOGERR << __func__ << " Unknown error";
    }
}
