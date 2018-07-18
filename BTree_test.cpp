#include "BTree.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
	vector<int> nums{ 12, 2, 6, 3, 8, 5, 10, 18, 11, 20, 30, 22, 26, 33, 66, 21, 56, 58, 16, 100 };
	BTree<int>* btree = new BTree<int>();
	int n = nums.size();
	for (int i = 0; i < n; i++) {
		btree->BTreeInsert(nums[i]);
	}
	btree->displayTree();

	if (btree->BTreeRemove(11)) {
		cout << "Remove success!" << endl;
	}
	else {
		cout << "Remove failed!" << endl;
	}

	btree->displayTree();

	return 0;
}