#ifndef BTREE_H_
#define BTREE_H_

#include <iostream>
#include <limits.h>

using namespace std;

template<class T>
class BTree {
private:
	static const int t = 3;						// B树的最小度数
	static const int key_min = t - 1;			// 结点包含关键字的最小个数
	static const int key_max = 2 * t - 1;		// 结点包含关键字的最大个数
	static const int child_min = key_min + 1;	// 孩子结点的最小个数
	static const int child_max = key_max + 1;	// 孩子结点的最大个数

	struct BNode {
		int keyNum;			// 结点包含的关键字的数量
		bool isLeaf;		// 该结点是否是叶子结点
		T keyValue[key_max];		// 结点中关键字数组
		BNode* Child[child_max];	// 结点中指向其孩子的指针数组

		BNode(bool flag = true, int n = 0) : isLeaf(flag), keyNum(n){ }
	};

	BNode* pRoot;		// B树的根节点

public:
	BTree() {
		pRoot = NULL;	// 创建一颗空的B树
	}

	~BTree() {
		clearBTree();
	}

	void BTreeInsert(const T& key) {
		if (Contain(key)) {
			cout << "This key had been Inserted!" << endl;
			return;
		}

		if (pRoot == NULL) {
			pRoot = new BNode();
		}
		if (pRoot->keyNum == key_max) {
			BNode* pNode = new BNode();
			pNode->isLeaf = false;
			pNode->Child[0] = pRoot;
			BTreeSplitChild(pNode, 0, pRoot);
			pRoot = pNode;		// 更新根节点指针
		}
		BTreeInsertNonFull(pRoot, key);
		//return true;
	}

	bool Contain(const T& key)const {	// 检查key是否包含在树中
		return BTreeSearch(pRoot, key);
	}

	// B树中删除关键字key
	bool BTreeRemove(const T& key) {
		if (!BTreeSearch(pRoot, key)) {
			cout << "B Tree does not contain this key" << endl;
			return false;
		}

		if (pRoot->keyNum == 1) {
			if (pRoot->isLeaf) {
				clearBTree();
				return true;
			}
			else {
				BNode* pChildLeftNode = pRoot->Child[0];
				BNode* pChildRightNode = pRoot->Child[1];
				if (pChildLeftNode->keyNum == key_min && pChildRightNode->keyNum == key_min) {
					BTreeMergeChild(pRoot, 0);
					deleteNode(pRoot);
					pRoot = pChildLeftNode;
				}
			}
		}

		recursive_removeKey(pRoot, key);
		return true;
	}

	void BTree_InOrder() {
		B_InOrder(pRoot);
		cout << endl;
	}

	void displayTree()const {
		displayNode(pRoot);
		cout << endl;
	}

	void clearBTree() {
		recursive_clearBTree(pRoot);
		pRoot = NULL;
	}

private:
	/*
	** 搜索在以pNode为根节点包括pNode结点的树中的关键字key
	*/
	bool BTreeSearch(BNode* pNode, const T& key)const {
		if (pNode == NULL) {
			return false;
		}

		int i = 0;
		while (i<pNode->keyNum && key>pNode->keyValue[i]) {
			i++;
		}

		if (i < pNode->keyNum && key == pNode->keyValue[i]) {
			return true;
		}
		else {
			if (pNode->isLeaf) {
				return false;
			}
			else {
				return BTreeSearch(pNode->Child[i], key);
			}
		}
	}

	/*
	** 分裂一个满结点
	** pChild: 将要被分裂的结点		pParent: 作为pChild结点的父结点，用来存储pCHild结点中提升（分裂）出的中间关键字
	** childIndex: 指针数组中下标为childIndex的指针元素指向的子树，即 pParent->Child[childIndex] = pChild
	*/
	void BTreeSplitChild(BNode* pParent, int childIndex, BNode* pChild) {
		BNode* pNode = new BNode();			// 用于存储pChild结点分裂后的右半部分，作为右结点
		pNode->isLeaf = pChild->isLeaf;
		pNode->keyNum = key_min;

		for (int i = 0; i < key_min; i++) {		// 将pChild结点中的右半部分的关键字拷贝到pNode中
			pNode->keyValue[i] = pChild->keyValue[i + child_min];
		}
		if (!pChild->isLeaf) {					// 如果pChild是非叶节点，则将pChild结点中的指向孩子结点的指针
			for (int i = 0; i < child_min; i++) {
				pNode->Child[i] = pChild->Child[i + child_min];
			}
		}

		pChild->keyNum = key_min;	// 更新pChild结点中的关键字数量

		// 将pParent结点中的指针数组和关键字数组中childIndex后的元素后移一位
		for (int i = pParent->keyNum; i > childIndex; i--) {
			pParent->keyValue[i] = pParent->keyValue[i - 1];
			pParent->Child[i + 1] = pParent->Child[i];
		}

		pParent->Child[childIndex + 1] = pNode;		// 存储新分裂出的右结点的指针，即将childIndex后一位指针指向新分裂出的右结点
		pParent->keyValue[childIndex] = pChild->keyValue[key_min];	// 把pChild结点中的中间关键字提到父节点中
		pParent->keyNum += 1;	// 更新父节点中的关键字数量
	}

	/*
	** 将关键字key插入非满的叶节点中
	** 结点pNode如果不是叶节点，则必须将key插入以内部结点pNode为根的子树中适当的叶节点中去；如果是叶节点，则直接插入该结点中
	*/
	void BTreeInsertNonFull(BNode* pNode, const T& key) {
		/*
		int i = pNode->keyNum - 1;
		if (pNode->isLeaf) {
			while (i >= 0 && key < pNode->keyValue[i]) {
				pNode->keyValue[i + 1] = pNode->keyValue[i];
				i--;
		}
		i += 1;
		}
		*/
		int i = pNode->keyNum;
		if (pNode->isLeaf) {	// pNode是叶节点的话，直接插入关键字
			while (i > 0 && key < pNode->keyValue[i - 1]) {			// 从后往前，查询关键字插入位置
				pNode->keyValue[i] = pNode->keyValue[i - 1];
				i--;
			}
			pNode->keyValue[i] = key;		// 插入关键字
			pNode->keyNum++;				// 更新pNode结点的关键字数量
		}
		else {			// pNode是内部结点
			while (i > 0 && key < pNode->keyValue[i - 1]) {			// 从后往前，查询关键字要插入的子树
				i--;
			}
			BNode* pChild = pNode->Child[i];
			if (pChild->keyNum == key_max) {		// 如果key要插入的子树结点中关键字的数量已满，则分裂该结点
				BTreeSplitChild(pNode, i, pChild);
				if (key > pNode->keyValue[i]) { // 确定key在分裂的两个子结点中哪一个下降子树是正确的
					//i += 1;
					pChild = pNode->Child[i + 1];
				}
			}
			BTreeInsertNonFull(pChild, key);		// 递归地将key插入合适的子树中
		}
	}

	/*
	** 合并两个子结点
	** 将pParent->Child[index+1]结点合并到pParent->Child[index]结点中
	*/
	void BTreeMergeChild(BNode* pParent, int index) {
		BNode* pChild1 = pParent->Child[index];
		BNode* pChild2 = pParent->Child[index + 1];

		pChild1->keyNum = key_max;
		pChild1->keyValue[key_min] = pParent->keyValue[index];	// 将父节点中index指向的值下移至pChild1结点中

		for (int i = 0; i < key_min; i++) {		// 将pChild2结点中的关键字移到pChild1结点中
			pChild1->keyValue[i + key_min + 1] = pChild2->keyValue[i];
		}
		if (!pChild1->isLeaf) {					// 将pChild2结点中的孩子指针数组移到pChild1结点中
			for (int i = 0; i < child_min; i++) {
				pChild1->Child[i + child_min] = pChild2->Child[i];
			}
		}

		// 删除父节点中的下移的关键字，将关键字数组中index后面的元素依次向前移动一位，将指针数组中index+1后面的元素向前移动一位
		pParent->keyNum--;
		for (int i = index; i < pParent->keyNum; i++) {
			pParent->keyValue[i] = pParent->keyValue[i + 1];
			pParent->Child[i + 1] = pParent->Child[i + 2];
		}
		deleteNode(pChild2);
	}

	/*
	** 递归删除关键字，并做修复
	*/
	void recursive_removeKey(BNode* pNode, const T& key) {
		int i = 0;
		while (i < pNode->keyNum && key > pNode->keyValue[i])
			i++;

		if (i <= pNode->keyNum && key == pNode->keyValue[i]) {	// 关键字在pNode结点中
			if (pNode->isLeaf) {		// pNode是叶节点
				// 从pNode结点中删除关键字
				--pNode->keyNum;
				for (; i < pNode->keyNum; i++) {
					pNode->keyValue[i] = pNode->keyValue[i + 1];
				}
				return;
			}
			else {		// pNode是内结点
				BNode* pChildLeft = pNode->Child[i];
				BNode* pChildRight = pNode->Child[i + 1];
				
				if (pChildLeft->keyNum >= child_min) {
					//T preKey = getPredecessorKey(pNode);
					T preKey = getPredecessorKey_1(pChildLeft);
					recursive_removeKey(pChildLeft, preKey);
					pNode->keyValue[i] = preKey;
					return;
				}
				else if (pChildRight->keyNum >= child_min) {
					//T succKey = getPredecessorKey(pNode);
					T succKey = getSuccessorKey_1(pChildRight);
					recursive_removeKey(pChildRight, succKey);
					pNode->keyValue[i] = succKey;
					return;
				}
			}
		}
		else {		// 关键字不在pNode结点中
			BNode* pChildNode = pNode->Child[i];
			/*
			** 递归删除pNode结点子树的关键字时主要需要考虑到孩子结点中关键字的数量，如果数量为key_min时需要进行调整
			*/
			if (pChildNode->keyNum == key_min) {
				BNode* pLeft = i > 0 ? pNode->Child[i - 1] : NULL;		// pChildNode结点的左边结点
				BNode* pRight = i < pNode->keyNum ? pNode->Child[i + 1] : NULL;	// pChildNode结点的右边结点
				int j;
				if (pLeft && pLeft->keyNum >= child_min) {
					for (j = pChildNode->keyNum; j > 0; j--) {
						pChildNode->keyValue[j] = pChildNode->keyValue[j - 1];
					}
					pChildNode->keyValue[0] = pNode->keyValue[i - 1];	// 将pNode结点中i-1指向的关键字下移至pChildNode结点中

					for (j = pChildNode->keyNum + 1; j > 0; j--) {
						pChildNode->Child[j] = pChildNode->Child[j - 1];
					}
					if (!pLeft->isLeaf) {
						pChildNode->Child[0] = pLeft->Child[pLeft->keyNum];	// 将pLeft结点中的最后一个孩子指针移至pChildNode结点中的第一个处
					}
					pChildNode->keyNum++;
					pNode->keyValue[i - 1] = pLeft->keyValue[pLeft->keyNum - 1]; // 将pLeft结点中的最后一个关键字上移至pNode结点中i的前面
					pLeft->keyNum--;
				}
				else if (pRight && pRight->keyNum >= child_min) {
					// 将pNode结点中i处的关键字下移至pChildNode结点中的最后一个位置处
					pChildNode->keyValue[pChildNode->keyNum] = pNode->keyValue[i];
					pChildNode->keyNum++;
					pNode->keyValue[i] = pRight->keyValue[0];
					pRight->keyNum--;
					
					for (j = 0; j < pRight->keyNum - 1; j++) {	// 将pRight结点中的关键字后移一位
						pRight->keyValue[j] = pRight->keyValue[j + 1];
					}
					if (!pRight->isLeaf) {
						// 将pRight结点中的第一个孩子指针移至pChildNode结点的最后一处，然后将pRight结点中的孩子指针往前移一位
						pChildNode->Child[pChildNode->keyNum] = pRight->Child[0];
						for (j = 0; j < pRight->keyNum; j++) {
							pRight->Child[j] = pRight->Child[j + 1];
						}
					}
				}
				// pLeft和pRight结点中关键字的数量都只为key_min
				else if (pLeft) {
					BTreeMergeChild(pNode, i - 1);	// 合并pLeft和pChildNode两个结点
					pChildNode = pLeft;
				}
				else if (pRight) {
					BTreeMergeChild(pNode, i);	// 合并pChildNode和pRight两个结点
				}
			}
			recursive_removeKey(pChildNode, key);
		}
	}

	// 删除B树
	void recursive_clearBTree(BNode* pNode) {
		if (pNode != NULL) {
			if (!pNode->isLeaf) {
				for (int i = 0; i <= pNode->keyNum; i++)
					recursive_clearBTree(pNode->Child[i]);
			}
			deleteNode(pNode);
		}
	}

	/*
	** 找到B树中某个结点的前驱关键字
	** 结点的前驱就是小于该结点的关键字中最大的关键字
	** pNode结点的前驱关键字
	*/
	T getPredecessorKey(BNode* pNode) {
		if (pNode == NULL) {
			cout << "结点为空！" << endl;
			return INT_MIN;
		}

		if (!pNode->isLeaf) {
			pNode = pNode->Child[0];
		}

		while (!pNode->isLeaf) {
			pNode = pNode->Child[pNode->keyNum];
		}
		return pNode->keyValue[pNode->keyNum - 1];
	}

	/*
	** 若pParent->Child[i] = pNode，则找到pParent->keyValue[i]关键字的前驱关键字
	** 找到pNode结点的父节点的前驱关键字
	*/
	T getPredecessorKey_1(BNode* pNode) {
		if (pNode == NULL) {
			cout << "结点为空！" << endl;
			return INT_MIN;
		}

		while (!pNode->isLeaf) {
			pNode = pNode->Child[pNode->keyNum];
		}
		return pNode->keyValue[pNode->keyNum - 1];
	}

	/*
	** 找到某个结点的后继关键字
	** 结点的后继就是大于该结点的关键字中最小的关键字
	** pNode结点的后继关键字
	*/
	T getSuccessorKey(BNode* pNode) {
		if (pNode == NULL) {
			cout << "结点为空！" << endl;
			return INT_MIN;
		}

		if (!pNode->isLeaf) {
			pNode = pNode->Child[pNode->keyNum];
		}
		while (!pNode->isLeaf) {
			pNode = pNode->Child[0];
		}
		
		return pNode->keyValue[0];
	}

	/*
	** 找到相应父节点中某个关键字的后继关键字
	*/
	T getSuccessorKey_1(BNode* pNode) {
		if (pNode == NULL) {
			cout << "结点为空！" << endl;
			return INT_MIN;
		}

		while (!pNode->isLeaf) {
			pNode = pNode->Child[0];
		}
		return pNode->keyValue[0];
	}

	// 删除结点
	void deleteNode(BNode* pNode) {
		if (pNode != NULL) {
			delete pNode;
			pNode = NULL;
		}
	}

	// 中序遍历B树，输出每个结点中的关键字信息
	void B_InOrder(BNode* pNode) {
		if (pNode == NULL) {
			return;
		}

		int i;
		for (i = 0; i < pNode->keyNum; i++) {
			if (!pNode->isLeaf) {
				B_InOrder(pNode->Child[i]);
			}
			cout << pNode->keyValue[i] << " ";
		}

		if (!pNode->isLeaf) {
			B_InOrder(pNode->Child[i]);
		}
	}

	void displayNode(BNode* pNode)const {
		if (pNode != NULL) {
			int i;
			for (i = 0; i < pNode->keyNum; i++) {
				if (!pNode->isLeaf) {
					displayNode(pNode->Child[i]);
				}
				cout << pNode->keyValue[i] << " ";
			}

			if (!pNode->isLeaf) {
				displayNode(pNode->Child[i]);
			}
		}
	}
};

#endif