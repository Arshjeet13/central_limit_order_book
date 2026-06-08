#include "engine/engine.hpp"
#include "gateway/server.hpp"

int main() {
    MatchingEngine engine;
    int rv = run_server(engine);
    return rv;
}
