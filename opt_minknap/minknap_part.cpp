#include "knapsack/opt_minknap/minknap.hpp"

#include "knapsack/lib/part_solution_2.hpp"
#include "knapsack/lb_greedy/greedy.hpp"
#include "knapsack/lb_greedynlogn/greedynlogn.hpp"
#include "knapsack/ub_dembo/dembo.hpp"
#include "knapsack/ub_dantzig/dantzig.hpp"
#include "knapsack/ub_surrogate/surrogate.hpp"

#include <bitset>

using namespace knapsack;

#define DBG(x)
//#define DBG(x) x

struct StatePart
{
    Weight w;
    Profit p;
    PartSol2 sol;
};

std::ostream& operator<<(std::ostream& os, const StatePart& s)
{
    os << "(" << s.w << " " << s.p << ")";
    return os;
}

struct MinknapData
{
    MinknapData(const Instance& ins): ins(ins) { }
    const Instance& ins;
    std::vector<StatePart> l0;
    ItemPos s;
    ItemPos t;
    Profit lb;
    StatePart best_state;
    Profit ub;
    PartSolFactory2 psolf;
};

void add_item(MinknapData& d)
{
    DBG(std::cout << "ADD ITEM... S " << s << " T " << t << " LB " << lb << std::endl;)
    d.psolf.add_item(d.t);
    d.best_state.sol = d.psolf.remove(d.best_state.sol);
    Weight c = d.ins.total_capacity();
    Weight wt = d.ins.item(d.t).w;
    Profit pt = d.ins.item(d.t).p;
    ItemPos tx = (d.ins.int_right_size() > 0 && d.t == d.ins.last_sorted_item())?
            d.ins.last_item()+1: d.t+1;
    ItemPos sx = (d.ins.int_left_size() > 0 && d.s == d.ins.first_sorted_item())?
            d.ins.first_item()-1: d.s;
    DBG(std::cout << "SX " << sx << " TX " << tx << std::endl;)
    std::vector<StatePart> l;
    std::vector<StatePart>::iterator it = d.l0.begin();
    std::vector<StatePart>::iterator it1 = d.l0.begin();
    while (it != d.l0.end() || it1 != d.l0.end()) {
        if (it == d.l0.end() || it->w > it1->w + wt) {
            StatePart s1{it1->w+wt, it1->p+pt, d.psolf.add(it1->sol)};
            DBG(std::cout << "STATE " << *it1 << " => " << s1 << std::flush;)
            if (l.empty() || s1.p > l.back().p) {
                if (s1.w <= c && s1.p > d.lb) { // Update lower bound
                    d.lb = s1.p;
                    d.best_state = s1;
                    if (d.lb == d.ub)
                        return;
                }
                if (!l.empty() && s1.w == l.back().w) {
                    l.back() = s1;
                    DBG(std::cout << " OK" << std::endl;)
                } else {
                    Profit ub = (s1.w <= c)?
                        ub_dembo(d.ins, tx, s1.p, c-s1.w):
                        ub_dembo_rev(d.ins, sx, s1.p, c-s1.w);
                    DBG(std::cout << " UB " << ub << " LB " << lb << std::flush;)
                    if (ub > d.lb) {
                        l.push_back(s1);
                        DBG(std::cout << " OK" << std::endl;)
                    } else {
                        DBG(std::cout << " X" << std::endl;)
                    }
                }
            } else {
                DBG(std::cout << " X" << std::endl;)
            }
            it1++;
        } else {
            assert(it != d.l0.end());
            DBG(std::cout << "STATE " << *it << std::flush;)
            it->sol = d.psolf.remove(it->sol);
            if (l.empty() || it->p > l.back().p) {
                if (!l.empty() && it->w == l.back().w) {
                    l.back() = *it;
                    DBG(std::cout << " OK" << std::endl;)
                } else {
                    Profit ub = (it->w <= c)?
                        ub_dembo(d.ins, tx, it->p, c-it->w):
                        ub_dembo_rev(d.ins, sx, it->p, c-it->w);
                    DBG(std::cout << " UB " << ub << " LB " << lb << std::flush;)
                    if (ub > d.lb) {
                        l.push_back(*it);
                        DBG(std::cout << " OK" << std::endl;)
                    } else {
                        DBG(std::cout << " X" << std::endl;)
                    }
                }
            } else {
                DBG(std::cout << " X" << std::endl;)
            }
            ++it;
        }
    }
    d.l0 = std::move(l);
    DBG(std::cout << "ADD ITEM... END" << std::endl;)
}

void remove_item(MinknapData& d)
{
    DBG(std::cout << "REMOVE ITEM... S " << s << " T " << t << " LB " << lb << std::endl;)
    d.psolf.add_item(d.s);
    d.best_state.sol = d.psolf.add(d.best_state.sol);
    Weight c = d.ins.total_capacity();
    Weight ws = d.ins.item(d.s).w;
    Profit ps = d.ins.item(d.s).p;
    ItemPos tx = (d.ins.int_right_size() > 0 && d.t == d.ins.last_sorted_item())?
            d.ins.last_item()+1: d.t;
    ItemPos sx = (d.ins.int_left_size() > 0 && d.s == d.ins.first_sorted_item())?
            d.ins.first_item()-1: d.s-1;
    DBG(std::cout << "SX " << sx << " TX " << tx << std::endl;)
    std::vector<StatePart> l;
    std::vector<StatePart>::iterator it = d.l0.begin();
    std::vector<StatePart>::iterator it1 = d.l0.begin();
    while (it != d.l0.end() || it1 != d.l0.end()) {
        if (it1 == d.l0.end() || it->w <= it1->w - ws) {
            DBG(std::cout << "STATE " << *it;)
            it->sol = d.psolf.add(it->sol);
            if (l.empty() || it->p > l.back().p) {
                if (!l.empty() && it->w == l.back().w) {
                    l.back() = *it;
                    DBG(std::cout << " OK" << std::endl;)
                } else {
                    Profit ub = (it->w <= c)?
                        ub_dembo(d.ins, tx, it->p, c-it->w):
                        ub_dembo_rev(d.ins, sx, it->p, c-it->w);
                    DBG(std::cout << " UB " << ub << " LB " << lb;)
                    if (ub > d.lb) {
                        l.push_back(*it);
                        DBG(std::cout << " OK" << std::endl;)
                    } else {
                        DBG(std::cout << " X" << std::endl;)
                    }
                }
            } else {
                DBG(std::cout << " X" << std::endl;)
            }
            ++it;
        } else {
            StatePart s1{it1->w-ws, it1->p-ps, d.psolf.remove(it1->sol)};
            DBG(std::cout << "STATE " << *it1 << " => " << s1;)
            if (l.empty() || s1.p > l.back().p) {
                if (s1.w <= c && s1.p > d.lb) { // Update lower bound
                    d.lb = s1.p;
                    d.best_state = s1;
                    if (d.lb == d.ub)
                        return;
                }
                if (!l.empty() && s1.w == l.back().w) {
                    l.back() = s1;
                    DBG(std::cout << " OK" << std::endl;)
                } else {
                    Profit ub = (s1.w <= c)?
                        ub_dembo(d.ins, tx, s1.p, c-s1.w):
                        ub_dembo_rev(d.ins, sx, s1.p, c-s1.w);
                    DBG(std::cout << " UB " << ub << " LB " << lb;)
                    if (ub > d.lb) {
                        l.push_back(s1);
                        DBG(std::cout << " OK" << std::endl;)
                    } else {
                        DBG(std::cout << " X" << std::endl;)
                    }
                }
            } else {
                DBG(std::cout << " X" << std::endl;)
            }
            it1++;
        }
    }
    d.l0 = std::move(l);
    DBG(std::cout << "REMOVE ITEM... END" << std::endl;)
}

#undef DBG

Solution knapsack::sopt_minknap_list_part(Instance& ins, Info& info,
        MinknapParams params, ItemPos k, Profit o)
{
    if (o == -1)
        info.verbose("*** minknap (list, part " + std::to_string(k) + ") ***\n");
    info.debug(
            "n " + std::to_string(ins.item_number()) + "/" + std::to_string(ins.total_item_number()) +
            " f " + std::to_string(ins.first_item()) +
            " l " + std::to_string(ins.last_item()) +
            " o " + std::to_string(o) +
            "\n");

    info.debug("Sort items...\n");
    ins.sort_partially();
    if (ins.break_item() == ins.last_item()+1) { // all items are in the break solution
        info.debug("All items fit in the knapsack.\n");
        Solution sol = *ins.break_solution();
        return algorithm_end(sol, info);
    }

    info.verbose("Compute lower bound...");
    Solution sol(ins);
    if (params.lb_greedynlogn == 0) {
        params.lb_greedynlogn = -1;
        Info info_tmp;
        sol = sol_bestgreedynlogn(ins, info_tmp);
    } else if (params.lb_greedy == 0) {
        params.lb_greedy = -1;
        Info info_tmp;
        sol = sol_greedy(ins, info_tmp);
    } else {
        sol = *ins.break_solution();
    }
    info.verbose(" " + std::to_string(sol.profit()) + "\n");

    Weight  c = ins.total_capacity();
    ItemPos n = ins.item_number();
    Profit lb = (o != -1 && o > sol.profit())? o-1: sol.profit();

    // Trivial cases
    if (n == 0 || c == 0) {
        info.debug("Empty instance.\n");
        if (ins.reduced_solution()->profit() > sol.profit())
            sol = *ins.reduced_solution();
        return algorithm_end(sol, info);
    } else if (n == 1) {
        info.debug("Instance only contains one item.\n");
        Solution sol1 = *ins.reduced_solution();
        sol1.set(ins.first_item(), true);
        return algorithm_end((sol1.profit() > sol.profit())? sol1: sol, info);
    } else if (ins.break_item() == ins.last_item()+1) {
        info.debug("All items fit in the knapsack.\n");
        if (ins.break_solution()->profit() > sol.profit())
            sol = *ins.break_solution();
        return algorithm_end(sol, info);
    }

    Weight w_bar = ins.break_solution()->weight();
    Profit p_bar = ins.break_solution()->profit();
    Info info_tmp;
    Profit u = (o != -1)? o: ub_dantzig(ins, info_tmp);
    if (sol.profit() == u) { // If UB == LB, then stop
        info.debug("Lower bound equals upper bound.");
        return algorithm_end(sol, info);
    }

    // Create memory table
    MinknapData d(ins);
    d.lb = lb;
    d.ub = u;
    d.psolf = PartSolFactory2(k);
    d.l0.push_back({w_bar, p_bar, 0});

    info.verbose("Recursion...\n");
    d.s = ins.break_item() - 1;
    d.t = ins.break_item();
    d.best_state = d.l0.front();
    StateIdx state_number = 1;
    while (!d.l0.empty()) {
        info.debug("List of states size " + std::to_string(d.l0.size()) + "\n");
        info.debug("f " + std::to_string(ins.first_item())
                + " s " + std::to_string(ins.first_sorted_item())
                + " t " + std::to_string(ins.last_sorted_item())
                + " l " + std::to_string(ins.last_item()) + "\n");
        if (ins.int_right_size() > 0 && d.t+1 > ins.last_sorted_item())
            ins.sort_right(lb);
        if (d.t <= ins.last_sorted_item()) {
            add_item(d);
            state_number += d.l0.size();
            ++d.t;
        }
        if (d.lb == d.ub)
            break;

        info.debug("List of states size " + std::to_string(d.l0.size()) + "\n");
        info.debug("f " + std::to_string(ins.first_item())
                + " s " + std::to_string(ins.first_sorted_item())
                + " t " + std::to_string(ins.last_sorted_item())
                + " l " + std::to_string(ins.last_item()) + "\n");
        if (ins.int_left_size() > 0 && d.s-1 < ins.first_sorted_item())
            ins.sort_left(lb);
        if (d.s >= ins.first_sorted_item()) {
            remove_item(d);
            state_number += d.l0.size();
            --d.s;
        }
        if (d.lb == d.ub)
            break;
    }

    info.verbose("State number: " + Info::to_string(state_number) + "\n");
    info.pt.put("Algorithm.StateNumber", state_number);

    if (d.best_state.p <= sol.profit())
        return algorithm_end(sol, info);
    assert(ins.check_opt(lb));

    ins.set_first_item(d.s+1);
    ins.set_last_item(d.t-1);
    info.debug("psol " + d.psolf.print(d.best_state.sol) + "\n");
    ins.fix(d.psolf, d.best_state.sol);

    Info info_tmp2;
    sol = knapsack::sopt_minknap_list_part(ins, info_tmp2, params, k, d.best_state.p);
    return algorithm_end(sol, info);
}

