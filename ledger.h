#ifndef _H_ledger
#define _H_ledger

#include "currency.h"

#include <list>

struct Ledger {

	struct Transaction {
		Currency delta;
		std::string desc;
	};

	static inline Currency balance = 0;
	static inline std::list<Transaction> transactions;

	static void save(const char* name);
	static void load(const char* name);
	static void transact(Currency c, std::string desc);
};

#endif