#include "dsl/branching.hpp"
#include "dsl/block.hpp"
#include "dsl/instructions.hpp"
#include "dsl/tracer.hpp"

namespace rcgp {

_branch_holder::~_branch_holder()
{
	if (not ifs.has_value())
		return;

	auto trace_body = [](const _branch_body &body) {
		auto sbr = std::make_shared <Block> ();
		if (auto s = jems::scope(sbr)) {
			body();
		}
		return sbr;
	};

	std::vector <Branch::Segment> segments;
	segments.reserve(1 + elifs.size());

	segments.push_back(Branch::Segment {
		ifs->cond,
		trace_body(ifs->body),
	});

	for (auto &elif : elifs) {
		segments.push_back(Branch::Segment {
			elif.cond,
			trace_body(elif.body),
		});
	}

	std::optional <SharedBlockReference> fallback;
	if (elses.has_value())
		fallback = trace_body(elses->body);

	Tracer::singleton.active().add(Instruction(Branch {
		std::move(segments),
		std::move(fallback),
	}, DebugInfo {}));
}

_branch_holder operator+(_branch_holder &&a, _elif b)
{
	auto result = std::move(a);
	result.elifs.push_back(b);
	return result;
}

_branch_holder operator+(_branch_holder &&a, _else b)
{
	auto result = std::move(a);
	result.elses = b;
	return result;
}

void _emit_switch_branch(
	std::vector <std::pair <Reference, _branch_body>> segments,
	std::optional <_branch_body> fallback
) {
	auto trace_body = [](const _branch_body &body) {
		auto sbr = std::make_shared <Block> ();
		if (auto s = jems::scope(sbr)) {
			body();
		}
		return sbr;
	};

	std::vector <Branch::Segment> out;
	out.reserve(segments.size());

	for (auto &seg : segments) {
		out.push_back(Branch::Segment {
			seg.first,
			trace_body(seg.second),
		});
	}

	std::optional <SharedBlockReference> fb;
	if (fallback.has_value())
		fb = trace_body(*fallback);

	Tracer::singleton.active().add(Instruction(Branch {
		std::move(out),
		std::move(fb),
	}, DebugInfo {}));
}

} // namespace rcgp
