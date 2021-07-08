/*
 * @author Larry Gonzalez
 *
 * */

#include <vlog/api.h>

#include <iostream>

int main() {
    // Initialize an API
    API api;

    // Create a vector of vectors to store a predicate's extension
    std::vector<std::vector<std::string>> pExtension;
    // Add some data to it
    pExtension.push_back({"a"});
    pExtension.push_back({"b"});

    // Add the predicate name and its extension to the API instance
    api.addFacts("p", pExtension);

    // Add some rule to the API instance
    api.addRule("q(X) :- p(X)");
    api.addRule("r(X) :- q(X)");

    // Call materialize
    api.materialize();

    // Count extension sizes (Only for IDB predicates)
    std::cout << "q: " << api.extensionsize("q") << std::endl;
    std::cout << "r: " << api.extensionsize("r") << std::endl;

    return 0;
}
