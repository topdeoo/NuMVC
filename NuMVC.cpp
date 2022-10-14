#include "NuMVC.h"

ifstream is;

void init() {
	for (int i = 1; i <= v_num; i++) {
		v_in_c[i] = 0;
		dscore[i] = 0;
		age[i] = 0;
		conf_change[i] = 0;
		only_domain[i] = 0;
		weight[i] = 1;
		unobserved_v.insert(i);
	}

	for (int i = 0; i < e_num; i++) {

		//uncover_e.insert(i);

		is >> edge[i].v1 >> edge[i].v2;

		/* 初始时，顶点都不在支配集内，dscore的值为顶点度的大小 */
		dscore[edge[i].v1] += 1;
		dscore[edge[i].v2] += 1;
		/* 存入顶点的邻居 */
		v_neig[edge[i].v1].push_back(edge[i].v2);
		v_neig[edge[i].v2].push_back(edge[i].v1);

		/* 顶点所对应的边 */
		v_edges[edge[i].v1].push_back(i);
		v_edges[edge[i].v2].push_back(i);

	}

	/* 维护点权均值 */
	weight_mean =  1;

	/* 维护dscore的大根堆 */
	for (int i = 1; i <= v_num; i++) {
		dscore_vec.insert(i);
	}
}

int main(void) {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	// cin algorithm param
	is.open("data.txt");
	/*is.open("brock400-2.txt");*/

	is >> max_time >> optimal >> seeds;
	is >> v_num >> e_num;
	init_instance();

	init();
	greedy_search();

	if (C_vec.size() + unobserved_v.size() > optimal) {
		dscore_vec.clear();
		for (int i = 1; i < v_num; i++) {
			if (C_vec.find(i) == C_vec.end())
				dscore_vec.insert(i);
		}
		numvc();
	}

	cout << "MVC size is " << C_vec.size() << endl;
	//for (auto& i : C_vec)
	//	cout << "Vec: " << i << endl;

	return 0;
}

/*
10 2 5
4 4
1 2
1 3
2 3
1 4
*/