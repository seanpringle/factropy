#include "common.h"
#include "ledger.h"

void Ledger::transact(Currency c, std::string d) {
	transactions.push_front({
		delta: c,
		desc: d,
	});
	balance += c;
}
