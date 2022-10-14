#pragma once
#include "basic.h"

void init_instance() {
	c_size = 0;
	best_c_size = 0;
	edge = new Edge[e_num];
	dscore = new int[v_num + 1];
	weight = new int[v_num + 1];
	conf_change = new int[v_num + 1];
	only_domain = new int[v_num + 1];
	vis = new int[v_num + 1]();
	age = new uint64[v_num + 1];
	memset(age, 0, sizeof age);
	v_in_c = new int[v_num + 1];
	best_v_in_c = new int[v_num + 1];
	weight_mean = 0;
	gama = v_num;
}

void update_sol() {
	while (!C_vec_dscore.empty())C_vec_dscore.clear();
	for (auto& i : C_vec)
		C_vec_dscore.emplace(i);
}

void observed(int v) {
	unobserved_v.erase(v);
}

void unobserved(int v) {
	unobserved_v.insert(v);
}

int randomSelect() {
	srand(unsigned(time(NULL)));
	return rand() % unobserved_v.size();
}

// dfs version(false)
//void prop(int v) {
//
//	vis[v] = 1;
//
//	if (v_edges[v].size() >= 2 && unobserved_v.find(v) != unobserved_v.end()) {
//		int d = 0, deg = v_edges[v].size();
//		for (int i = 0; i < deg; i++) {
//			int n = v_neig[v][i];
//			if (unobserved_v.find(n) == unobserved_v.end())
//				d++;
//		}
//		if (d == deg - 1)
//			observed(v);
//	}
//
//	for (int i = 0; i < v_edges[v].size(); i++) {
//		int n = v_neig[v][i];
//		if (!vis[n]) {
//			prop(n);
//		}
//	}
//}

// bfs + loop limit version
void prop(int v) {
	openlist.push(v);
	vis[v]++;
	while (!openlist.empty()) {
		int u = openlist.front();
		openlist.pop();
		for (int i = 0; i < v_edges[u].size(); i++) {
			int n = v_neig[u][i];
			if (unobserved_v.find(n) == unobserved_v.end() && vis[n] < v_edges[n].size()) {
				vis[n] = v_edges[n].size();
				openlist.push(n);
			}
			else if (vis[n] < v_edges[n].size()) {
				openlist.push(n);
				vis[n] += 1;
			}
		}
		if (v_edges[u].size() >= 2 && unobserved_v.find(u) != unobserved_v.end()) {
			int d = 0, deg = v_edges[u].size();
			for (int i = 0; i < deg; i++) {
				int n = v_neig[u][i];
				if (unobserved_v.find(n) == unobserved_v.end())
					d++;
			}
			if (d >= deg - 1)
				observed(u);
		}
		for (int i = 0; i < v_edges[u].size(); i++) {
			int n = v_neig[u][i];
			if (v_edges[n].size() == 1 && unobserved_v.find(u) == unobserved_v.end())
				observed(n);
		}
	}
}

void remove(int v) {
	C_vec.erase(v);
	dscore[v] = 0;
	conf_change[v] = 0;

	//for (int i = 0; i < v_edges[v].size(); i++) {
	//	int e = v_edges[v][i], n = v_neig[v][i];
	//	if (C_vec.find(n) == C_vec.end()) {
	//		dscore[n] += weight[e];
	//		conf_change[n] = 1;
	//		uncover(e);
	//	}
	//	else
	//		dscore[n] -= weight[e];
	//}

	int flag = 0;
	for (int i = 0; i < v_edges[v].size(); i++) {
		int n = v_neig[v][i];
		if (C_vec.find(n) != C_vec.end()) {
			flag = 1;
			break;
		}
		if (only_domain[n])
			unobserved(n);
		conf_change[n] = 1;
	}

	if (!flag)
		unobserved(v);
	

	memset(vis, 0, sizeof vis);
	prop(*(C_vec.begin()));

	for (int i = 0; i < v_edges[v].size(); i++) {
		int n = v_neig[v][i];
		if (unobserved_v.find(n) != unobserved_v.end())
			dscore[v] += weight[n];
		if (C_vec.find(n) == C_vec.end()) {
			dscore[n] += (!flag) * weight[v];
		}
	}

	dscore_vec.insert(v);

}

void add(int v) {
	C_vec.insert(v);
	dscore[v] = 0;
	observed(v);
	//for (int i = 0; i < v_edges[v].size(); i++) {
	//	int e = v_edges[v][i], n = v_neig[v][i];
	//	if (C_vec.find(n) == C_vec.end()) {
	//		dscore[n] -= weight[e];
	//		conf_change[n] = 1;
	//		observed(e);
	//	}
	//	else
	//		dscore[n] += weight[e];
	//}

	for (int i = 0; i < v_edges[v].size(); i++) {
		int n = v_neig[v][i];
		conf_change[n] = 1;
		observed(n);
		if (C_vec.find(n) == C_vec.end()) { // n is not in C
			dscore[n] -= weight[v];
		}
		if (only_domain[n])
			dscore[v] -= weight[n];
	}

	memset(vis, 0, sizeof vis);
	prop(v);
	C_vec_dscore.insert(v);
	dscore_vec.erase(v);
}

void forget_weight() {
	for (int i = 0; i < e_num; i++) {
		weight[i] = floor(roi * weight[i]);
		int v1 = C_vec.find(edge[i].v1) != C_vec.end();
		int v2 = C_vec.find(edge[i].v2) != C_vec.end();
		if (v1 + v2 == 0) {
			dscore[edge[i].v1] += weight[i];
			dscore[edge[i].v2] += weight[i];
		}
		else if (v1 + v2 == 1) {
			if (v1)dscore[edge[i].v1] -= weight[i];
			else dscore[edge[i].v2] -= weight[i];
		}
	}
}

void update_weight() {
	weight_mean = 0;
	for (int i = 0; i < e_num; i++) {
		weight[i] += 1;
		dscore[edge[i].v1] += 1;
		dscore[edge[i].v2] += 1;
		weight_mean += weight[i];
	}
	weight_mean /= 1.0*e_num;
	if (weight_mean >= gama)
		forget_weight();
}

int select_vec() {
	//Edge e = edge[idx];
	//if (!conf_change[e.v1])
	//	return e.v2;
	//else if (!conf_change[e.v2])
	//	return e.v1;
	//else {
	//	if (dscore[e.v1] > dscore[e.v2] || (dscore[e.v1] == dscore[e.v2] && age[e.v1] > age[e.v2]))
	//		return e.v1;
	//	else
	//		return e.v2;
	//}
	int maxn = -1, v = -1;
	for (auto& i : dscore_vec) {
		if (dscore[i] < 0)
			continue;
		if (maxn < dscore[i] && conf_change[i]) {
			maxn = dscore[i];
			v = i;
		}
		else if (maxn == dscore[i] && age[i] > age[v]) {
			v = i;
		}
	}
	return v == -1 ? (*dscore_vec.begin()) : v;

}

void greedy_search() {
	//int i = 0;
	//while(!uncover_e.empty()) {
	//	int best_v = *(dscore_vec.begin());
	//	if (dscore[best_v] > 0) {
	//		dscore_vec.erase(dscore_vec.begin());
	//		C_vec.insert(best_v);
	//		dscore[best_v] = -dscore[best_v];
	//		for (int j = 0; j < v_edges[best_v].size(); j++) {
	//			int e = v_edges[best_v][j], n = v_neig[best_v][j];
	//			if (C_vec.find(n) == C_vec.end()) {
	//				dscore[n] -= weight[e];
	//				conf_change[n] = 1;
	//				cover(e);
	//			}
	//			else
	//				dscore[n] += weight[e];
	//		}
	//	}
	//}

	/* Greedy Algorithm Search */

	while (!unobserved_v.empty()) {
		int best_v = *(dscore_vec.begin());
		if (dscore[best_v] >= 0) {
			dscore_vec.erase(dscore_vec.begin());
			// Check if vec is observed
			if (unobserved_v.find(best_v) == unobserved_v.end())
				continue;
			C_vec.insert(best_v);
			observed(best_v);
			dscore[best_v] = -dscore[best_v];

			for (int j = 0; j < v_edges[best_v].size(); j++) {
				int n = v_neig[best_v][j];
				if (unobserved_v.find(n) != unobserved_v.end()) { // n is unobserved
					dscore[n] -= weight[best_v];
					conf_change[n] = 1;
					only_domain[n] = 1;
					observed(n);
				}
				else {
					if (only_domain[n])
						only_domain[n] = 0;
					dscore[n] -= weight[best_v];
				}
			}

		}
	}
	cout << "Greedy: " << C_vec.size() << endl;
	update_sol();
}

int find_max_dscore() {
	int v = *(C_vec_dscore.begin());
	C_vec_dscore.erase(C_vec_dscore.begin());
	return v;
}


void numvc() {
	step = 1;
	while (step <= max_time) {
		if (unobserved_v.empty()) {
			update_sol();
			if (C_vec.size() == optimal)
				return;
			int v = find_max_dscore();
			remove(v);
			continue;
		}
		int v = find_max_dscore();
		remove(v);
		//auto it = unobserved_v.begin();
		//advance(it, randomSelect());
		//int idx = *it;
		//int v_add = select_vec(idx);
		int v_add = select_vec();
		add(v_add);
		age[v_add]++, age[v]++;
		tabu = v_add;
		update_weight();
		step++;
	}
}
