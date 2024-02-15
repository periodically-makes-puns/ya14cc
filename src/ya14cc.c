#include <stdio.h>
#include "qual.h"

int main() {
    while (true) {
        qual_state st;
        // printf("Enter IQ: ");
        // scanf("%hhu", &st.iq);
        // printf("Enter CP: ");
        // scanf("%hu", &st.cp);
        // printf("Enter Dur: ");
        // scanf("%hhu", &st.dur);
        // printf("Enter WN: ");
        // scanf("%hhu", &st.wn);
        // printf("Enter Manip: ");
        // scanf("%hhu", &st.manip);
        // printf("Enter Inno: ");
        // scanf("%hhu", &st.inno);
        // printf("Enter GS: ");
        // scanf("%hhu", &st.gs);
        // printf("Enter HaS: ");
        // scanf("%hhu", &st.has);
        st.iq = 0;
        st.cp = 600;
        st.dur = 14;
        st.wn = 0;
        st.manip = 0;
        st.inno = 0;
        st.gs = 0;
        st.t = 0;
        st.has = 0;
        u64 ust = from_qstate(&st);
        solve_qual(ust);
        dump_results(ust);
        return 0;
    }
}