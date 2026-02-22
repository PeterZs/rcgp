#include <rcgp.hpp>

using namespace rcgp;

template <typename V, typename I>
struct DrawParameters {
	// MirrorBuffer <
};

struct CommandStream : CommandBuffer {
};

// TODO: experimental command buffer recording change:
// direct invocations for pipelines
// and promises/futures for synchronization
//
// for the current descriptor set only model:
// auto params = DrawParameters { vb, ib };
// pipeline.draw(dset1, dset2, ..., pc, params, DrawSize(count, 1));
//
// has to be done in a recording(cmd) { ... } environment, which can
// be enforced if draw is a macro _draw(_context, ...)?
//
// nah, just keep it cmd.draw(pipeline, <dsets...>, <push constants...>, <dispatch specific params....>, <size...>);
//
// where _context is a recording context used for caching items to avoid recording
//
// also holds a command buffer

// TODO: how much the topology really matter to the pipeline signature?

int main() {}
