#include "knapsacksolver/algorithms/greedy.hpp"

using namespace knapsacksolver;

Output knapsacksolver::greedy(const Instance& instance, Info info)
{
    LOG_FOLD_START(info, "greedy" << std::endl);
    VER(info, "*** greedy ***" << std::endl);
    Output output(instance, info);

    assert(instance.break_item() != -1);

    output.update_sol(*instance.break_solution(), std::stringstream("break"), info);
    std::string best_algo = "break";
    LOG(info, "break " << output.solution.profit() << std::endl);
    ItemPos b = instance.break_item();

    if (b < instance.last_item()) {
        Profit  p = 0;
        ItemPos j = -1;

        // Backward greedy
        Weight rb = output.solution.remaining_capacity() - instance.item(b).w;
        for (ItemPos k=instance.first_item(); k<=b; ++k) {
            if (rb + instance.item(k).w >= 0 && instance.item(b).p - instance.item(k).p > p) {
                p = instance.item(b).p - instance.item(k).p;
                j = k;
            }
        }

        // Forward greedy
        Weight rf = output.solution.remaining_capacity();
        for (ItemPos k=b+1; k<=instance.last_item(); ++k) {
            if (instance.item(k).w <= rf && instance.item(k).p > p) {
                p = instance.item(k).p;
                j = k;
            }
        }

        if (j == -1) {
        } else if (j <= b) {
            best_algo = "backward";
            Solution sol = output.solution;
            sol.set(b, true);
            sol.set(j, false);
            output.update_sol(sol, std::stringstream("backward"), info);
        } else {
            best_algo = "forward";
            Solution sol = output.solution;
            sol.set(j, true);
            output.update_sol(sol, std::stringstream("forward"), info);
        }
    }

    LOG_FOLD_END(info, "greedy " << output.solution.profit());
    PUT(info, "Algorithm", "Best", best_algo);
    output.algorithm_end(info);
    return output;
}

