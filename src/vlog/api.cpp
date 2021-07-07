/*
 * @author Larry Gonzalez
 *
 */

#include <vlog/api.h>

void API::addRule(std::string rule){
    program->parseRule(rule,false);
}

void API::addFacts(std::string predicate, std::vector<std::vector<std::string>> data){
    layer->addInmemoryTable(predicate, data);
}

void API::materialize(bool skolem, unsigned long timeout){
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
    //std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    //LOG(INFOL) << "Runtime materialization = " << sec.count() * 1000 << " milliseconds";
}

long API::extensionsize(std::string predicate){
    Predicate pred = program->getPredicate(predicate);
    long result;
    if (pred.getType() == EDB) {
        result = static_cast<long>(layer->getPredSize(pred.getId()));
    } else {
        result = static_cast<long>(sn->getSizeTable(pred.getId()));
    }
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

