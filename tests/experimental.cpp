#include <rcgp.hpp>

using namespace rcgp;

// TODO: experimental command buffer recording change:
// direct invocations for pipelines
// and promises/futures for synchronization
//
// for the current descriptor set only model:
// auto params = DrawParameters { vb, ib };
// pipeline.draw(dset1, dset2, ..., pc, params, DrawSize(count, 1));
//
// has to be done in a recording(cmd) { ... } environment, which can
// be enforced if draw is a macro _draw(cmd, ...)?

int main() {}
