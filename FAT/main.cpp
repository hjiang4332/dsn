#include <iostream>		//couts
#include <vector>		//data storage
#include "node.cpp"		//nodes
#include "block.cpp"	//blocks for createBlock
#include <fstream>		//file generation
#include <iomanip>		//io manipulation - for inputs 
#include <sstream>		//for importing FAT (iss >> x)
#include <stdlib.h>		//random

#define RESET 	"\033[0m"
#define YELLOW 	"\033[33m"
#define RED 	"\033[31m"
#define CYAN 	"\033[36m"

void printNice(std::string print){
	std::cout << YELLOW << "*****" << print << "*****"  << RESET << std::endl;
}
void split(std::vector<std::string>& vecToPopulate, std::string stringToSplit, int numPieces) {	//Splits stringToSplit into numPieces and puts them into vecToPopulate
	int length = stringToSplit.length();
	if (length == 0) {	//check empty
		printNice("Nothing to Split");
		return;
	}

	int splitSize = length / numPieces;
	std::string temp;

	for (int i = 0; i < length; i++) {	//split
		temp += stringToSplit[i];
		if (i % splitSize == 0 && i != 0) {
			vecToPopulate.push_back(temp);
			temp = "";
		}
	}

	if (vecToPopulate.size() != numPieces) {	//get last string if not perfect division
		vecToPopulate.push_back(temp);
	}
}	  
void swapStrings(std::string& a, std::string& b) {
	std::string temp = a;
	a = b;
	b = temp;
}
void shuffleVector(std::vector<std::string> vecToShuffle) {
	int random;
	for (int i = 0; i < vecToShuffle.size(); i++) {
		random = rand() % vecToShuffle.size();	//between 0 and size()-1
		swapStrings(vecToShuffle[i], vecToShuffle[random]);
	}
}
void createFatFromVector(std::vector<Node> &nodes, std::vector<std::string> &splitStrings) {	//populates fat with splitStrings
	for (int i = 0; i < splitStrings.size(); i++) {
		Node temp(nodes.size(), splitStrings[i]);	//nodeNumber, content 
		nodes.push_back(temp);
	}
	splitStrings.clear();
}
void createBlocks(std::vector<Node> &nodes, std::string content){
	Block temp(content);
	nodes[0].getBlocks().push_back(temp);
}
void printFat(std::vector<Node> &nodes){	//properly working
	printNice("Showing File Allocation Table");

	if(nodes.size() > 0){
		for(int i=0; i<nodes.size(); i++){	//goes through each node
			for (int j = 0; j < nodes[i].getBlocks().size(); j++) {	//goes through each block of that node.
				std::cout << "Node number: " << nodes[i].getNodeNumber() << " content: " << nodes[i].getBlocks()[j].getData() << std::endl;
			}
		}
	}
	else{
		printNice("Your File Allocation Table is empty");
	}
}
void importFat(std::vector<Node> &nodes, std::string filename){	//not working
	printNice("Importing File Allocation Table from file");
	std::ifstream myFile;
	std::string line, content;
	int nodeNumber, blockNumber, counter=0;
	
	myFile.open(filename);
	if (myFile) {
		while (getline(myFile, line)) {
			std::istringstream iss(line);
			iss >> nodeNumber;
			Node temp(nodeNumber);
			nodes.push_back(temp);
			while (iss >> blockNumber >> content) {
				Block temp(content);
				nodes[counter].getBlocks().push_back(temp);
			}
			counter++;
		}
		myFile.close();
	}
	else {
		printNice("error opening file");
		return;
	}
}
void exportFat(std::vector<Node> &nodes){	//working properly
	printNice("Exporting File Allocation Table as backup.txt");

	std::ofstream myFile;

	myFile.open("backup.txt");
	//similar to the print function, just outputs.
	for (int i = 0; i < nodes.size(); i++) {
		for (int j = 0; j < nodes[i].getBlocks().size(); j++) {
			myFile << nodes[i].getNodeNumber() << " " << nodes[i].getBlocks()[j].getData() << std::endl;
		}
	}
	myFile.close();
}

int main (int argc, char *argv[]){
	int userChoice, numFiles;
	std::string userInput;
	bool fatIsSetUp = false;
	std::vector<std::string> splitStrings;
	std::vector<Node> nodes;

	while (true) 
	{
		std::cout << CYAN << "1: Set up FAT, 2: Add new file, 3: Show FAT, 7: Clear FAT, 8: Import FAT, 9: Export FAT" << RESET << std::endl;
		std::cout << "Your Input: ";
		std::cin >> userChoice;

		switch (userChoice) 
		{
			case 1:	//set up fat once: makes the nodes and keeps that amount of nodes until deleted.
				if(fatIsSetUp == false){
					printNice("Setting up File Allocation Table");
					/*std::cout << "Enter input to store" << std::endl;
					std::cin >> userInput;
					std::cout << "enter amount of files: " << std::endl; 
					std::cin >> numFiles;*/
					userInput = "Hello there, i am a very long string";
					numFiles = 3;

					split(splitStrings, userInput, numFiles); //string vector, string, int					
					createFatFromVector(nodes, splitStrings);
					fatIsSetUp = true;
				}			
				else{
					printNice("The File Allocation Table is already set up");
				}	
				break;
			case 2:	//any additional file will be split and added randomly to different nodes that was already set up.
				if (fatIsSetUp == true) {	//only adds these blocks if there are nodes to hold them.
					printNice("Creating a file");					
					//std::cout << "Enter what to put into the file" << std::endl;
					//std::cin >> userInput;
					userInput = "This is my plain text that wil' be split up";

					split(splitStrings, userInput, numFiles);
					for(int i=0; i<splitStrings.size(); i++){
						createBlocks(nodes, splitStrings[i]);
					}
					splitStrings.clear();
				}
				else {
					printNice("Please set up the File Allocation Table First");
				}
				break;
			case 3:
				printFat(nodes);
				break;
			case 7: 
				printNice("Clearing File Allocation Table");
				nodes.clear();
				fatIsSetUp = false;
				break;
			case 8: 
				//std::cout << "Enter file name for import (add extension if applies)" << std::endl;
				//std::cin >> userInput
				userInput = "backup.txt";

				if(fatIsSetUp){
					printNice("You can't import, FAT is already set up");
				}
				else{
					importFat(nodes, userInput);
					fatIsSetUp = true;
				}
				break;
			case 9:
				exportFat(nodes);
				break;
			default:
				std::cout << RED << "*****You didn't enter a correct command*****" << RESET << std::endl;
				break;
		}
	}
	return 0;
}
