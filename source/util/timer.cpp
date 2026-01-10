#include <algorithm>
#include <list>
#include <memory>
#include <string>

#include <fmt/color.h>
#include <fmt/format.h>

#include "util/logging.hpp"
#include "util/timer.hpp"

struct TimerToken::Payload {
	std::string name;
	double milliseconds = 0.0;
	clock::time_point begin;
	std::list <std::string> notes;
	std::list <std::shared_ptr <Payload>> nested;

	struct ReportWidths {
		size_t label = 0;
		size_t ms = 0;
		size_t pct = 0;
	};

	static ReportWidths merge_widths(const ReportWidths &a, const ReportWidths &b);

	ReportWidths measure_widths(
		const std::string &prefix,
		bool is_last,
		bool has_parent,
		double parent = -1
	) const;

	std::string report_impl(
		const std::string &prefix,
		bool is_last,
		bool has_parent,
		const ReportWidths &widths,
		double parent = -1
	) const;

	void report() const;
};

thread_local TimerToken::clock TimerToken::clk;
thread_local std::stack <std::shared_ptr <TimerToken::Payload>> TimerToken::active;

TimerToken::Payload::ReportWidths TimerToken::Payload::merge_widths(
	const ReportWidths &a,
	const ReportWidths &b
) {
	return {
		std::max(a.label, b.label),
		std::max(a.ms, b.ms),
		std::max(a.pct, b.pct),
	};
}

TimerToken::Payload::ReportWidths TimerToken::Payload::measure_widths(
	const std::string &prefix,
	bool is_last,
	bool has_parent,
	double parent
) const {
	const std::string line_prefix = has_parent
		? prefix + (is_last ? "└─ " : "├─ ")
		: std::string();
	const std::string label = fmt::format("{}:", name);
	const std::string ms_str = fmt::format("{:.2f} ms", milliseconds);
	size_t pct_len = 0;
	if (parent > 0) {
		const std::string pct_str = fmt::format("({:.2f}%)", 100 * milliseconds / parent);
		pct_len = 1 + pct_str.size();
	}

	ReportWidths widths {
		line_prefix.size() + label.size(),
		ms_str.size(),
		pct_len,
	};

	std::string child_prefix = prefix;
	if (has_parent)
		child_prefix += is_last ? "   " : "│  ";

	const size_t nested_count = nested.size();
	size_t idx = 0;
	for (auto &payload : nested) {
		const bool last = (++idx == nested_count);
		widths = merge_widths(widths, payload->measure_widths(
			child_prefix, last, true, milliseconds
		));
	}

	return widths;
}

std::string TimerToken::Payload::report_impl(
	const std::string &prefix,
	bool is_last,
	bool has_parent,
	const ReportWidths &widths,
	double parent
) const {
	const std::string line_prefix = has_parent
		? prefix + (is_last ? "└─ " : "├─ ")
		: std::string();
	const std::string label = fmt::format("{}:", name);
	const std::string ms_str = fmt::format("{:.2f} ms", milliseconds);

	std::string result;
	result += line_prefix + label;
	const size_t label_pad = widths.label > line_prefix.size() + label.size()
		? widths.label - (line_prefix.size() + label.size())
		: 0;
	result += std::string(label_pad + 1, ' ');
	const size_t ms_pad = widths.ms > ms_str.size() ? widths.ms - ms_str.size() : 0;
	result += std::string(ms_pad, ' ');
	result += fmt::format(fmt::fg(fmt::color::gray), "{}", ms_str);

	if (parent > 0) {
		const std::string pct_str = fmt::format("({:.2f}%)", 100 * milliseconds / parent);
		const size_t pct_len = 1 + pct_str.size();
		const size_t pct_pad = widths.pct > pct_len ? widths.pct - pct_len : 0;
		result += std::string(pct_pad + 1, ' ');
		result += fmt::format(
			fmt::emphasis::italic | fmt::fg(fmt::color::gray),
			"{}", pct_str
		);
	} else if (widths.pct > 0) {
		result += std::string(widths.pct, ' ');
	}
	result += "\n";

	std::string child_prefix = prefix;
	if (has_parent)
		child_prefix += is_last ? "   " : "│  ";

	const size_t note_count = notes.size();
	const size_t nested_count = nested.size();
	const size_t total = note_count + nested_count;
	size_t idx = 0;

	for (auto &note : notes) {
		const bool last = (++idx == total);
		std::string label_note = fmt::format(
			fmt::emphasis::italic | fmt::fg(fmt::color::gray),
			"!{}", note
		);
		result += child_prefix + (last ? "└─ " : "├─ ") + label_note;
		result += "\n";
	}

	for (auto &payload : nested) {
		const bool last = (++idx == total);
		result += payload->report_impl(child_prefix, last, true, widths, milliseconds);
	}

	return result;
}

void TimerToken::Payload::report() const {
	auto widths = measure_widths("", true, false);
	info("scoped timer payload report:\n%s",
		report_impl("", true, false, widths).c_str());
}

TimerToken::TimerToken(const std::string &name) {
	begin(name);
}

void TimerToken::begin(const std::string &name) {
	payload = std::make_shared <Payload> ();
	payload->name = name;
	payload->begin = clk.now();

	if (not active.empty()) {
		auto &top = active.top();
		top->nested.emplace_back(payload);
	}

	active.push(payload);
}

void TimerToken::end() {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;

	auto us = duration_cast <microseconds> (
		clk.now() - payload->begin
	).count();

	payload->milliseconds = double(us) / 1000.0;

	assertion(active.top() == payload, "scoped timer invariant was broken");
	active.pop();

	if (active.empty())
		// TODO: callback would act here...
		payload->report();
}

void TimerToken::note(const std::string &note) {
	assertion(not active.empty(), "cannot add notes when there is no active scope timer");
	auto &top = active.top();
	top->notes.emplace_back(note);
}

void TimerToken::entry(const std::string &name, double ms) {
	assertion(not active.empty(), "cannot add entry when there is no active scope timer");

	auto entry_payload = std::make_shared <Payload> ();
	entry_payload->name = name;
	entry_payload->milliseconds = ms;

	auto &top = active.top();
	top->nested.emplace_back(entry_payload);
}
