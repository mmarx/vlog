/*
 * @author Larry Gonzalez
 *
 * */

#include <vlog/api.h>

#include <iostream>

std::string to_string(std::vector<std::string> row, std::string sep) {
    int len = row.size();
    std::string result = "[";
    for (int i=0; i<len-1; ++i)
        result += row[i] + sep;
    if (len)
        result += row[len-1];
    result += "]";
    return result;
}

int main() {
    // Initialize an API
    API api;

    // Create a vector of vectors to store a predicate's extension
    std::vector<std::vector<std::string>> pExtension;
    // Add some data to it
    pExtension.push_back({"a","b"});
    pExtension.push_back({"b","c"});

    // Add the predicate name and its extension to the API instance
    api.addFacts("p", pExtension);

    /* IMPORTANT: the data must be added before the rules */
    /* IMPORTANT: terms starting with an uppercase letter are interpreted as variables */
    /* IMPORTANT: terms starting with a lowercase letter are interpreted as constants */
    // Add some rule to the API instance
    api.addRule("q(X,Z,Y) :- p(X,Y)");
    api.addRule("r(X,Y,Z) :- p(X,Y), p(Y,Z)");
    api.addRule("s(V,X,Y) :- q(X,Y,Z)");

    // Call materialize
    api.materialize();

    // Execute some queries
    std::vector<std::string> queries = {"p(X,Y)", "p(a,X)", "p(b,X)", "p(c,X)", "q(X,Y,Z)", "q(a,X,Y)", "q(c,x,X)", "r(X,Y,Z)", "s(X,Y,Z)"};
    for (uint8_t i=0; i<queries.size(); i++) {
        std::vector<std::vector<std::string>> result = api.query(queries[i]);
        std::cout << queries[i] << ": " << result.size() << std::endl;
        for(size_t j=0; j<result.size(); j++)
            std::cout << to_string(result[j], ", ") << std::endl;
        std::cout << std::endl;
    }
    return 0;
}
