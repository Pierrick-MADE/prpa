#include "tree_dictionary.hpp"

#include <algorithm>
#include <unordered_set>

#include "IDictionary.hpp"

Tree_Dictionary::Tree_Dictionary()
    : root_(Node('\0'))
    , book_leafs_(Tree_Dictionary::delete_map())
{}

Tree_Dictionary::Tree_Dictionary(const dictionary_t& d)
    : root_(Node('\0'))
    , book_leafs_(Tree_Dictionary::delete_map())
{
    this->_init(d);
}

void Tree_Dictionary::init(const dictionary_t& d)
{
    this->_init(d);
}

void Tree_Dictionary::_init(const dictionary_t& d)
{
    for (const auto& [book, words] : d)
        for (const auto& word : words)
            _add_word(word, book);
    root_._init_leafs(book_leafs_);
}

void Tree_Dictionary::_add_word(const char* word, int book,
                                std::vector<Leaf::book_set*>& vect)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time
            // Can't use std::call_once cause this function maybe called
            // multiple time
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
    vect.emplace_back(&(cur->get_leaf()->books));
}

void Tree_Dictionary::_add_word(const char* word, const int book)
{
    Node* cur = &root_;
    // Here data race on array but on the array itself, since we are accessing the cells in a thread-safe manner
    // There is a data race on the array but not on the cell, the programm is thus working

    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time, thread-safe
            // Can't use std::call_once cause this function maybe called
            // multiple times
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
}

void Tree_Dictionary::_search_word(const char* word, result_t& r) const
{
    const Node* cur = &root_;
    const int len = strlen(word);
    for (int i = 0; i < len; ++i)
    {
        cur = (*cur)[word[i]];
        if (cur == nullptr)
        {
            r.m_count = 0;
            return;
        }
    }

    cur->read_books(r);
}

result_t Tree_Dictionary::search(const char* word) const
{
    result_t r;
    _search_word(word, r);
    return r;
}

void Tree_Dictionary::insert(int document_id, gsl::span<const char*> text)
{
    // TODO change it to use their concurrent hashmap
    auto s = std::unordered_set<const char*>();
    for (const char* word : text)
        s.emplace(word);

    delete_map::accessor a;
    if (!book_leafs_.find(a, document_id))
    {
        book_leafs_.insert(
            a, std::make_pair(document_id, std::vector<Leaf::book_set*>{}));
    }

    if (a->second.empty())
    {
        for (const char* word : s)
        {
            _add_word(word, document_id, a->second);
        }
    }
}

void Tree_Dictionary::_remove(int document_id)
{
    delete_map::accessor a;
    if (book_leafs_.find(a, document_id))
    {
        if (a->second.empty() || a->second[0] == nullptr)
            return;

        for (size_t i = 0; i < a->second.size(); ++i)
        {
            a->second[i]->erase(std::remove(a->second[i]->begin(),
                                            a->second[i]->end(), document_id),
                                a->second[i]->end());
        }

        book_leafs_.erase(a);
    }
}

void Tree_Dictionary::remove(int document_id)
{
    _remove(document_id);
}