#include <utility>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * B+Tree implementation. Note that the tree may not contain duplicate keys!
 *
 * Template parameters:
 * K: Key type.
 *   You may assume that keys can be compared with the < operator
 *   and are copy constructible and copy assignable.
 *   You may NOT use the == or the <= operator on keys, as these might be undefined for a key type!
 * V: Value type. You may assume that values are copy constructible and copy assignable.
 * NODE_SIZE: Number of key-value-pairs in leaves, number of keys in inner nodes.
 *    You may assume that S is >= 1
 *
 */
template<typename K, typename V, unsigned NODE_SIZE = 256>
class BPlusTree{
public:
	typedef K Key;
	typedef V Value;
	typedef BPlusTree<K,V,NODE_SIZE> ThisType;

	/**
	 * Copy assignment not allowed. This makes it easier for you!
	 */
	BPlusTree& operator=(const BPlusTree&other) = delete;

	/**
	 * Copy constructor. Make sure your class is correctly
	 * copy constructible!
	 */
	BPlusTree(const BPlusTree& other) : root(cloneSubtree(other.root)) {}

	/**
	 *	Initializes an empty tree
	 */
	BPlusTree() : root(nullptr) {}

	BPlusTree(BPlusTree&& other) : root(other.root) {
		other.root = nullptr;
	}

	~BPlusTree() {
		destroySubtree(root);
	}

	/**
	 *	Inserts a new key value pair <`key`,`value`> into the tree.
	 *	If the `key` already exists in the tree, then this operation
	 *	overwrites the old key value pair with the new one.
	 */
	void insert(Key key, Value value){
		if (root == nullptr) {
			root = makeLeaf();
			root->count = 1;
			root->keys[0] = key;
			root->values[0] = value;
			return;
		}

		Key sepKey;
		Node* newRight = nullptr;
		if (insertRec(root, key, value, sepKey, newRight)) {
			Node* newRoot = makeInner();
			newRoot->count = 1;
			newRoot->keys[0] = sepKey;
			newRoot->children[0] = root;
			newRoot->children[1] = newRight;
			root = newRoot;
		}
	}

	/**
	 * Returns whether the `key` is contained in the tree.
	 */
	bool hasKey(Key key){
		if (root == nullptr) {
			return false;
		}

		Node* node = root;
		while (!node->isLeaf) {
			unsigned i = childIndex(node->keys, node->count, key);
			node = node->children[i];
		}

		unsigned pos = lowerBound(node->keys, node->count, key);
		return pos < node->count && eq(node->keys[pos], key);
	}

	/**
	 * Finds the value associated to the `key`. If the `key`
	 * is not present in the tree, then a std::exception must
	 * be thrown.
	 */
	Value find(Key key){
		if (root == nullptr) {
			throw std::out_of_range("key not found");
		}

		Node* node = root;
		while (!node->isLeaf) {
			unsigned i = childIndex(node->keys, node->count, key);
			node = node->children[i];
		}

		unsigned pos = lowerBound(node->keys, node->count, key);
		if (pos < node->count && eq(node->keys[pos], key)) {
			return node->values[pos];
		}
		throw std::out_of_range("key not found");
	}

	/**
	 * Splits this tree at `key`, that is, all keys which are
	 * less than `key` are moved into a new tree T which is
	 * returned by the method. This tree retains all keys
	 * which are greater than or equal `key`. Note that splitting
	 * a tree at a key which is not present in the tree must
	 * be possible.
	 *
	 * Make sure that the implementation has a worst case complexity
	 * of O(log n), i.e., simply copying all keys which are less
	 * than `key` will not suffice, as this will be O(n)!
	 *
	 * Example:
	 * This tree contains the keys (2,4,6,8)
	 * splitting it at key 5 results in two trees, one
	 * with keys (2,4) which is returned and
	 * one with (6,8) which is this tree.
	 */
	ThisType split(Key key){
		if (root == nullptr) {
			return ThisType();
		}

		Node* leftRoot = splitRec(root, key);
		collapseRoot(leftRoot);
		collapseRoot(root);

		ThisType result;
		result.root = leftRoot;
		return result;
	}

private:
	struct Node {
		bool isLeaf;
		unsigned count;
		Key keys[NODE_SIZE + 1];
		union {
			Value values[NODE_SIZE + 1];
			Node* children[NODE_SIZE + 2];
		};
	};

	Node* root;

	static Node* makeLeaf() {
		Node* node = new Node();
		node->isLeaf = true;
		node->count = 0;
		return node;
	}

	static Node* makeInner() {
		Node* node = new Node();
		node->isLeaf = false;
		node->count = 0;
		return node;
	}

	static bool eq(const Key& a, const Key& b) {
		return !(a < b) && !(b < a);
	}

	static unsigned lowerBound(const Key* keys, unsigned count, const Key& key) {
		unsigned i = 0;
		while (i < count && keys[i] < key) {
			++i;
		}
		return i;
	}

	static unsigned childIndex(const Key* keys, unsigned count, const Key& key) {
		unsigned i = 0;
		while (i < count && !(key < keys[i])) {
			++i;
		}
		return i;
	}

	static void destroySubtree(Node* node) {
		if (node == nullptr) {
			return;
		}
		if (!node->isLeaf) {
			for (unsigned i = 0; i <= node->count; ++i) {
				destroySubtree(node->children[i]);
			}
		}
		delete node;
	}

	static Node* cloneSubtree(const Node* node) {
		if (node == nullptr) {
			return nullptr;
		}

		Node* copy = node->isLeaf ? makeLeaf() : makeInner();
		copy->count = node->count;
		for (unsigned i = 0; i < node->count; ++i) {
			copy->keys[i] = node->keys[i];
		}
		if (node->isLeaf) {
			for (unsigned i = 0; i < node->count; ++i) {
				copy->values[i] = node->values[i];
			}
		} else {
			for (unsigned i = 0; i <= node->count; ++i) {
				copy->children[i] = cloneSubtree(node->children[i]);
			}
		}
		return copy;
	}

	static void collapseRoot(Node*& node) {
		while (node != nullptr) {
			if (node->isLeaf) {
				if (node->count == 0) {
					delete node;
					node = nullptr;
				}
				return;
			}

			if (node->count == 0) {
				Node* child = node->children[0];
				delete node;
				node = child;
			} else {
				return;
			}
		}
	}

	static void splitLeaf(Node* leaf, Key& outSepKey, Node*& outNewRight) {
		unsigned total = leaf->count;
		unsigned leftSize = total / 2;
		Node* right = makeLeaf();
		right->count = total - leftSize;
		for (unsigned j = 0; j < right->count; ++j) {
			right->keys[j] = leaf->keys[leftSize + j];
			right->values[j] = leaf->values[leftSize + j];
		}
		leaf->count = leftSize;
		outSepKey = right->keys[0];
		outNewRight = right;
	}

	static void splitInner(Node* inner, Key& outSepKey, Node*& outNewRight) {
		unsigned total = inner->count;
		unsigned promoteIndex = total / 2;
		outSepKey = inner->keys[promoteIndex];

		Node* right = makeInner();
		right->count = total - promoteIndex - 1;
		for (unsigned j = 0; j < right->count; ++j) {
			right->keys[j] = inner->keys[promoteIndex + 1 + j];
		}
		for (unsigned j = 0; j <= right->count; ++j) {
			right->children[j] = inner->children[promoteIndex + 1 + j];
		}
		inner->count = promoteIndex;
		outNewRight = right;
	}

	static bool insertRec(Node* node, const Key& key, const Value& value, Key& outSepKey, Node*& outNewRight) {
		if (node->isLeaf) {
			unsigned pos = lowerBound(node->keys, node->count, key);
			if (pos < node->count && eq(node->keys[pos], key)) {
				node->values[pos] = value;
				return false;
			}

			for (unsigned j = node->count; j > pos; --j) {
				node->keys[j] = node->keys[j - 1];
				node->values[j] = node->values[j - 1];
			}
			node->keys[pos] = key;
			node->values[pos] = value;
			node->count++;

			if (node->count > NODE_SIZE) {
				splitLeaf(node, outSepKey, outNewRight);
				return true;
			}
			return false;
		}

		unsigned i = childIndex(node->keys, node->count, key);
		Key childSep;
		Node* childNewRight = nullptr;
		if (!insertRec(node->children[i], key, value, childSep, childNewRight)) {
			return false;
		}

		for (unsigned j = node->count; j > i; --j) {
			node->keys[j] = node->keys[j - 1];
		}
		node->keys[i] = childSep;
		for (unsigned j = node->count + 1; j > i + 1; --j) {
			node->children[j] = node->children[j - 1];
		}
		node->children[i + 1] = childNewRight;
		node->count++;

		if (node->count > NODE_SIZE) {
			splitInner(node, outSepKey, outNewRight);
			return true;
		}
		return false;
	}

	static Node* splitRec(Node* node, const Key& key) {
		if (node == nullptr) {
			return nullptr;
		}

		if (node->isLeaf) {
			unsigned i = lowerBound(node->keys, node->count, key);
			if (i == 0) {
				return nullptr;
			}

			Node* left = makeLeaf();
			left->count = i;
			for (unsigned j = 0; j < i; ++j) {
				left->keys[j] = node->keys[j];
				left->values[j] = node->values[j];
			}

			unsigned newCount = node->count - i;
			for (unsigned j = 0; j < newCount; ++j) {
				node->keys[j] = node->keys[i + j];
				node->values[j] = node->values[i + j];
			}
			node->count = newCount;
			return left;
		}

		unsigned i = childIndex(node->keys, node->count, key);

		Key savedKeys[NODE_SIZE + 1];
		Node* savedChildren[NODE_SIZE + 2];
		for (unsigned j = 0; j < i; ++j) {
			savedKeys[j] = node->keys[j];
			savedChildren[j] = node->children[j];
		}

		Node* leftPart = splitRec(node->children[i], key);

		unsigned rightCount = node->count - i;
		for (unsigned j = 0; j < rightCount; ++j) {
			node->keys[j] = node->keys[i + j];
		}
		for (unsigned j = 0; j <= rightCount; ++j) {
			node->children[j] = node->children[i + j];
		}
		node->count = rightCount;

		if (i == 0 && leftPart == nullptr) {
			return nullptr;
		}

		Node* left = makeInner();
		if (leftPart == nullptr) {
			left->count = i - 1;
			for (unsigned j = 0; j < left->count; ++j) {
				left->keys[j] = savedKeys[j];
			}
			for (unsigned j = 0; j < i; ++j) {
				left->children[j] = savedChildren[j];
			}
		} else {
			left->count = i;
			for (unsigned j = 0; j < i; ++j) {
				left->keys[j] = savedKeys[j];
			}
			for (unsigned j = 0; j < i; ++j) {
				left->children[j] = savedChildren[j];
			}
			left->children[i] = leftPart;
		}
		return left;
	}
};
//--------- END OF IMPLEMENTATION ----------


//--------- TEST CODE ------------
typedef BPlusTree<int,int,16> Tree;
bool failed = false;

void fail(std::string message){
	failed = true;
	std::cerr << message << std::endl;
}

void expectHasKey(Tree& tree, int key, bool expected, const std::string& label) {
	if (tree.hasKey(key) != expected) {
		fail(label);
	}
}

void expectFind(Tree& tree, int key, int expected, const std::string& label) {
	if (!tree.hasKey(key) || tree.find(key) != expected) {
		fail(label);
	}
}

void expectFindThrows(Tree& tree, int key, const std::string& label) {
	bool threw = false;
	try {
		tree.find(key);
	} catch (const std::exception&) {
		threw = true;
	}
	if (!threw) {
		fail(label);
	}
}

void expectKeys(Tree& tree, const std::vector<int>& keys, const std::string& label) {
	for (int key : keys) {
		if (!tree.hasKey(key)) {
			fail(label + " missing key " + std::to_string(key));
		}
	}
	for (int probe = keys.empty() ? 0 : keys.front() - 10; probe < (keys.empty() ? 1 : keys.back() + 10); ++probe) {
		bool shouldHave = false;
		for (int key : keys) {
			if (key == probe) {
				shouldHave = true;
				break;
			}
		}
		if (tree.hasKey(probe) != shouldHave) {
			fail(label + " unexpected membership for key " + std::to_string(probe));
		}
	}
}

void exampleTest(){
	Tree tree;
	tree.insert(5,4);
	if(tree.find(5) != 4) fail("Wrong value for key 5");
}

void testEmptyTree() {
	Tree tree;
	expectHasKey(tree, 1, false, "empty tree should not contain keys");
	expectFindThrows(tree, 1, "find on empty tree should throw");
}

void testOverwrite() {
	Tree tree;
	tree.insert(10, 100);
	tree.insert(10, 200);
	expectFind(tree, 10, 200, "overwrite should replace value");
	expectHasKey(tree, 5, false, "overwrite should not add extra keys");
}

void testFirstSplit() {
	Tree tree;
	for (int i = 0; i < 16; ++i) {
		tree.insert(i, i * 10);
	}
	if (tree.find(0) != 0) fail("before split key 0");
	tree.insert(16, 160);
	if (tree.find(0) != 0) fail("after split key 0");
	if (tree.find(8) != 80) fail("after split key 8");
	if (tree.find(16) != 160) fail("after split key 16");
}

void testBulkInsert() {
	Tree tree;
	for (int i = 0; i < 100; ++i) {
		tree.insert(i, i * 10);
	}
	for (int i = 0; i < 100; ++i) {
		expectFind(tree, i, i * 10, "bulk ascending insert");
	}
	expectHasKey(tree, 100, false, "missing key after bulk insert");

	Tree descending;
	for (int i = 99; i >= 0; --i) {
		descending.insert(i, i + 1);
	}
	for (int i = 0; i < 100; ++i) {
		expectFind(descending, i, i + 1, "bulk descending insert");
	}
}

void testLeafSplit() {
	Tree tree;
	for (int i = 0; i < 17; ++i) {
		tree.insert(i, i);
	}
	for (int i = 0; i < 17; ++i) {
		expectFind(tree, i, i, "leaf split should preserve entries");
	}
}

void testDeepTree() {
	Tree tree;
	for (int i = 0; i < 500; ++i) {
		tree.insert(i, i * 2);
	}
	for (int i = 0; i < 500; ++i) {
		expectFind(tree, i, i * 2, "deep tree lookup");
	}
}

void testCopyConstructor() {
	Tree original;
	for (int i = 0; i < 50; ++i) {
		original.insert(i, i + 5);
	}
	Tree copy(original);
	copy.insert(999, 999);
	original.insert(1000, 1000);

	expectHasKey(copy, 999, true, "copy should contain its own inserts");
	expectHasKey(copy, 1000, false, "copy should not see original-only inserts");
	expectHasKey(original, 1000, true, "original should contain its own inserts");
	expectHasKey(original, 999, false, "original should not see copy-only inserts");
	for (int i = 0; i < 50; ++i) {
		expectFind(copy, i, i + 5, "copy should preserve copied data");
		expectFind(original, i, i + 5, "original should preserve copied data");
	}
}

Tree makeExampleTree() {
	Tree tree;
	tree.insert(2, 2);
	tree.insert(4, 4);
	tree.insert(6, 6);
	tree.insert(8, 8);
	return tree;
}

void testSplitExamples() {
	Tree tree = makeExampleTree();
	Tree left = tree.split(5);
	expectKeys(left, {2, 4}, "split at 5 left side");
	expectKeys(tree, {6, 8}, "split at 5 right side");

	Tree allLeft = makeExampleTree();
	Tree emptyRight = allLeft.split(9);
	expectKeys(emptyRight, {2, 4, 6, 8}, "split above max moves all keys left");
	expectKeys(allLeft, {}, "split above max leaves right empty");

	Tree emptyLeft = makeExampleTree();
	Tree allRight = emptyLeft.split(1);
	expectKeys(allRight, {}, "split below min leaves left empty");
	expectKeys(emptyLeft, {2, 4, 6, 8}, "split below min keeps all keys right");

	Tree atKey = makeExampleTree();
	Tree leftAt4 = atKey.split(4);
	expectKeys(leftAt4, {2}, "split at existing key 4 left");
	expectKeys(atKey, {4, 6, 8}, "split at existing key 4 right");
}

void testSplitLargeTree() {
	Tree tree;
	for (int i = 0; i < 200; ++i) {
		tree.insert(i, i);
	}

	Tree left = tree.split(100);
	for (int i = 0; i < 100; ++i) {
		expectHasKey(left, i, true, "large split left membership");
		expectHasKey(tree, i, false, "large split right should not keep smaller keys");
	}
	for (int i = 100; i < 200; ++i) {
		expectHasKey(left, i, false, "large split left should not keep larger keys");
		expectHasKey(tree, i, true, "large split right membership");
		expectFind(tree, i, i, "large split right values");
	}
}

struct CustomKey {
	int value;
	CustomKey() : value(0) {}
	explicit CustomKey(int v) : value(v) {}
	bool operator<(const CustomKey& other) const {
		return value < other.value;
	}
};

void testCustomKeyType() {
	BPlusTree<CustomKey, int, 4> tree;
	for (int i = 0; i < 20; ++i) {
		tree.insert(CustomKey(i), i * 3);
	}
	for (int i = 0; i < 20; ++i) {
		if (!tree.hasKey(CustomKey(i)) || tree.find(CustomKey(i)) != i * 3) {
			fail("custom key type lookup failed");
		}
	}
}

void testNodeSizeOne() {
	BPlusTree<int, int, 1> tree;
	for (int i = 0; i < 30; ++i) {
		tree.insert(i, i + 7);
	}
	for (int i = 0; i < 30; ++i) {
		if (!tree.hasKey(i) || tree.find(i) != i + 7) {
			fail("NODE_SIZE=1 lookup failed");
		}
	}

	BPlusTree<int, int, 1> left = tree.split(15);
	for (int i = 0; i < 15; ++i) {
		if (!left.hasKey(i)) {
			fail("NODE_SIZE=1 split left failed");
		}
	}
	for (int i = 15; i < 30; ++i) {
		if (!tree.hasKey(i)) {
			fail("NODE_SIZE=1 split right failed");
		}
	}
}

void testMemoryChurn() {
	for (int t = 0; t < 100; ++t) {
		Tree tree;
		for (int i = 0; i < 200; ++i) {
			tree.insert(i, i);
		}
		Tree copy(tree);
		Tree left = tree.split(100);
		(void)copy;
		(void)left;
	}
}

void yourTests(){
	exampleTest();
	testEmptyTree();
	testOverwrite();
	testBulkInsert();
	testLeafSplit();
	testDeepTree();
	testCopyConstructor();
	testSplitExamples();
	testSplitLargeTree();
	testCustomKeyType();
	testNodeSizeOne();
	testMemoryChurn();
}


int main(){
	try {
		yourTests();
	} catch (...) {
		std::cerr << "=============== EXCEPTION THROWN! Implementation is buggy! ===============" << std::endl;
	}
	if(failed){
		std::cerr << "=============== TESTS FAILED! Implementation is buggy! ===============" << std::endl;
	} else {
		std::cout << "=============== TESTS PASSED! ===============" << std::endl;
	}
}
