#ifndef NODE_H
#define NODE_H

#include <iostream>		//couts
#include <vector>		//data storage
#include <map>			//maps
#include "fat.cpp"		//fat
#include "block.cpp"		//block data

class Node{
	public:
		//Node(int nodeNumber);
		Node(int nodeNumber, std::string userInput);
		
		//getters and setters
		int getNodeNumber();
		void setNodeNumber(int nodeNumber);
		Block getBlock();

	private:
		int nodeNumber;
		Block block;	//blocks currently just hold 1 piece of data. Later implement to hold more. 
		//std::map<int, Block> blockInfo;	//use case 4, when each block more than 1 piece of information. Do later
		//Fat fat;	//fix after
};	

/*Node::Node(int nodeNumber) {
	this->nodeNumber = nodeNumber;
}*/

Node::Node(int nodeNumber, std::string userInput) {
	this->nodeNumber = nodeNumber;
	Block block(userInput);
	this->block = block;
}

//getters and setters
int Node::getNodeNumber() {
	return nodeNumber;
}

void Node::setNodeNumber(int nodeNumber) {
	this->nodeNumber = nodeNumber;
}

Block Node::getBlock() {
	return this->block;
}

/*Fat Node::getFat() {
	return fat;
}*/
#endif 
