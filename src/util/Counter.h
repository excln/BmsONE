#ifndef COUNTER_H
#define COUNTER_H


class Counter
{
	int value;

public:
	Counter() : value(0) { }

	int GetValue() const { return value; }
	operator bool() const { return value != 0; }

	void operator ++() { value++; }
	void operator --() { value--; }
	void operator ++(int) { value++; }
	void operator --(int) { value--; }
};


class CounterScope
{
	Counter &c;

public:
	CounterScope(Counter &c) : c(c) { c++; }
	~CounterScope() { c--; }
};


#endif // COUNTER_H
