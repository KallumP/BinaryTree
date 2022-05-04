#include "raylib.h"
#include <iostream>
#include <string>

int screenWidth = 1280;
int screenHeight = 720;
bool balance = true;
bool debug = true;

int toAddMantisa;
int toAddExponent;
int toAdd;

Vector2 mousePos;

Color background = CLITERAL(Color) { 10, 24, 32, 255 };
Color nodeBackground = CLITERAL(Color) { 207, 107, 221, 255 };
Color fontColor = WHITE;
Color hoverNode = CLITERAL(Color) { 227, 0, 43, 255 };

struct Node {

	int value;
	Node* lChild;
	Node* rChild;
	Node* parent;

	bool rebalance;
	int lDepth;
	int rDepth;
	int balanceFactor;

	bool hovered;

	//sets the value of this node
	void Set(Node* _parent, int _value) {
		value = _value;
		balanceFactor = 0;

		parent = _parent;

		if (debug)
			std::cout << "Inserted value: " << value << "\n";
	}

	//tries to insert a value into this node
	int Insert(int _value, int height) {

		int insertHeight = height + 1;

		if (_value == value) {

			std::cout << "Didn't insert duplicate value: " << value << "\n";
			return -1;


			//checks if this value was smaller or equal
		} else if (_value < value) {

			//checks if the left node was not occupied
			if (lChild == nullptr) {

				//creates the new left node and sets its value
				lChild = new Node();
				lChild->Set(this, _value);
			} else {

				//inserts into the left node
				insertHeight = lChild->Insert(_value, insertHeight);
			}

		} else {

			//checks if the right node was not occupied
			if (rChild == nullptr) {

				//creates the new right node and sets its value
				rChild = new Node();
				rChild->Set(this, _value);
			} else {

				//inserts into the right node
				insertHeight = rChild->Insert(_value, insertHeight);
			}
		}

		return insertHeight;
	}

	//Calculates the depth of this balance factor of this node, returns this node's depth
	int CalculateBalanceFactor() {

		//gets the depth of the left side
		if (lChild == nullptr) {
			lDepth = 1;
		} else {

			lChild->CalculateBalanceFactor();
			lDepth = 1 + lChild->BiggestDepth();
		}

		//gets the depth of the right side
		if (rChild == nullptr) {
			rDepth = 1;
		} else {
			rChild->CalculateBalanceFactor();
			rDepth = 1 + rChild->BiggestDepth();
		}

		balanceFactor = rDepth - lDepth;

		rebalance = balanceFactor < -1 || balanceFactor > 1;


		return BiggestDepth();
	}

	//returns the biggest depth from either the left or right side
	int BiggestDepth() {
		if (lDepth > rDepth)
			return lDepth;
		return rDepth;
	}
};

class Tree {

public:
	Node* root;
	Node* hoveredNode;

	std::string valuesString;

	int totalHeight;
	int nodeRadius;



	//constructor
	Tree() {
		root = new Node();
		hoveredNode = nullptr;
		totalHeight = 0;
		nodeRadius = 15;
		valuesString = "";
	}

	//inserts a key into the root
	void InsertRoot(int value) {
		root->Set(nullptr, value);
	}

	//inserts a value into the tree
	void Insert(int value) {

		int _height = root->Insert(value, 1);

		if (totalHeight < _height)
			totalHeight = _height;

		if (balance) {

			AttemptRebalance();
		}
	}

	//draws this node, and then calls the draw on its children nodes
	void Draw(Node* node, int level, int position) {

		if (node != nullptr) {

			//the position to draw this node
			Vector2 nodePosition = GetPosition(level, position);

			//the string of the node's value
			std::string valueString = std::to_string(node->value);
			int fontSize = 4;
			Vector2 stringPos;
			stringPos.x = nodePosition.x - (valueString.size() * fontSize * 1.5) / 2;
			stringPos.y = nodePosition.y - (fontSize * 1.5) / 2;

			CheckHovered(node, nodePosition);

			//draws the node
			if (node == hoveredNode)
				DrawCircleLines(nodePosition.x, nodePosition.y, nodeRadius, hoverNode);
			else
				DrawCircleLines(nodePosition.x, nodePosition.y, nodeRadius, nodeBackground);

			//draws the node value
			DrawText(valueString.c_str(), stringPos.x, stringPos.y, fontSize, nodeBackground);

			if (debug)
				if (node->parent != nullptr)
					DrawText(std::to_string(node->parent->value).c_str(), stringPos.x, stringPos.y - nodeRadius * 2, fontSize, nodeBackground);

			//draw the lNode subtree
			if (node->lChild != nullptr) {

				Vector2 nextNodePosition = GetPosition(level + 1, position * 2);

				//draws the line between this node and the next
				DrawLine(nodePosition.x, nodePosition.y + nodeRadius, nextNodePosition.x, nextNodePosition.y - nodeRadius, fontColor);

				//draws the next node
				Draw(node->lChild, level + 1, position * 2);
			}

			//draw the rNode subtree
			if (node->rChild != nullptr) {

				Vector2 nextNodePosition = GetPosition(level + 1, position * 2 + 1);

				//draws the line between this node and the next
				DrawLine(nodePosition.x, nodePosition.y + nodeRadius, nextNodePosition.x, nextNodePosition.y - nodeRadius, fontColor);

				//draws the next node
				Draw(node->rChild, level + 1, position * 2 + 1);
			}
		}
	}

	//returns a string of all the values in the tree (comma separated)
	std::string Traverse(Node* node) {

		if (node->lChild)
			Traverse(node->lChild) + ",";

		valuesString += std::to_string(node->value) + ",";

		if (node->rChild)
			Traverse(node->rChild) + ",";

		return valuesString;
	}

	//returns the position of a node based on its level and position in that level
	Vector2 GetPosition(int level, int position) {

		//number of possible nodes in this level
		int totalNodesInThisLevel = (int)std::pow(2, level);

		//the gaps between nodes
		Vector2 gaps;
		gaps.x = (float)screenWidth / (totalNodesInThisLevel + 1);

		if (totalHeight != 0)
			gaps.y = (screenHeight - nodeRadius * 2) / totalHeight;
		else
			gaps.y = (screenHeight - nodeRadius * 2) / 1;

		//the position to draw this node
		Vector2 pos = Vector2();
		pos.x = gaps.x * (position + 1);
		pos.y = (level + 1) * gaps.y;

		return pos;
	}

	int GetHeight() {
		if (root != nullptr)
			return root->BiggestDepth();
	}

	bool endRebalanceRecursion;
	void AttemptRebalance() {
		CalculateBalances();
		endRebalanceRecursion = false;
		Rebalance(root);

		if (endRebalanceRecursion)
			AttemptRebalance();
	}

	//calculates the balances of the nodes in the tree
	void CalculateBalances() {

		if (root != nullptr)
			root->CalculateBalanceFactor();

	}


	//rebalances all nodes that need it
	void Rebalance(Node* node) {


		if (node != nullptr) {


			if (node->lChild != nullptr) {
				Rebalance(node->lChild);
			}

			if (node->rChild != nullptr) {
				Rebalance(node->rChild);
			}


			if (!endRebalanceRecursion && node->rebalance) {

				if (debug)
					std::cout << "Rebalancing node: " + std::to_string(node->value) + "\n";

				//checks if this was a left case
				if (node->balanceFactor < 0) {

					//stops the recursion from rebalancing
					endRebalanceRecursion = true;

					//checks if the left child exists
					if (node->lChild != nullptr) {

						//checks for left right case
						if (node->lChild->lDepth < node->lChild->rDepth)
							LRCase(node);

						//checks if this was a left left case
						//else if (node->lChild->lDepth > node->lChild->rDepth)
						else
							LLCase(node);
					}

					//checks if this was a right case
				} else if (node->balanceFactor > 0) {

					//checks if the right child exists
					if (node->rChild != nullptr) {

						//checks for right left case
						if (node->rChild->lDepth > node->rChild->rDepth)
							RLCase(node);

						//checks if this was a right right case
						//else if (node->rChild->lDepth < node->rChild->rDepth)
						else
							RRCase(node);
					}
				}
			}
		}
	}

	//AVL rebalancing
	void LRCase(Node* node) {

		Node* three = node;
		Node* one = three->lChild;
		Node* two = one->rChild;
		Node* B = two->lChild;

		three->lChild = two;
		two->parent = three;

		two->lChild = one;
		one->parent = two;

		one->rChild = B;
		if (B != nullptr) B->parent = one;

		LLCase(node);
	}
	void LLCase(Node* node) {

		Node* parent = node->parent;
		Node* three = node;
		Node* two = node->lChild;
		Node* C = two->rChild;

		//sets the parent/root to point to the two node
		if (parent != nullptr) {

			if (parent->rChild == three) {
				parent->rChild = two;
				parent->rChild->parent = parent;
			} else {
				parent->lChild = two;
				parent->lChild->parent = parent;
			}

		} else {
			root = two;
			root->parent = nullptr;
		}

		two->rChild = three;
		three->parent = two;

		three->lChild = C;
		if (C != nullptr) C->parent = three;
	}

	void RLCase(Node* node) {

		Node* one = node;
		Node* three = one->rChild;
		Node* two = three->lChild;
		Node* C = two->rChild;

		one->rChild = two;
		two->parent = one;

		two->rChild = three;
		three->parent = two;

		three->lChild = C;
		if (C != nullptr) C->parent = three;

		RRCase(node);
	}
	void RRCase(Node* node) {

		Node* parent = node->parent;
		bool isRoot = parent == nullptr;

		Node* one = node;
		Node* two = node->rChild;
		Node* B = two->lChild;

		//sets the parent/root to point to the two node
		if (!isRoot) {

			if (parent->rChild == node) {
				parent->rChild = two;
				parent->rChild->parent = parent;
			} else {
				parent->lChild = two;
				parent->lChild->parent = parent;
			}

		} else {
			root = two;
			root->parent = nullptr;
		}

		two->lChild = node;
		node->parent = two;

		node->rChild = B;
		if (B != nullptr) B->parent = node;

	}

	//checks and saves if this node was being hovered on
	void CheckHovered(Node* node, Vector2 nodePos) {

		//gets the distance betweenthe mouse and the center of this node
		int xDif = nodePos.x - mousePos.x;
		int yDif = nodePos.y - mousePos.y;
		int distance = std::sqrt(xDif * xDif + yDif * yDif);

		//if the mouse is in this node
		if (distance < nodeRadius)
			hoveredNode = node;
	}

	//deletes the hovered node
	void DeleteHoveredNode() {

		if (hoveredNode != nullptr) {

			//checks if the hovered node was the root
			bool rootNode = false;
			if (hoveredNode == root)
				rootNode = true;

			//saves the node's pointers
			Node* parent = hoveredNode->parent;
			Node* lChild = hoveredNode->lChild;
			Node* rChild = hoveredNode->rChild;


			//checks which side of the parent the hovered node was on
			bool rSide = false;
			if (!rootNode)
				if (parent->rChild == hoveredNode)
					rSide = true;

			//if (debug)
			//	std::cout << "Deleted node: " << hoveredNode->value << "\n";
			////deletes the node
			//delete hoveredNode;


			//promotes a child from the left side (if there was the option)
			if (lChild != nullptr)
				LSidePromote(parent, lChild, rChild, rootNode, rSide);


			//promotes a child from the right side (if there was one)
			else if (rChild != nullptr)
				RSidePromote(parent, lChild, rChild, rootNode, rSide);


			//there were no children at all
			else {

				if (!rootNode)

					//sets the parent's correct side to nullptr
					if (rSide)
						parent->rChild = nullptr;
					else
						parent->lChild = nullptr;

				else
					root = nullptr;
			}
		}

		if (balance) {

			AttemptRebalance();
		}
	}

	void LSidePromote(Node* parent, Node* lChild, Node* rChild, bool rootNode, bool rSide) {

		//sets the node to promote to the lChild
		Node* toPromote = lChild;

		//keeps saving the rChild of toPromote
		while (toPromote->rChild != nullptr)
			toPromote = toPromote->rChild;

		//checks if the hovered node was the promote parent, then sets the pointer of the toPromote's parent to the toPromote (because toPromote is moving)
		if (toPromote->parent == hoveredNode) {

			toPromote->parent->lChild = toPromote->rChild;

			if (toPromote->rChild != nullptr)
				toPromote->parent->lChild->parent = toPromote->parent;

		} else {

			toPromote->parent->rChild = toPromote->lChild;

			if (toPromote->lChild != nullptr)
				toPromote->parent->rChild->parent = toPromote->parent;
		}


		//sets the l/r children of toPromote
		toPromote->rChild = rChild;
		if (rChild != nullptr)
			rChild->parent = toPromote;

		//only sets the lSide if the parent of toPromote wasn't the hovered node. If it was the hovered node, toPromote's lChild would end up pointing back to toPromote
		if (!(toPromote->parent == hoveredNode)) {

			toPromote->lChild = lChild;
			lChild->parent = toPromote;
		}

		//deleted node's parent node points to toPromote
		if (!rootNode) {

			toPromote->parent = parent;

			//sets the parent to point to toPromote
			if (rSide) {

				parent->rChild = new Node();
				parent->rChild = toPromote;
				toPromote->parent = parent;

				//sets the children's parents of the the promoted node to the promoted node
				if (parent->rChild->lChild != nullptr)
					parent->rChild->lChild->parent = parent->rChild;

				if (parent->rChild->rChild != nullptr)
					parent->rChild->rChild->parent = parent->rChild;

			} else {

				parent->lChild = new Node();
				parent->lChild = toPromote;
				parent->lChild->parent = parent;

				//sets the children's parents of the the promoted node to the promoted node
				if (parent->lChild->lChild != nullptr)
					parent->lChild->lChild->parent = parent->lChild;

				if (parent->lChild->rChild != nullptr)
					parent->lChild->rChild->parent = parent->lChild;
			}
		} else {

			//root doesn't have a parent
			toPromote->parent = nullptr;

			//sets the root to the innerLeft node
			root = toPromote;
		}

	}

	void RSidePromote(Node* parent, Node* lChild, Node* rChild, bool rootNode, bool rSide) {

		//sets the node to promote to the lChild
		Node* toPromote = rChild;

		//keeps saving the rChild of toPromote
		while (toPromote->lChild != nullptr)
			toPromote = toPromote->lChild;

		//checks if the hovered node was the promote parent, then sets the pointer of the toPromote's parent to the toPromote (because toPromote is moving)
		if (toPromote->parent == hoveredNode) {

			toPromote->parent->rChild = toPromote->lChild;

			if (toPromote->lChild != nullptr)
				toPromote->parent->rChild->parent = toPromote->parent;

		} else {

			toPromote->parent->lChild = toPromote->rChild;

			if (toPromote->rChild != nullptr)
				toPromote->parent->lChild->parent = toPromote->parent;
		}

		//sets the l/r children of toPromote
		toPromote->lChild = nullptr;

		//only sets the rSide if the parent of toPromote wasn't the hovered node. If it was the hovered node, toPromote's rChild would end up pointing back to toPromote
		if (!(toPromote->parent == hoveredNode))
			toPromote->rChild = rChild;

		if (!rootNode) {

			//sets the parent to point to toPromote
			if (rSide) {

				parent->rChild = new Node();
				parent->rChild = toPromote;
				parent->rChild->parent = parent;

				//sets the children's parents of the the promoted node to the promoted node
				if (parent->rChild->lChild != nullptr)
					parent->rChild->lChild->parent = parent->rChild;

				if (parent->rChild->rChild != nullptr)
					parent->rChild->rChild->parent = parent->rChild;

			} else {

				parent->lChild = new Node();
				parent->lChild = toPromote;
				parent->lChild->parent = parent;

				//sets the children's parents of the the promoted node to the promoted node
				if (parent->lChild->lChild != nullptr)
					parent->lChild->lChild->parent = parent->lChild;

				if (parent->lChild->rChild != nullptr)
					parent->lChild->rChild->parent = parent->lChild;
			}
		} else {

			//root doesn't have a parent
			toPromote->parent = nullptr;

			//sets the root to the innerLeft node
			root = toPromote;

		}
	}

};

Tree* t;


//resets the tree
void Reset() {
	t = new Tree();
}

//inserts random values into the tree
void InsertRandoms() {

	int randValue = GetRandomValue(0, 1000);
	t->InsertRoot(randValue);
	for (int i = 0; i < toAdd - 1; i++) {

		randValue = GetRandomValue(0, 1000);
		t->Insert(randValue);
	}
}

//values to insert for debugging
void DebugValues() {
	t->InsertRoot(41);
	t->Insert(449);
	t->Insert(328);

	t->Insert(474);
	t->Insert(150);
	t->Insert(709);
	t->Insert(467);
	t->Insert(330);
	t->Insert(936);
	t->Insert(329);

	//t->InsertRoot(3);
	//t->Insert(2);
	//t->Insert(1);



	//t->Rebalance(t->root);
}

//remakes the tree with an amount of nodes
void RemakeTree() {
	Reset();
	InsertRandoms();
	//DebugValues();

	if (debug)
		std::cout << "Ordered values: " << t->Traverse(t->root) << "\n";
}

//program draw call
void Draw() {
	BeginDrawing();

	ClearBackground(background);

	std::string heightString = "Height of tree: " + std::to_string(t->GetHeight());
	std::string balanceString = ", Balancing: " + std::to_string(balance);
	std::string toAddString = ", Next adding: " + std::to_string(toAdd) + " nodes";

	std::string out = heightString + balanceString + toAddString;
	//draws the height of the tree
	DrawText(out.c_str(), 10, 10, 4, fontColor);

	//draws the tree
	t->Draw(t->root, 0, 0);

	EndDrawing();
}

//checks for and acts upon keyboard inputs
void Inputs() {

	if (IsMouseButtonPressed(0))
		t->DeleteHoveredNode();

	//checks if enter key was pressed to make a new tree
	if (IsKeyPressed(257))
		RemakeTree();

	if (IsKeyPressed(66))
		balance = !balance;

	//checks for number presses
	for (int i = 1; i < 10; i++)
		if (IsKeyPressed(48 + i))
			toAddMantisa = i;

	if (debug)
		if (IsKeyPressed(49))
			std::cout << "one pressed\n";

	//checks for 0 press
	if (IsKeyPressed(48))
		toAddExponent++;

	//checks for backspace press
	if (IsKeyPressed(259) && toAddExponent > 0)
		toAddExponent--;

	toAdd = toAddMantisa * std::pow(10, toAddExponent);

	mousePos = GetMousePosition();
}

int main(void) {

	toAddExponent = 1;
	toAddMantisa = 2;
	toAdd = toAddMantisa * std::pow(10, toAddExponent);

	//makes the tree
	RemakeTree();

	//sets up the window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Binary Tree");

	//draw loop
	while (!WindowShouldClose()) {

		Draw();

		Inputs();

		screenWidth = GetScreenWidth();
		screenHeight = GetScreenHeight();
	}

	//closes the window
	CloseWindow();

	return 0;
}