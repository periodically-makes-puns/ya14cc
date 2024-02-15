#include "qual.h"

#ifndef max
    #define max(a,b) ((a >= b) ? (a) : (b))
#endif


#ifndef min
    #define min(a,b) ((a <= b) ? (a) : (b))
#endif

#define   QUAL(x) (((x) & 0xFFFF000000000000) >> 48)
#define ACTION(x) (((x) & 0x0000FF0000000000) >> 40)
#define NEXTST(x) ((x) & 0x000000FFFFFFFFFF)

#include <Judy.h>

void* cache = NULL;

const char* actions[] = {"(finished)", "",
    "Basic Touch", "Standard Touch", "Advanced Touch", "Basic+Standard", "Advanced Combo", 
    "Focused Touch", "Prudent Touch", "Preparatory Touch", "Trained Finesse", "Waste Not I", 
    "Waste Not II", "Manipulation", "Master's Mend", "Innovation", "Great Strides", 
    "Observe", "Byregot's", "Precise Touch"};

u64 from_qstate(const qual_state* st) {
    return (st->cp) + ((u64) st->iq << 10) + ((u64) st->dur << 14) + 
        ((u64) st->manip << 18) + ((u64) st->wn << 22) + ((u64) st->inno << 26) +
        ((u64) st->gs << 29) + ((u64) st->t << 32) + ((u64) st->has << 40);
}

void load_qstate(qual_state* st, u64 nst) {
    st->cp =    nst & 0x000003FF;
    st->iq =    (nst & 0x00003C00) >> 10;
    st->dur =   (nst & 0x0003C000) >> 14;
    st->manip = (nst & 0x003C0000) >> 18;
    st->wn =    (nst & 0x03C00000) >> 22;
    st->inno =  (nst & 0x1C000000) >> 26;
    st->gs =    (nst & 0x60000000) >> 29;
    st->has =   (nst & 0x80000000) >> 31;
    st->t =   (nst & 0xFF00000000) >> 32;
}

#define check_time(t, dt) (((t) == 0) || ((t) > (dt)))
#define dec_time(t, dt) ((t == 0) ? 0 : (t - dt))

u64 cons_result(u64 qual, u8 ind, u64 nst) {
    return ((u64) qual << 48) + ((u64) ind << 40) + nst;
}

#define QUAL_PER_100 400.0

u16 calc_qual(u8 iq, u8 inno, u8 gs, u8 sz) {
    return (10 + iq) * (2 + (inno ? 1 : 0) + (gs ? 2 : 0)) * sz;
}

u64 solve_qual(u64 state) {
    // Time = 0 means don't check time.
    // Time <= 2 is an ending condition.
    // Dur = 0 is NOT an ending criterion, since manip may let it tick up.
    // CP <= 17 IS an ending condition, since all actions cost CP.
    PWord_t val;
    JLG(val, cache, state);
    if (val != NULL) return *val;
    qual_state st;
    qual_state nst;
    load_qstate(&st, state);
    if (st.cp < 18) return 0;
    u64 result = 0, res = 0, nstu = 0;
    #define TICK_EFFECTS(d, s, amt) d.manip = max(s.manip, amt) - amt, d.wn = max(s.wn, amt) - amt, d.inno = max(s.inno, amt) - amt, d.gs = max(s.gs, amt) - amt;
    if ((st.cp >= 25)
        && check_time(st.t, 6) 
        &&(st.dur + (st.wn > 1) + (st.manip > 0) >= 2)) 
    {
        // Focused Touch
        nst.cp = st.cp - 25;
        nst.iq = min(st.iq + 1, 10);
        nst.dur = st.dur + (st.wn > 1) + min(st.manip, 2) - 2;
        TICK_EFFECTS(nst, st, 2)
        nst.gs = 0;
        nst.has = st.has;
        nst.t = dec_time(st.t, 6);
        nstu = from_qstate(&nst);
        res = solve_qual(nstu);
        res = QUAL(res) + calc_qual(st.iq, max(st.inno, 1) - 1, max(st.gs, 1) - 1, 30);
        res = cons_result(res, 7, nstu);
        result = max(res, result);
    }
    if ((st.cp >= 25)
        && (st.dur >= 1)
        && (!st.wn)
        && check_time(st.t, 3) ) 
    {
        // Prudent Touch
        nst.cp = st.cp - 25;
        nst.iq = min(st.iq + 1, 10);
        nst.dur = st.dur + min(st.manip, 1) - 1;
        TICK_EFFECTS(nst, st, 1)
        nst.gs = 0;
        nst.has = st.has;
        nst.t = dec_time(st.t, 3);
        nstu = from_qstate(&nst);
        res = solve_qual(nstu);
        res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20);
        res = cons_result(res, 8, nstu);
        result = max(res, result);
    }
    if ((st.cp >= 32)
        && (st.iq == 10)
        && check_time(st.t, 3) ) 
    {
        // Trained Finesse
        nst.cp = st.cp - 32;
        nst.iq = 10;
        nst.dur = min(st.dur + min(st.manip, 1), 14);
        TICK_EFFECTS(nst, st, 1)
        nst.gs = 0;
        nst.has = st.has;
        nst.t = dec_time(st.t, 3);
        nstu = from_qstate(&nst);
        res = solve_qual(nstu);
        res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20);
        res = cons_result(res, 10, nstu);
        result = max(res, result);
    }
    if (st.dur + (st.wn > 0) >= 2 && check_time(st.t, 3)) {
        // Horrifying code.
        if (st.cp >= 18) {
            // Basic Touch
            nst.cp = st.cp - 18;
            nst.iq = min(st.iq + 1, 10);
            nst.dur = st.dur + (st.wn > 0) + min(st.manip, 1) - 2;
            TICK_EFFECTS(nst, st, 1)
            nst.gs = 0;
            nst.has = st.has;
            nst.t = dec_time(st.t, 3);
            nstu = from_qstate(&nst);
            res = solve_qual(nstu);
            res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20);
            res = cons_result(res, 2, nstu);
            result = max(res, result);
            if (st.has && check_time(st.t, 6)) {
                // Precise Touch
                nst.cp = st.cp - 18;
                nst.iq = min(st.iq + 2, 10);
                nst.dur = st.dur + (st.wn > 0) + min(st.manip, 1) - 2;
                TICK_EFFECTS(nst, st, 1)
                nst.has = 0;
                nst.t = dec_time(st.t, 3);
                nstu = from_qstate(&nst);
                res = solve_qual(nstu);
                res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 30);
                res = cons_result(res, 19, nstu);
                result = max(res, result);
            }
            if (st.cp >= 24) {
                // Byregot's
                nst.cp = st.cp - 24;
                nst.iq = 0;
                nst.dur = st.dur + (st.wn > 0) + min(st.manip, 1) - 2;
                TICK_EFFECTS(nst, st, 1)
                nst.gs = 0;
                nst.has = st.has;
                nst.t = dec_time(st.t, 3);
                nstu = from_qstate(&nst);
                res = solve_qual(nstu);
                res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20 + 4 * min(st.iq, 10));
                res = cons_result(res, 18, nstu);
                result = max(res, result);
                if (st.cp >= 32) {
                    // Standard Touch
                    nst.cp = st.cp - 32;
                    nst.iq = min(st.iq + 1, 10);
                    nst.dur = st.dur + (st.wn > 0) + min(st.manip, 1) - 2;
                    TICK_EFFECTS(nst, st, 1)
                nst.gs = 0;
                    nst.has = st.has;
                    nst.t = dec_time(st.t, 3);
                    nstu = from_qstate(&nst);
                    res = solve_qual(nstu);
                    res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 25);
                    res = cons_result(res, 3, nstu);
                    result = max(res, result);
                    if (st.cp >= 36 && st.dur + min(st.wn, 2) + (st.manip > 0) >= 4 && check_time(st.t, 6)) {
                        // Std combo
                        nst.cp = st.cp - 36;
                        nst.iq = min(st.iq + 2, 10);
                        nst.dur = st.dur + min(st.wn, 2) + min(st.manip, 2) - 4;
                        TICK_EFFECTS(nst, st, 2)
                        nst.gs = 0;
                        nst.has = st.has;
                        nst.t = dec_time(st.t, 6);
                        nstu = from_qstate(&nst);
                        res = solve_qual(nstu);
                        res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20) + calc_qual(min(st.iq + 1, 10), max(st.inno, 1) - 1, 0, 25);
                        res = cons_result(res, 5, nstu);
                        result = max(res, result);
                        if (st.cp >= 54 && st.dur + min(st.wn, 3) + min(st.manip, 2) >= 6 && check_time(st.t, 9)) {
                            // Adv combo
                            nst.cp = st.cp - 54;
                            nst.iq = min(st.iq + 3, 10);
                            nst.dur = st.dur + min(st.wn, 3) + min(st.manip, 3) - 6;
                            TICK_EFFECTS(nst, st, 3)
                            nst.gs = 0;
                            nst.has = st.has;
                            nst.t = dec_time(st.t, 9);
                            nstu = from_qstate(&nst);
                            res = solve_qual(nstu);
                            res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 20) 
                                + calc_qual(min(st.iq + 1, 10), max(st.inno, 1) - 1, 0, 25)
                                + calc_qual(min(st.iq + 2, 10), max(st.inno, 2) - 2, 0, 30);
                            res = cons_result(res, 6, nstu);
                            result = max(res, result);
                        }
                    }
                    if (st.cp >= 46) {
                        // Adv. Touch
                        nst.cp = st.cp - 46;
                        nst.iq = min(st.iq + 1, 10);
                        nst.dur = st.dur + (st.wn > 0) + min(st.manip, 1) - 2;
                        TICK_EFFECTS(nst, st, 1)
                        nst.gs = 0;
                        nst.has = st.has;
                        nst.t = dec_time(st.t, 3);
                        nstu = from_qstate(&nst);
                        res = solve_qual(nstu);
                        res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 30);
                        res = cons_result(res, 4, nstu);
                        result = max(res, result);
                    }
                }
            }
        }
        if ((st.dur + ((st.wn) ? 2 : 0) >= 4) && (st.cp >= 40)) {
            // Prep
            nst.cp = st.cp - 40;
            nst.iq = min(st.iq + 2, 10);
            nst.dur = st.dur + ((st.wn) ? 2 : 0) + (st.manip > 0) - 4;
            TICK_EFFECTS(nst, st, 1)
            nst.gs = 0;
            nst.has = st.has;
            nst.t = dec_time(st.t, 3);
            nstu = from_qstate(&nst);
            res = solve_qual(nstu);
            res = QUAL(res) + calc_qual(st.iq, st.inno, st.gs, 40);
            res = cons_result(res, 9, nstu);
            result = max(res, result);
        }
    }
    if (st.cp >= 18 && check_time(st.t, 2)) {
        // Inno
        nst.cp = st.cp - 18;
        nst.iq = st.iq;
        nst.dur = min(st.dur + min(st.manip, 1), 14);
        TICK_EFFECTS(nst, st, 1)
        nst.inno = 4;
        nst.has = st.has;
        nst.t = dec_time(st.t, 2);
        nstu = from_qstate(&nst);
        res = solve_qual(nstu);
        res = cons_result(QUAL(res), 15, nstu);
        result = max(res, result);
        if (st.cp >= 32) {
            // GS
            nst.cp = st.cp - 32;
            nst.iq = st.iq;
            nst.dur = min(st.dur + min(st.manip, 1), 14);
            TICK_EFFECTS(nst, st, 1)
            nst.gs = 3;
            nst.has = st.has;
            nst.t = dec_time(st.t, 2);
            nstu = from_qstate(&nst);
            res = solve_qual(nstu);
            res = cons_result(QUAL(res), 16, nstu);
            result = max(res, result);
            if (st.cp >= 56) {
                // WN1
                nst.cp = st.cp - 56;
                nst.iq = st.iq;
                nst.dur = min(st.dur + min(st.manip, 1), 14);
                TICK_EFFECTS(nst, st, 1)
                nst.wn = 4;
                nst.has = st.has;
                nst.t = dec_time(st.t, 2);
                nstu = from_qstate(&nst);
                res = solve_qual(nstu);
                res = cons_result(QUAL(res), 11, nstu);
                result = max(res, result);
                if (st.cp >= 88) {
                    // MM
                    nst.cp = st.cp - 88;
                    nst.iq = st.iq;
                    nst.dur = min(st.dur + min(st.manip, 1) + 6, 14);
                    TICK_EFFECTS(nst, st, 1)
                    nst.has = st.has;
                    nst.t = dec_time(st.t, 2);
                    nstu = from_qstate(&nst);
                    res = solve_qual(nstu);
                    res = cons_result(QUAL(res), 14, nstu);
                    result = max(res, result);
                    if (st.cp >= 96) {
                        // Manip
                        nst.cp = st.cp - 96;
                        nst.iq = st.iq;
                        nst.dur = st.dur;
                        TICK_EFFECTS(nst, st, 1)
                        nst.manip = 8;
                        nst.has = st.has;
                        nst.t = dec_time(st.t, 2);
                        nstu = from_qstate(&nst);
                        res = solve_qual(nstu);
                        res = cons_result(QUAL(res), 13, nstu);
                        result = max(res, result);
                        if (st.cp >= 98) {
                            // WN2
                            nst.cp = st.cp - 98;
                            nst.iq = st.iq;
                            nst.dur = min(st.dur + min(st.manip, 1), 14);
                            TICK_EFFECTS(nst, st, 1)
                            nst.wn = 8;
                            nst.has = st.has;
                            nst.t = dec_time(st.t, 2);
                            nstu = from_qstate(&nst);
                            res = solve_qual(nstu);
                            res = cons_result(QUAL(res), 12, nstu);
                            result = max(res, result);
                        }
                    }
                }
            }
        }
    }
    JLI(val, cache, state);
    if (val == PJERR) {
        fputs("malloc error!!", stderr);
        exit(1);
    }
    *val = result;
    return result;
}

void print_state(u64 state) {
    qual_state st;
    load_qstate(&st, state);
    printf("IQ: %2hhu CP: %4hu D: %2hhu WN: %hhu M: %hhu I: %hhu G: %hhu H: %hhu\n", 
        st.iq, st.cp, st.dur * 5, st.wn, st.manip, st.inno, st.gs, st.has);
}

void dump_results(u64 state) {
    PWord_t val;
    JLG(val, cache, state);
    if (val == NULL) {
        puts("state not found");
    }
    u64 next_state = NEXTST(*val);
    u8 action = ACTION(*val);
    u16 qual, pq = QUAL(*val);
    while (action != 0) {
        printf("%2d %-20s ", action, actions[action]);
        JLG(val, cache, next_state);
        if (val != NULL) {
            next_state = NEXTST(*val);
            action = ACTION(*val);
            qual = QUAL(*val);
        } else {
            next_state = 0;
            action = 0;
            qual = 0;
        }
        printf("%7.4f ", (pq - qual) / QUAL_PER_100);
        pq = qual;
        print_state(state);
        state = next_state;
    }
}