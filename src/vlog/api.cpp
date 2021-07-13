/*
 * @author Larry Gonzalez
 *
 */

#include <vlog/api.h>

std::string tostring(std::vector<std::string> vector){
    std::string result = "[";
    int len = vector.size();
    for (size_t i=0; i<len-1; ++i)
        result += vector[i] + ", ";
    if (len)
        result += vector[len-1];
    result += std::string("]");
    return result;
}

std::string tostring(std::vector<size_t> vector){
    std::string result = "[";
    int len = vector.size();
    for (size_t i=0; i<len-1; ++i)
        result += std::to_string(vector[i]) + ", ";
    if (len)
        result += std::to_string(vector[len-1]);
    result += std::string("]");
    return result;
}

void log(std::string msg){
//    std::cout << msg << std::endl;
}
    
void API::addRule(std::string rule){
    log("  API::addRule: start");
    std::string result = program->parseRule(rule, false);
    log("  API::addRule: " + result);
    log("  API::addRule: end");
}

void API::addFacts(std::string predicate, std::vector<std::vector<std::string>> data){
    log("  API::addFacts: predicate: " + predicate);
    program->getOrAddPredicate(predicate, data[0].size());
    layer->addInmemoryTable(predicate, data);
}

void API::materialize(bool skolem, unsigned long timeout){
    log("  API::materialize: start");
    log("  API::materialize: number of rules: " + std::to_string(program->getAllRules().size()));
    sn = new SemiNaiver(
            *layer,
            program,
            true,//opt_intersect
            false,//opt_filtering
            false,//multithreaded
            (bool) skolem ? TypeChase::SKOLEM_CHASE : TypeChase::RESTRICTED_CHASE,
            -1,
            false,//shuffleRules
            false//ignoreExistentialRules
            );
    //std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    sn->run(&timeout);
    //sn->printCountAllIDBs("");
    //std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    //LOG(INFOL) << "Runtime materialization = " << sec.count() * 1000 << " milliseconds";
    log("  API::materialize: end");
}

std::vector<std::vector<std::string>> API::query(std::string query) {
    log("  API::query: start");
    std::vector<std::vector<std::string>> result;

    Literal queryLiteral = getVLogLiteral(query);
    log("  queryLiteral arity: "+ std::to_string(layer->getPredArity(queryLiteral.getPredicate().getCardinality())));
    log("  API::query: queryLiteral built");
    log("    " + queryLiteral.toprettystring(program, layer, true));
    log("    " + queryLiteral.getPredicate().getType() == EDB ? "EDB": "IDB");

    TupleIterator *iter = NULL;
    Reasoner reasoner((uint64_t) 0);
    if (queryLiteral.getPredicate().getType() == EDB) {
        log("  API::query: calling reasoner.getEDBIterator");
        iter = reasoner.getEDBIterator(queryLiteral, NULL, NULL, *layer, true, NULL);
    } else if (sn != NULL) {
        log("  API:query: calling reasoner.getIteratorWithMaterialization");
        iter = reasoner.getIteratorWithMaterialization(sn, queryLiteral, false, NULL);
    } else {
        // No materialization yet, but non-EDB predicate ... so, empty.
        log("  API:query: creating empty tupletable");
        TupleTable *table = new TupleTable(queryLiteral.getNConstants()+queryLiteral.getNVars());
        std::shared_ptr<TupleTable> pt(table);
        iter = new TupleTableItr(pt);
    }

    log("  API:query: iterating");
    while(iter->hasNext()){
        log("  API:query: iterating ...");
        iter -> next();
        std::vector<std::string> row;
        for (uint8_t i = 0; i< iter->getTupleSize(); i++)
            row.push_back(layer->getDictText(iter->getElementAt(i)));
        result.push_back(row);
    }
    log("  API::query: end");
    return result;
}

API::API() {
    Logger::setMinLevel(ERRORL);
    EDBConf conf("", false);
    layer = new EDBLayer(conf, false);
    program = new Program(layer);
}

API::~API(){
    if (layer != NULL) {
        delete layer;
        layer = NULL;
    }
    if (program != NULL) {
        delete program;
        program = NULL;
    }
    if (sn != NULL) {
        delete sn;
        sn = NULL;
    }
}

/*
 * Get string representation of the literal's predicate name
 */
std::string getPredicateFromLiteral(std::string literal){
    return literal.substr(0,literal.find("("));
}

/*
 * Get list of string representation of terms
 */
std::vector<std::string> getTermsFromLiteral(std::string literal){
    std::vector<std::string> result;
    size_t aux;
    size_t start = literal.find("(")+1;
    size_t stop = literal.find(")");
    while(start < stop) {
        aux = literal.find(",",start);
        if (aux == std::string::npos) {
            result.push_back(literal.substr(start, stop-start));
            break;
        } else{
            result.push_back(literal.substr(start,aux-start));
            start = aux+1;
        }
    }
    return result;
}


Literal API::getVLogLiteral(std::string literal){
    log("  API::getVLogLiteral: start");
    std::string predicateName = getPredicateFromLiteral(literal);
    log("  API::getVLogLiteral: predicateName: " + predicateName);
    Predicate predicate = program->getPredicate(predicateName);
    log("  API::getVLogLiteral: predicate's cardinality: " + std::to_string(predicate.getCardinality()));
    std::vector<std::string> terms = getTermsFromLiteral(literal);
    log("  API::getVLogLiteral: terms: " + tostring(terms));

    Dictionary variables;
    VTuple tuple((uint8_t) terms.size());
    for(uint8_t i=0; i<terms.size(); i++)
        tuple.set(getVLogTerm(terms[i], variables), i);
    Literal lit(predicate, tuple);
    log("  API::getVLogLiteral: end");
    return lit;
}

VTerm API::getVLogTerm(std::string term, Dictionary &variables){
    log("  API::getVLogTerm: start");
    VTerm result;
    if (std::isupper(term.at(0))){
        // variable
        log("  API::getVLogTerm: variable");
        uint64_t var_id = variables.getOrAdd(term);
        if (var_id != (Var_t) var_id) {
            log("  API::getVLogTerm: ERROR: too many variables");
            throw "Too many variables in rule (>255)";
        }
        result = VTerm((Var_t) var_id, 0);
    } else {
        // constant
        log("  API::getVLogTerm: constant");
        uint64_t term_id;
        if (! layer->getOrAddDictNumber(term.c_str(),term.size(),term_id)){
            log("  API::getVLogTerm: ERROR: cant add dictnumber:");
            log("  -> term:" + term);
            log("  -> term.size():" + std::to_string(term.size()));
            log("  -> term_id:" + std::to_string(term_id));
            throw "Can't add term.";
        }
        result = VTerm(0,term_id);
    }
    log("  API::getVLogTerm: end");
    return result;
}
