/*
 * @author Larry Gonzalez
 * Based on https://raw.githubusercontent.com/karmaresearch/vlog/master/src/vlog/java/native/VLog.cpp
 *
 */

#include <vlog/concepts.h>
#include <vlog/edb.h>
#include <vlog/edbconf.h>
#include <vlog/seminaiver.h>
#include <vlog/cycles/checker.h>
#include <vlog/reasoner.h>
#include <vlog/utils.h>
#include <kognac/utils.h>
#include <kognac/logs.h>

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <vector>

#ifndef VLOGAPI_API_H
#define VLOGAPI_API_H


class API {
private:
    SemiNaiver *sn;
    Program *program;
    EDBLayer *layer;
    Literal getVLogLiteral(std::string literal);
    VTerm getVLogTerm(std::string literal, Dictionary &variables);
public:
    void addRule(std::string rule);
    void addFacts(std::string predicate, std::vector<std::vector<std::string>> data);
    void materialize(bool skolem = false, unsigned long timeout = 0);
    std::vector<std::vector<std::string>> query(std::string query);
    API();
    ~API();
};

std::string getPredicateFromLiteral(std::string literal);
std::vector<std::string> getTermsFromLiteral(std::string literal);

#endif //VLOGAPI_API_H

