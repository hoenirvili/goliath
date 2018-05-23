#pragma once

#include "api.hpp"

class Stack {

private:
	const ExecutionContext *ctx;
	
public:
	Stack(const ExecutionContext *ctx);
	~Stack() = default;

	void trace(char *out);

};