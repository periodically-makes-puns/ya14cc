#pragma once

#include "types.h"

u64 from_qstate(const qual_state* st);
void load_qstate(qual_state* st, u64 nst);

u64 solve_qual(u64 state);

void print_state(u64 state);
void dump_results(u64 state);
