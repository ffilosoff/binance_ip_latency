#include "DNSLookup.h"

#include "Log.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <arpa/inet.h>

#ifndef C_ANY
#define C_ANY 255
#endif

class DNSResolver
    : public IDNSLookup
{
    std::vector<IPAddress> resolve(const std::string & domain) final
    {
        u_char res[NS_MAXDNAME];
        const auto len = res_query(domain.c_str(), C_ANY, C_ANY, res, NS_PACKETSZ);
        LOG_LINE("ret res_query(): " << len);
        if (len > 0) {
            ns_msg handle;
            if (const auto err = ns_initparse(res, len, &handle); err < 0) {
                LOG_LINE("err ns_initparse(): " << err);
                return {};
            }
            ns_rr rr;
            ns_sect section = ns_s_an;
            const auto count = ns_msg_count(handle, section);
            std::vector<IPAddress> ret;
            ret.reserve(count);
            for (size_t rrnum = 0; rrnum < count; rrnum++) {
                if (const auto err = ns_parserr(&handle, ns_s_an, rrnum, &rr); err < 0) {
                    LOG_LINE("err ns_parserr(): " << err);
                    return {};
                }
                if (ns_rr_type(rr) == ns_t_a) {
                    const auto rr_data = ns_rr_rdata(rr);
                    const auto addr = (in_addr *)(rr_data);
                    ret.emplace_back(inet_ntoa(*addr));
                }
            }
            return ret;
        }
        return {};
    }
};

std::unique_ptr<IDNSLookup> create_dns_resolver()
{
    return std::make_unique<DNSResolver>();
}