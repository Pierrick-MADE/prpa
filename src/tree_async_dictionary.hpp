#pragma once

#include "IAsyncDictionary.hpp"
#include "tree_dictionary.hpp"


/// The tree implementation is blocking to ensure Sequential Consistency
class tree_async_dictionary : public IAsyncReversedDictionary
{
public:
  tree_async_dictionary() = default;
  tree_async_dictionary(const dictionary_t& d);

  void init(const dictionary_t& d) final;

  std::future<result_t> search(const char* word) const final;
  std::future<void>     insert(int document_id, gsl::span<const char*> text) final;
  std::future<void>     remove(int document_id) final;


private:
  Tree_Dictionary m_dic;
};
