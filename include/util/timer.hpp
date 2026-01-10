#pragma once

#include <chrono>
#include <memory>
#include <stack>
#include <string>

#include <fmt/format.h>

struct TimerToken {
	using clock = std::chrono::system_clock;

	struct Payload;

	static thread_local clock clk;
	static thread_local std::stack <std::shared_ptr <Payload>> active;

	// TODO: callback in the case of not wanting to immediately dump it
	std::shared_ptr <Payload> payload;

	TimerToken() = default;
	explicit TimerToken(const std::string &name);

	void begin(const std::string &name);
	void end();

	static void note(const std::string &note);
	static void entry(const std::string &name, double ms);
};

struct ScopedTimerToken {
	TimerToken token;

	explicit ScopedTimerToken(const std::string &name) : token(name) {}
	~ScopedTimerToken() { token.end(); }
	ScopedTimerToken(const ScopedTimerToken &) = delete;
	ScopedTimerToken &operator=(const ScopedTimerToken &) = delete;
	ScopedTimerToken(ScopedTimerToken &&) = delete;
	ScopedTimerToken &operator=(ScopedTimerToken &&) = delete;
};

#define TSCOPE(str) ScopedTimerToken timer_token_##__LINE__(str)
#define TSTART(id, str) TimerToken id(str)
#define TEND(id) (id).end()
#define TNOTE(...) TimerToken::note(fmt::format(__VA_ARGS__))
#define TENTRY(str, t) TimerToken::entry(str, t)
