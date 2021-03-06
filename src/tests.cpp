#include <gtest/gtest.h>
#include <functional>
#include <thread>
#include "tools.hpp"
#include "naive_dictionary.hpp"
#include "naive_async_dictionary.hpp"

using namespace std::string_literals;

// TODO
// Adapt/Create new tests tests with your new structures
// naive_dictionary/async_naive_dictionary can be used as references

using dic_t = std::vector<std::vector<const char*>>;

// A basic add/remove/search test
TEST(Dictionary, Basic)
{

  dic_t d = {{"massue", "lamasse", "massive"}, //
             {"massue", "limace"},             //
             {"limace", "lamassue"}};

  naive_dictionary dic = dictionary_t{
      {0, gsl::make_span(d[0])},
      {1, gsl::make_span(d[1])},
      {2, gsl::make_span(d[2])},
  };

  {
    auto res = dic.search("massue");
    ASSERT_EQ(res.count(), 2);
    ASSERT_TRUE(res.item(0).id() == 0 || res.item(0).id() == 1);
    ASSERT_TRUE(res.item(0).id() == 1 || res.item(0).id() == 0);
  }

  {
    auto res = dic.search("masseur");
    ASSERT_EQ(res.count(), 0);
  }

  // Insertion
  {
    const char* text[] = {"masseur", "massue"};
    dic.insert(42, text);
    ASSERT_EQ(dic.search("massue").count(), 3);
    ASSERT_EQ(dic.search("masseur").count(), 1);
    ASSERT_EQ(dic.search("masseur").item(0).id(), 42);
  }

  {
    dic.remove(1);
    ASSERT_EQ(dic.search("limace").count(), 1);
    ASSERT_EQ(dic.search("limace").item(0).id(), 2);
  }
}


// A simple scenario
TEST(Dictionary, SimpleScenario)
{
  Scenario::param_t params;
  params.word_count       = 100;
  params.doc_count        = 10;
  params.word_redoundancy = 0.1f;
  params.word_occupancy   = 0.9f;
  params.n_queries        = 20;
  params.ratio_indel      = 0.2;

  Scenario scn(params);

  naive_dictionary dic;
  scn.prepare(dic);
  scn.execute(dic);
}


// A long scenario, check that the async dictionary as the
// same output as the blocking one
TEST(Dictionary, AsyncConsistency)
{
  Scenario::param_t params;
  params.word_count       = 1000;
  params.doc_count        = 30;
  params.word_redoundancy = 0.3f;
  params.word_occupancy   = 0.9f;
  params.n_queries        = 100;
  params.ratio_indel      = 0.2;


  Scenario scn(params);

  naive_dictionary dic;
  naive_async_dictionary async_dic;
  scn.prepare(dic);
  scn.prepare(async_dic);
  auto r1 = scn.execute(async_dic);
  auto r2 = scn.execute(dic);
  ASSERT_EQ(r1, r2);
}


