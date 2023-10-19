#include "main.h"

std::vector<std::vector<int>> optimal_stack_INTERNAL;
std::vector<int> method(int N, std::vector<std::vector<int>>& distance, int step, int start_from, bool enable_dumb_insert)
{
    if (distance.size() == 0) return {};
    std::vector<bool> X = std::vector<bool>(N, false); // Если мы уже входили в город, пишем 1
    std::vector<int> trace;
    int overall_distance = 0;
    int x = start_from;
	bool do_show_steps = (step == N);
    X[x] = true;
    trace.push_back(x);
	if (do_show_steps) std::cout << "1) Trace " << to_string(trace) << " Select " << x << "\n";

    for (int i = 1; i < N; i++) //общий шаг
    {
        int current_town = x;
        std::vector<int> current_min_pairs;
        for (int j = 0; j < i; j++) //Вычисление минимумов для каждого внутреннего элемента
        {
            int current_min_distance = __INT_MAX__;
            int current_min_town = -1;
			std::vector<int> output_neighbors_distance;
            for (int k = 0; k < N; k++)
            {
				if (!X[k]) output_neighbors_distance.push_back(distance[trace[j]][k]);
				else output_neighbors_distance.push_back(-1);
                if (!X[k] && distance[trace[j]][k] < current_min_distance)
                {
                    current_min_distance = distance[trace[j]][k];
                    current_min_town = k;
                }
            }
			if (do_show_steps) std::cout << trace[j] << " neighbors distance: " << to_string(output_neighbors_distance) << "\n";
            current_min_pairs.push_back(current_min_town);
        }
        if (do_show_steps) 
        {
            std::cout << "neighbors winners (from | to == distance):\n";
            for (int j = 0; j < i; j++)
            {
                std::cout << trace[j] << " | " << current_min_pairs[j] << " == " << distance[trace[j]][current_min_pairs[j]] <<"\n";
            }
        }
        int current_min_distance_in_pairs = __INT_MAX__;
        int current_min_to = -1;
        int current_min_from = -1;
        for (int j = 0; j < i; j++)
        {
            if (distance[trace[j]][current_min_pairs[j]] < current_min_distance_in_pairs)
            {
                current_min_from = j;
                current_min_to = current_min_pairs[j];
                current_min_distance_in_pairs = distance[trace[j]][current_min_pairs[j]];
            }
        }
		int show_current_min_from = trace[current_min_from];
		if (current_min_from == 0 && !enable_dumb_insert) current_min_from = -1; //обход, чтобы дать возможность вставлять в начало
        trace.insert(trace.begin() + current_min_from + 1, current_min_to);
        X[current_min_to] = true;
		overall_distance = 0;
		for (int j = 0; j < trace.size()-1; j++)
		{
			overall_distance += distance[trace[j]][trace[j+1]];
		}
        
        if (do_show_steps) 
		{
			print_step(i + 1, trace, show_current_min_from, current_min_to, overall_distance, current_min_distance_in_pairs);
		}
        if (step == i) return trace;
    }
    overall_distance += distance[trace[0]][trace[N-1]];

    std::cout << "Final: Trace " << to_string(trace) << " Distance " << overall_distance << "\n";
    return {}; 
    return trace;
}

// Код функций для вычисления точного оптимального значения
void print_optimal(std::vector<std::vector<int>>& distance)
{
    for (int i = 0; i < distance.size(); i++)
    {
        std::vector<bool> used;
        for (int j = 0; j < distance.size(); j++)
        {
            if (i == j) used.push_back(1);
            else used.push_back(0);
        }
        optimal_internal(used, { i }, 0, distance);
    }
}
bool is_optimal(int input, bool RESET)
{
    static int current_optimal = __INT_MAX__;
    if (RESET) 
    {
        current_optimal = __INT_MAX__;
        return 0;
    }
    if (input < current_optimal) 
    {
        current_optimal = input;
        return true;
    }
    return false;
}
void optimal_internal(std::vector<bool> used, std::vector<int> trace, int overall_distance, std::vector<std::vector<int>>& distance)
{
    bool is_done = true;
    for (int i = 0; i < distance.size(); i++)
    {
        if (used[i]) continue;
        std::vector<bool> new_used = used;
        new_used[i].flip();
        std::vector<int> new_trace = trace;
        new_trace.push_back(i);
        is_done = false;
        optimal_internal(new_used, new_trace, overall_distance + distance[trace[trace.size() - 1]][i], distance);
    }
    if (is_done && is_optimal(overall_distance + distance[trace[trace.size() - 1]][trace[0]]))
    {
        get_current_optimal_trace(trace);
    }
}
std::vector<int> get_current_optimal_trace(std::vector<int> trace_input)
{
    if (trace_input.size() != 0)
    {
        optimal_stack_INTERNAL.push_back(trace_input);
        return {};
    }
    else
    {
        if (optimal_stack_INTERNAL.size() == 0) return {};
        std::vector<int> output = optimal_stack_INTERNAL[0];
        optimal_stack_INTERNAL.erase(optimal_stack_INTERNAL.begin());
        return output;
    }
} 
// ...

// Разное
std::string to_string(std::vector<int> input)
{
    std::string output = "[ ";
    for (int i = 0; i < input.size(); i++)
    {
        output.append(std::to_string(input[i]) + " ");    
    }
    output.append("]");
    return output;
}
int randomnumber(int included_min, int included_max)
{
    static std::default_random_engine rng(time(NULL));
    std::uniform_int_distribution<int> dist(included_min, included_max); 
    dist(rng);
    return dist(rng); 
}
void print_step(int step, std::vector<int> trace, int selected_town_in, int selected_town_out, int overall_distance, int distance)
{
    std::cout << step << ") Trace " << to_string(trace) << " Select " << selected_town_in << "-->" << selected_town_out
    << " Distance " << distance << " Overall Distance " << overall_distance << "\n";
    return;
}
// ...