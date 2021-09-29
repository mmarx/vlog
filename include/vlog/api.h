/*
 * @author Larry Gonzalez
 * Based on https://raw.githubusercontent.com/karmaresearch/vlog/master/src/vlog/java/native/VLog.cpp
 *
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>

#ifndef VLOGAPI_API_H
#define VLOGAPI_API_H

class API {
private:
    class APIOpaque;
    class APIOpaque *d_ptr_;
  //  Literal getVLogLiteral(std::string literal);
  //    VTerm getVLogTerm(std::string literal, Dictionary &variables);
public:
    void addRule(std::string rule);
    void addFacts(std::string predicate, std::vector<std::vector<std::string>> data);
    void materialize(bool skolem = false, unsigned long timeout = 0);
    void logEDBLayer();
    std::vector<std::vector<std::string>> query(std::string query);
    API();
    ~API();
};

std::string getPredicateFromLiteral(std::string literal);
std::vector<std::string> getTermsFromLiteral(std::string literal);

#endif //VLOGAPI_API_H

