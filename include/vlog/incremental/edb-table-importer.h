#ifndef VLOG__INCREMENTAL__EDB_TABLE_IMPORTER__H__
#define VLOG__INCREMENTAL__EDB_TABLE_IMPORTER__H__

#include <vlog/concepts.h>
#include <vlog/edb.h>
#include <vlog/seminaiver.h>

/**
 * Wrapper to just re-use an EDB table from an older EDBLayer
 */
class EDBimporter : public EDBTable {
protected:
    PredId_t    predid;
    const std::shared_ptr<SemiNaiver> prevSemiNaiver;
    std::shared_ptr<EDBTable> edbTable;
    EDBLayer *layer;

public:
    EDBimporter(PredId_t predid, EDBLayer *layer,
                const std::shared_ptr<SemiNaiver> prevSN);

    virtual ~EDBimporter();

    //execute the query on the knowledge base
    virtual void query(QSQQuery *query, TupleTable *outputTable,
                       std::vector<uint8_t> *posToFilter,
                       std::vector<Term_t> *valuesToFilter) {
        edbTable->query(query, outputTable, posToFilter, valuesToFilter);
    }

    virtual size_t estimateCardinality(const Literal &query) {
        return edbTable->estimateCardinality(query);
    }

    virtual size_t getCardinality(const Literal &query) {
        return edbTable->getCardinality(query);
    }

    virtual size_t getCardinalityColumn(const Literal &query, uint8_t posColumn) {
        return edbTable->getCardinalityColumn(query, posColumn);
    }

    virtual bool isEmpty(const Literal &query, std::vector<uint8_t> *posToFilter,
                         std::vector<Term_t> *valuesToFilter) {
        return edbTable->isEmpty(query, posToFilter, valuesToFilter);
    }

    virtual EDBIterator *getIterator(const Literal &q) {
        return edbTable->getIterator(q);
    }

    virtual EDBIterator *getSortedIterator(const Literal &query,
                                           const std::vector<uint8_t> &fields) {
        return edbTable->getSortedIterator(query, fields);
    }

    virtual void releaseIterator(EDBIterator *itr) {
        return edbTable->releaseIterator(itr);
    }

    virtual bool getDictNumber(const char *text, const size_t sizeText,
                               uint64_t &id) {
        return edbTable->getDictNumber(text, sizeText, id);
    }

    virtual bool getDictText(const uint64_t id, char *text) {
        return edbTable->getDictText(id, text);
    }

    virtual bool getDictText(const uint64_t id, std::string &text) {
        return edbTable->getDictText(id, text);
    }

    virtual uint64_t getNTerms() {
        return edbTable->getNTerms();
    }

    virtual uint8_t getArity() const {
        return edbTable->getArity();
    }

    virtual uint64_t getSize() {
        return edbTable->getSize();
    }

    virtual PredId_t getPredicateID() const {
        return predid;
    }

    std::ostream &dump(std::ostream &os);
};

#endif  // ndef VLOG__INCREMENTAL__EDB_TABLE_IMPORTER__H__
