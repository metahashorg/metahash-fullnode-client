#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <string>
#include <memory>

#include "OopUtils.h"

struct BlockInfo;
struct BlockHeader;

class Statistics: public no_copyable, no_moveable {   
public:
    
    virtual void start() = 0;
    
    virtual void join() = 0;
    
    virtual ~Statistics() = default;
    
public:
    
    virtual void saveNewBlock(const BlockInfo &bi) = 0;
    
    virtual void setLastBlock(const BlockHeader &bh) = 0;
    
    virtual void incRequest(const std::string &request) = 0;
    
};

class StatisticsEmpty: public Statistics {   
public:
    
    StatisticsEmpty();
    
    void start() override;
    
    void join() override;
    
    ~StatisticsEmpty() override;
    
public:
    
    void saveNewBlock(const BlockInfo &bi) override;
    
    void setLastBlock(const BlockHeader &bh) override;
    
    void incRequest(const std::string &request) override;

};

extern std::unique_ptr<Statistics> statistics;

void setStatistics(std::unique_ptr<Statistics> &&newStatistics);

void startStatistics();

void joinStatistics();

#endif // STATISTICS_H_
