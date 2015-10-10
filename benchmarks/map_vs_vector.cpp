#include <benchmark/benchmark.h>

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>

std::vector<std::tuple<std::string, std::string> > values = {
  std::make_tuple("AideoTexture1", "Foobar1"),
  std::make_tuple("BideoTexture2", "Foobar2"),
  std::make_tuple("CideoTexture3", "Foobar3"),
  std::make_tuple("DideoTexture4", "Foobar4"),
  std::make_tuple("EideoTexture5", "Foobar5"),
  std::make_tuple("FideoTexture6", "Foobar6"),
  std::make_tuple("GideoTexture7", "Foobar7"),
  std::make_tuple("HideoTexture8", "Foobar8"),
  std::make_tuple("IideoTexture9", "Foobar9"),
};

std::string needle = "EideoTexture5";

static void BM_unordered_map(benchmark::State& state)
{
  std::unordered_map<std::string, std::string> lst;

  for(auto const& it : values)
  {
    lst[std::get<0>(it)] = std::get<1>(it);
  }

  while (state.KeepRunning())
  {
    auto it = lst.find(needle);
    if (it != lst.end())
    {
    }
  }
}
BENCHMARK(BM_unordered_map);

static void BM_map(benchmark::State& state)
{
  std::map<std::string, std::string> lst;

  for(auto const& it : values)
  {
    lst[std::get<0>(it)] = std::get<1>(it);
  }

  while (state.KeepRunning())
  {
    auto it = lst.find(needle);
    if (it != lst.end())
    {
    }
  }
}
BENCHMARK(BM_map);

static void BM_vector(benchmark::State& state)
{
  while (state.KeepRunning())
  {
    auto it = std::find_if(std::begin(values), std::end(values),
                           [](std::tuple<std::string, std::string> const& t){
                             return std::get<0>(t) == "VideoTexture3";
                           });
    if (it != values.end())
    {
    }
  }
}
BENCHMARK(BM_vector);

BENCHMARK_MAIN()

/* EOF */
