#include "tree_async_dictionary.hpp"
#include <iostream>

Tree_Async_Dictionary::Tree_Async_Dictionary()
    //: thread_pool_(8)
{}

Tree_Async_Dictionary::Tree_Async_Dictionary(const dictionary_t& d)
    //: thread_pool_(8)
{
    m_dic.init(d);
}

void Tree_Async_Dictionary::init(const dictionary_t& d)
{
    m_dic.init(d);
}

// TODO put back const if needed
std::future<result_t> Tree_Async_Dictionary::search(const char* query) const
{
    // TODO Find a way to move promise inside the lambda, current problem say
    // it's const
    auto p = new std::promise<result_t>;
    auto futur = p->get_future();
    thread_pool_.push([this, query, p]() {
        p->set_value(this->m_dic.search(query));
        delete p;
    });
    return futur;
}

std::future<void> Tree_Async_Dictionary::insert(int doc_id,
                                                gsl::span<const char*> text)
{
    auto p = new std::promise<void>;
    auto futur = p->get_future();
    thread_pool_.push([this, doc_id, text, p]() {
        this->m_dic.insert(doc_id, text);
        p->set_value();
        delete p;
    });
    return futur;
}

std::future<void> Tree_Async_Dictionary::remove(int doc_id)
{
    auto p = new std::promise<void>;
    auto futur = p->get_future();
    thread_pool_.push([this, doc_id, p]()
    {
        this->m_dic.remove(doc_id);
        p->set_value();
        delete p;
    });
    return futur;
}
