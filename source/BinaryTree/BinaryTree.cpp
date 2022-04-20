#include "raylib.h"
#include <iostream>
#include <string>

int screenWidth = 1280;
int screenHeight = 720;
bool balance = true;
bool debug = true;

int toAddMantisa;
int toAddExponent;
int toAdd = 100;

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

		CalculateBalances();

		if (balance)
			Rebalance(root);
	}

	//draws this node, and then calls the draw on its children nodes
	void Draw(Node* node, int level, int position) {

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

		if (node->value == 329) {
			std::cout << "";
		}
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

	//calculates the balances of the nodes in the tree
	void CalculateBalances() {

		root->CalculateBalanceFactor();

	}

	//rebalances all nodes that need it
	void Rebalance(Node* node) {

		if (node->lChild != nullptr)
			Rebalance(node->lChild);

		if (node->rChild != nullptr)
			Rebalance(node->rChild);


		if (node->rebalance) {

			if (debug)
				std::cout << "Rebalancing node: " + std::to_string(node->value) + "\n";

			//checks if this was a left case
			if (node->balanceFactor < 0) {



				//checks if the left child exists
				if (node->lChild != nullptr)

					//checks if this was a left left case
					if (node->lChild->lDepth > node->lChild->rDepth)

						LLCase(node);

				//checks for left right case
					else if (node->lChild->lDepth < node->lChild->rDepth)

						LRCase(node);

				//checks if this was a right case
			} else if (node->balanceFactor > 0)

				//checks if the right child exists
				if (node->rChild != nullptr)

					//checks if this was a right right case
					if (node->rChild->lDepth < node->rChild->rDepth)

						RRCase(node);

			//checks for right left case
					else if (node->rChild->lDepth > node->rChild->rDepth)

						RLCase(node);

			CalculateBalances();

		}
	}

	//AVL rebalancing
	void LRCase(Node* node) {

		Node one = *node->lChild;
		Node two = *node->lChild->rChild;

		Node B = Node();
		bool BNull;
		if (two.lChild != nullptr) {
			BNull = false;
			B = *two.lChild;
		} else {
			BNull = true;
		}


		node->lChild = new Node();
		*node->lChild = two;
		node->lChild->parent = node;

		one.rChild = new Node();
		if (BNull)
			one.rChild = nullptr;
		else
			*one.rChild = B;


		node->lChild->lChild = new Node();
		*node->lChild->lChild = one;

		if (!BNull)
			*node->lChild->lChild->rChild->parent = *node->lChild->lChild;
		node->lChild->lChild->parent = node->lChild;

		LLCase(node);
	}
	void LLCase(Node* node) {

		Node* parent = node->parent;
		Node three = *node;
		Node two = *node->lChild;

		Node C = Node();
		bool CNull;
		if (two.rChild != nullptr) {
			CNull = false;
			C = *two.rChild;
		} else {
			CNull = true;
		}


		*node = two;
		node->parent = parent;

		three.lChild = new Node();
		if (CNull)
			three.lChild = nullptr;
		else
			*three.lChild = C;


		node->rChild = new Node();
		*node->rChild = three;

		if (!CNull)
			*node->rChild->lChild->parent = *node->rChild;
		node->rChild->parent = node;
		node->lChild->parent = node;
	}

	void RLCase(Node* node) {

		Node  two = *node->rChild->lChild;
		Node three = *node->rChild;

		Node C = Node();
		bool CNull;
		if (two.rChild != nullptr) {
			CNull = false;
			C = *two.rChild;
		} else {
			CNull = true;
		}


		node->rChild = new Node();
		*node->rChild = two;
		node->rChild->parent = node;

		three.lChild = new Node();
		if (CNull)
			three.lChild = nullptr;
		else
			*three.lChild = C;


		node->rChild->rChild = new Node();
		*node->rChild->rChild = three;

		if (!CNull)
			node->rChild->rChild->lChild->parent = node->rChild->rChild;
		node->rChild->rChild->parent = node->rChild;

		RRCase(node);
	}
	void RRCase(Node* node) {

		Node* parent = node->parent;
		Node one = *node;
		Node two = *node->rChild;

		Node B = Node();
		bool BNull;
		if (two.lChild != nullptr) {
			BNull = false;
			B = *two.lChild;
		} else {
			BNull = true;
		}

		*node = two;
		node->parent = parent;

		one.rChild = new Node();
		if (BNull)
			one.rChild = nullptr;
		else
			*one.rChild = B;


		node->lChild = new Node();
		*node->lChild = one;

		if (!BNull)
			node->lChild->rChild->parent = node->lChild;
		node->lChild->parent = node;
		node->rChild->parent = node;

	}

	//figure out why the pointers are not pointing to right locatoin after avl 
	// 
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
		if (parent->rChild == hoveredNode)
			rSide = true;

		//sets the parents connection to the hovered node to null
		if (rSide)
			parent->rChild = nullptr;
		else
			parent->lChild = nullptr;

		if (lChild != nullptr) {

			//sets the inner most node to the lChild
			Node* innerLeft = lChild;

			//keeps saving the rChild of the innerLeft
			while (innerLeft->rChild != nullptr)
				innerLeft = innerLeft->rChild;

			////sets the parent to the innerLeft node
			//if (rSide)
			//	parent->rChild = innerLeft;
			//else
			//	parent->lChild = innerLeft;

			std::cout << "Deleted node: " << hoveredNode->value << "\n";
			delete hoveredNode;

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
	t->InsertRoot(10);

	t->Insert(4);
	t->Insert(15);

	t->Insert(3);
	t->Insert(5);

	t->Insert(6);


	//t->Rebalance(t->root);
}

//remakes the tree with an amount of nodes
void RemakeTree() {
	Reset();
	InsertRandoms();
	//DebugValues()

	if (debug)
		std::cout << "Ordered values: " << t->Traverse(t->root) << "\n";
}

//program draw call
void Draw() {
	BeginDrawing();

	ClearBackground(background);

	std::string heightString = "Height of tree: " + std::to_string(t->totalHeight);
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

int main(void)
{

	toAddExponent = 0;
	toAddMantisa = 5;
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