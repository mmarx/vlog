/*
 * @author Larry Gonzalez
 *
 * */

#include <vlog/api.h>

#include <iostream>

int main() {
    API api;

    std::string predicate = "p";
    std::vector<std::string> row;
    row.push_back("a");
    std::vector<std::vector<std::string>> data;
    data.push_back(row);
    api.addFacts(predicate, data);

    api.addRule("q(X) :- p(X)");
    api.addRule("r(X) :- q(X)");

    api.materialize();

    std::cout << "q: " << api.extensionsize("q") << std::endl;
    std::cout << "r: " << api.extensionsize("r") << std::endl;

    return 0;
}
