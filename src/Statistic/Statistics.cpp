#include "Statistics.h"

using namespace common;

namespace torrent_node_lib {

std::unique_ptr<Statistics> statistics = std::make_unique<StatisticsEmpty>();

void setStatistics(std::unique_ptr<Statistics> &&newStatistics) {
    statistics = std::move(newStatistics);
}

StatisticGuard startStatistics() {
    statistics->start();
    return StatisticGuard(true);
}

static void joinStatistics() {
    statistics->abort();
    statistics->join();
}

void StatisticGuard::join() {
    if (joined) {
        return;
    }
    joinStatistics();
    joined = true;
}

StatisticGuard::StatisticGuard(bool)
    : joined(false)
{}

StatisticGuard::~StatisticGuard() {
    join();
}

StatisticsEmpty::StatisticsEmpty() {
}

StatisticsEmpty::~StatisticsEmpty() {
}

void StatisticsEmpty::join() {
}

void StatisticsEmpty::start() {
}

void StatisticsEmpty::abort() {
}

void StatisticsEmpty::incRequest(const std::string& request) {
}

void StatisticsEmpty::saveNewBlock(const BlockInfo& bi) {
}

void StatisticsEmpty::setLastBlock(const BlockHeader& bh) {
}

}
