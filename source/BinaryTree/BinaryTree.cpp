#include "raylib.h"
#include <iostream>
#include <string>

const int screenWidth = 1280;
const int screenHeight = 720;


struct Node {
	int value;
	Node* lChild;
	Node* rChild;

	void Set(int _value) {
		value = _value;
	}

	int Insert(int _value, int height) {

		int toReturn = height + 1;

		//checks if this value was smaller or equal
		if (_value <= value) {

			//checks if the left node was not occupied
			if (lChild == nullptr) {

				//creates the new left node and sets its value
				lChild = new Node();
				lChild->Set(_value);
			} else {

				//inserts into the left node
				toReturn = lChild->Insert(_value, toReturn);
			}
		} else {

			//checks if the right node was not occupied
			if (rChild == nullptr) {

				//creates the new right node and sets its value
				rChild = new Node();
				rChild->Set(_value);
			} else {

				//inserts into the right node
				toReturn = rChild->Insert(_value, toReturn);
			}
		}

		return toReturn;
	}
};

class Tree {

public:
	Node* root;
	int totalHeight;

	int nodeRadius;

	//constructor
	Tree() {
		root = new Node();
		totalHeight = 0;
		nodeRadius = 15;
	}

	//inserts a key into the root
	void InsertRoot(int value) {
		root->Set(value);
	}

	//inserts a value into the tree
	void Insert(int value) {

		int _height = root->Insert(value, 1);

		if (totalHeight < _height)
			totalHeight = _height;
	}

	//draws this node, and then calls the draw on its children nodes
	void Draw(Node* node, int level, int position) {

		//the position to draw this node
		Vector2 nodePosition = GetPosition(level, position);

		//the string of the node to draw's value
		std::string valueString = std::to_string(node->value);
		int fontSize = 4;
		Vector2 stringPos;
		stringPos.x = nodePosition.x - (valueString.size() * fontSize * 1.5) / 2;
		stringPos.y = nodePosition.y - (fontSize * 1.5) / 2;


		//draw this node
		DrawCircleLines(nodePosition.x, nodePosition.y, nodeRadius, RED);
		DrawText(valueString.c_str(), stringPos.x, stringPos.y, fontSize, BLACK);

		//draw the lNode subtree
		if (node->lChild != nullptr) {

			Vector2 nextNodePosition = GetPosition(level + 1, position * 2);
			DrawLine(nodePosition.x, nodePosition.y + nodeRadius, nextNodePosition.x, nextNodePosition.y - nodeRadius, BLACK);

			Draw(node->lChild, level + 1, position * 2);
		}

		//draw the rNode subtree
		if (node->rChild != nullptr) {

			Vector2 nextNodePosition = GetPosition(level + 1, position * 2 + 1);
			DrawLine(nodePosition.x, nodePosition.y + nodeRadius, nextNodePosition.x, nextNodePosition.y - nodeRadius, BLACK);
			Draw(node->rChild, level + 1, position * 2 + 1);
		}
	}

	//returns the position of a node based on its level and position in that level
	Vector2 GetPosition(int level, int position) {

		//number of possible nodes in this level
		int totalNodesInThisLevel = (int)std::pow(2, level);

		//the gaps between nodes
		Vector2 gaps;
		gaps.x = (float)screenWidth / (totalNodesInThisLevel + 1);
		gaps.y = (screenHeight - nodeRadius * 2) / totalHeight;

		//the position to draw this node
		Vector2 pos = Vector2();
		pos.x = gaps.x * (position + 1);
		pos.y = (level + 1) * gaps.y;

		return pos;
	}

};

Tree* t;

//resets the tree
void Reset() {
	t = new Tree();
}

//inserts random values into the tree
void InsertRandoms(int count) {

	int randValue = GetRandomValue(0, 1000);
	t->InsertRoot(randValue);
	for (int i = 0; i < count; i++) {

		randValue = GetRandomValue(0, 1000);
		t->Insert(randValue);
	}
}

//remakes the tree with an amount of nodes
void RemakeTree(int amount) {
	Reset();
	InsertRandoms(amount);
}

//inserts a perfect binary tree of height 3
void DebugValues() {
	//t->InsertRoot(10);

	//t->Insert(5);
	//t->Insert(15);
	//
	//t->Insert(4);
	//t->Insert(6);
	//t->Insert(14);
	//t->Insert(16);
}

int main(void)
{
	int nodeCount = 30;

	RemakeTree(nodeCount);


	InitWindow(screenWidth, screenHeight, "Binary Tree");

	// Draw
	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(RAYWHITE);

		std::string heightString = "Height of tree: " + std::to_string(t->totalHeight);
		//draws the height of the tree
		DrawText(heightString.c_str(), 10, 10, 4, BLACK);

		//draws the tree
		t->Draw(t->root, 0, 0);

		EndDrawing();

		//checks if enter key was pressed to make a new tree
		if (IsKeyPressed(257))
			RemakeTree(nodeCount);

	}

	CloseWindow();

	return 0;
}