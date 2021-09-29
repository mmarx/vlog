/*
 * @author Larry Gonzalez
 *
 */

#include <vlog/api.h>
#include <vlog/concepts.h>
#include <vlog/edb.h>
#include <vlog/edbconf.h>
#include <vlog/seminaiver.h>
#include <vlog/cycles/checker.h>
#include <vlog/reasoner.h>
#include <vlog/utils.h>
#include <memory>
#include <kognac/utils.h>
#include <kognac/logs.h>

class API::APIOpaque {
public:
  SemiNaiver *sn;
  Program *program;
  EDBLayer *layer;
};

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

VTerm getVLogTerm(EDBLayer *layer, std::string term, Dictionary &variables){
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

Literal getVLogLiteral(Program *program, EDBLayer *layer, std::string literal){
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
      tuple.set(getVLogTerm(layer, terms[i], variables), i);
    Literal lit(predicate, tuple);
    log("  API::getVLogLiteral: end");
    return lit;
}

void API::addRule(std::string rule){
    log("  API::addRule: start");
    std::string result = d_ptr_->program->parseRule(rule, false);
    log("  API::addRule: " + result);
    log("  API::addRule: end");
}

void API::addFacts(std::string predicate, std::vector<std::vector<std::string>> data){
    log("  API::addFacts: predicate: " + predicate);
    d_ptr_->program->getOrAddPredicate(predicate, data[0].size());
    d_ptr_->layer->addInmemoryTable(predicate, data);
}

void API::materialize(bool skolem, unsigned long timeout){
    log("  API::materialize: start");
    log("  API::materialize: number of rules: " + std::to_string(d_ptr_->program->getAllRules().size()));
    d_ptr_->sn = new SemiNaiver(
            *d_ptr_->layer,
            d_ptr_->program,
            true,//opt_intersect
            false,//opt_filtering
            false,//multithreaded
            (bool) skolem ? TypeChase::SKOLEM_CHASE : TypeChase::RESTRICTED_CHASE,
            -1,
            false,//shuffleRules
            false//ignoreExistentialRules
            );
    //std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    d_ptr_->sn->run(&timeout);
    //d_ptr_->sn->printCountAllIDBs("");
    //std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    //LOG(INFOL) << "Runtime materialization = " << sec.count() * 1000 << " milliseconds";
    log("  API::materialize: end");
}

std::vector<std::vector<std::string>> API::query(std::string query) {
    log("  API::query: start");
    std::vector<std::vector<std::string>> result;

    Literal queryLiteral = getVLogLiteral(d_ptr_->program,d_ptr_->layer,query);
    log("  queryLiteral arity: "+ std::to_string(d_ptr_->layer->getPredArity(queryLiteral.getPredicate().getCardinality())));
    log("  API::query: queryLiteral built");
    log("    " + queryLiteral.toprettystring(d_ptr_->program, d_ptr_->layer, true));
    log("    " + queryLiteral.getPredicate().getType() == EDB ? "EDB": "IDB");

    TupleIterator *iter = NULL;
    Reasoner reasoner((uint64_t) 0);
    if (queryLiteral.getPredicate().getType() == EDB) {
        log("  API::query: calling reasoner.getEDBIterator");
        iter = reasoner.getEDBIterator(queryLiteral, NULL, NULL, *d_ptr_->layer, true, NULL);
    } else if (d_ptr_->sn != NULL) {
        log("  API:query: calling reasoner.getIteratorWithMaterialization");
        iter = reasoner.getIteratorWithMaterialization(d_ptr_->sn, queryLiteral, false, NULL);
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
        for (uint8_t i = 0; i< iter->getTupleSize(); i++){
            std::string value = d_ptr_->layer->getDictText(iter->getElementAt(i));
            if (value == ""){
                //uint64_t v = iitr->getCurrentValue(m);
                uint64_t v = iter->getElementAt(i);
                value = "_:" + std::to_string(v >> 40) + "_" + std::to_string((v >> 32) & 0377) + "_" + std::to_string(v & 0xffffffff);
            }
            row.push_back(value);
        }

        result.push_back(row);
    }
    log("  API::query: end");
    return result;
}

API::API() {
    Logger::setMinLevel(ERRORL);
    EDBConf conf("", false);
    d_ptr_ = new API::APIOpaque();
    d_ptr_->layer = new EDBLayer(conf, false);
    d_ptr_->program = new Program(d_ptr_->layer);
}

API::~API(){
    if (d_ptr_->layer != NULL) {
        delete d_ptr_->layer;
        d_ptr_->layer = NULL;
    }
    if (d_ptr_->program != NULL) {
        delete d_ptr_->program;
        d_ptr_->program = NULL;
    }
    if (d_ptr_->sn != NULL) {
        delete d_ptr_->sn;
        d_ptr_->sn = NULL;
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
