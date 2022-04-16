// Copyright 2018 Nesterov Alexander
#include <vector>
#include <string>
#include <random>
#include <functional>
#include <numeric>
#include <iostream>
#include "../../../modules/task_3/kulemin_p_discharge_double_sort_tbb/kulemin_p_discharge_double_sort_tbb.h"

vector* create_random_vector(int size_n) {
    std::random_device dev;
    std::mt19937 gen(dev());
    vector* in = new vector(size_n);
    for (int i = 0; i < size_n; ++i) {
        in->ptr[i] = static_cast<double>(gen()) / gen();
    }
    return in;
}

struct DischargeSort {
    vector* arr;
    size_t byte;
    std::vector<std::list<double>> lists;
    explicit DischargeSort(vector* ta, size_t _b): arr(ta), byte(_b) {
        lists.resize(256);
    }
    DischargeSort(DischargeSort&s, tbb::split) : arr(s.arr), byte(s.byte) {
        lists.resize(256);
    }
    void operator()(const tbb::blocked_range<int>& r) {
        int end = r.end();
        int begin = r.begin();
        for (int i = begin; i < end; i++) {
            unsigned char* pt = (unsigned char*)(arr->ptr + i);
            lists[*(pt + byte)].push_back(arr->ptr[i]);
        }
    }
    void join(const DischargeSort& s) {
        for (int i = 0; i < 256; i++) {
			if (!s.lists[i].empty()) {
				for (std::list<double>::const_iterator it = s.lists[i].begin(); it != s.lists[i].end(); it++) {
					lists[i].push_back(*it);
				}
			}
        }
    }
};
void discharge_sort(vector* v) {
    for (size_t j = 0; j < sizeof(double); j++) {
        v->last_el = 0;
        DischargeSort s(v, j);
        tbb::parallel_reduce(tbb::blocked_range<int>(0, v->size, v->size / 4), s);
		union_from_lists_seq(&s.lists, v->ptr);
    }
}
void union_from_lists_seq(std::vector<std::list<double>> *lists, double* in) {
	int j = 0;
	for (size_t i = 0; i < (*lists).size(); i++) {
		while (!(*lists)[i].empty()) {
			in[j] = ((*lists)[i].front());
			(*lists)[i].pop_front();
			j++;
		}
	}
}
void discharge_sort_seq(vector* in) {
	std::vector<std::list<double >> lists(256);
	for (size_t j = 0; j < sizeof(double); j++) {
		for (int i = 0; i < in->size; i++) {
			unsigned char* pt = (unsigned char*)(in->ptr + i);
			lists[*(pt + j)].push_back(in->ptr[i]);
		}
		union_from_lists_seq(&lists, in->ptr);
	}
}

bool check_vectors(double* st, double* sd, int size) {
    bool res = true;
    for (int i = 0; i < size; i++) {
        if (st[i] != sd[i]) {
            res = false;
            break;
        }
    }
    return res;
}
void copy_vectors(double* st, double* sd, int size) {
    for (int i = 0; i < size; i++) {
        sd[i] = st[i];
    }
}
