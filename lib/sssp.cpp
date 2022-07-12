#include <string>
#include <napi.h>
#include "datastructs.cxx"
#include "sssp.cxx"

using address_t = std::string;
using price_t = long double;
using index_t = int;
using pair_t = arb::pair_t<address_t, price_t>;
using pairs_t = arb::pairs_t<index_t, pair_t>;

pairs_t pairs;

void AddPool(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsString() || !info[1].IsString() || !info[2].IsString() || !info[3].IsString() || !info[4].IsString())
    {
        Napi::TypeError::New(env, "Wrong Arguments").ThrowAsJavaScriptException();
    }

    pair_t pair(info[0].ToString().Utf8Value(), stold(info[1].ToString().Utf8Value()), info[2].ToString().Utf8Value(), stold(info[3].ToString().Utf8Value()), info[4].ToString().Utf8Value());
    pairs.addEntry(pair);
}

void PrintPools(const Napi::CallbackInfo &info)
{
    pairs.print();
}

void Sssp(const Napi::CallbackInfo &info)
{
    arb::graph_t<arb::csr_t<pairs_t>> graph(pairs);
    spfa(graph);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "addPool"), Napi::Function::New(env, AddPool));
    exports.Set(Napi::String::New(env, "printPools"), Napi::Function::New(env, PrintPools));
    exports.Set(Napi::String::New(env, "sssp"), Napi::Function::New(env, Sssp));
    return exports;
}

NODE_API_MODULE(sssp, InitAll)